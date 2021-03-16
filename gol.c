//Copyright (c) 2011-2020 <>< Charles Lohr - Under the MIT/x11 or NewBSD License you choose.
// NO WARRANTY! NO GUARANTEE OF SUPPORT! USE AT YOUR OWN RISK

// TODO(#2): action popup animation
// currently: paused, reset
// TODO(#3): update readme

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "rawdraw/os_generic.h"

#ifdef _MSC_VER
#pragma comment(lib, "gdi32")
#pragma comment(lib, "User32")
#endif

#define CNFG_IMPLEMENTATION

#include "rawdraw/CNFG.h"

#define ROWS 32
#define COLS 32

#define ALIVE 1
#define DEAD 0

int grid[ROWS][COLS] = {{0}};
short w, h;
int cell_width, cell_height;
int paused = 0;
int reset_t = 0;
int font_size = 10;

volatile int suspended;

#ifdef __ANDROID__
static int keyboard_up;
#endif

void ToggleCell(int x, int y, int val);

void HandleKey(int keycode, int bDown)
{
	if (bDown)
		switch (keycode)
		{
		case 32:
			paused = !paused;
			break;
		case 114:
			memset(grid, 0, sizeof(int[ROWS][COLS]));
			reset_t = OGGetAbsoluteTime();
			break;
		}
	else
	{
#ifdef __ANDROID__
		switch (keycode)
		{
		case 10:
			keyboard_up = 0;
			AndroidDisplayKeyboard(keyboard_up);
			break;
		case 4:
			AndroidSendToBack(1);
			break;
		}
#endif
	}
}

void HandleButton(int x, int y, int button, int bDown)
{
	if (bDown)
	{
#ifdef __ANDROID__
		if ((w - 100 <= x && x <= w) && (0 <= y && y <= 100))
		{
			keyboard_up = !keyboard_up;
			AndroidDisplayKeyboard(keyboard_up);
			return;
		}
#endif
		ToggleCell(x, y, ALIVE);
	}
}

void HandleMotion(int x, int y, int mask)
{
#ifndef __ANDROID__
	if (!mask)
		return;
#endif
	ToggleCell(x, y, ALIVE);
}

void HandleDestroy() {}
void HandleSuspend() { suspended = 1; }
void HandleResume() { suspended = 0; }

void sleep_ms(int ms)
{
#ifdef _WIN32
	_sleep(ms);
#else
	usleep(ms * 1000);
#endif
}

void GolSetup()
{
#ifdef __ANDROID__
	CNFGSetupFullscreen("gol", 0);
	CNFGGetDimensions(&w, &h);
#else
	w = 1024;
	h = 768;
	CNFGSetup("gol", w, h);
#endif
}

void GetCellIndex(int x, int y, int *cell_x, int *cell_y)
{
	*cell_x = x / (w / COLS);
	*cell_y = y / (h / ROWS);
}

int OnGrid(int cell_x, int cell_y)
{
	return (0 <= cell_x && cell_x <= COLS - 1) && (0 <= cell_y && cell_y <= ROWS - 1);
}

void ToggleCell(int x, int y, int val)
{
	int cell_x, cell_y;
	GetCellIndex(x, y, &cell_x, &cell_y);
	if (OnGrid(cell_x, cell_y))
		grid[cell_x][cell_y] = val;
}

void DrawMessage(int x, int y, const char *t)
{
	CNFGColor(0xffffffff);
	CNFGPenX = x;
	CNFGPenY = y;
	CNFGDrawText(t, font_size);
}

void DrawCell(int x, int y)
{
	int cell_x = x * cell_width;
	int cell_y = y * cell_height;

	CNFGTackRectangle(cell_x, cell_y, cell_x + cell_width, cell_y + cell_height);
}

int CountNeighbours(int x, int y)
{
	int neighbours = 0;

	for (int i = x - 1; i <= x + 1; i++)
		for (int j = y - 1; j <= y + 1; j++)
			if (OnGrid(i, j) && (i != x || j != y) && grid[i][j] == ALIVE)
				neighbours++;

	return neighbours;
}

void ApplyRules(int x, int y, int next_gen[ROWS][COLS])
{
	int neighbours_count = CountNeighbours(x, y);
	if (grid[x][y] == ALIVE)
	{
		if (neighbours_count < 2)
			next_gen[x][y] = DEAD;
		else if (neighbours_count == 2 || neighbours_count == 3)
		{
		}
		else if (neighbours_count > 3)
			next_gen[x][y] = DEAD;
	}
	else if (grid[x][y] == DEAD)
	{
		if (neighbours_count == 3)
			next_gen[x][y] = ALIVE;
	}
}

void DrawCells()
{
	int next_gen[ROWS][COLS];
	memcpy(next_gen, grid, sizeof(int[ROWS][COLS]));
	for (int y = 0; y < ROWS; ++y)
		for (int x = 0; x < COLS; ++x)
		{
			if (!paused)
				ApplyRules(x, y, next_gen);
			if (grid[x][y] == ALIVE)
				DrawCell(x, y);
		}
	memcpy(grid, next_gen, sizeof(int[ROWS][COLS]));
}

void DrawMessages(int t)
{
	if (paused)
		DrawMessage(w - 200, 10, "Paused");
	if (reset_t)
		if (t - reset_t <= 1)
			DrawMessage(10, 10, "Reset");
		else
			reset_t = 0;
}

int main()
{
	int absolute_time;
	CNFGBGColor = 0x000080ff;
	GolSetup();
	cell_width = w / COLS;
	cell_height = h / ROWS;

	while (1)
	{
		CNFGClearFrame();
		CNFGHandleInput();

		if (suspended)
			continue;

		if (!paused)
			sleep_ms(50);

		CNFGColor(0xff00ffff);
		DrawCells();

		absolute_time = OGGetAbsoluteTime();
		DrawMessages(absolute_time);

		CNFGSwapBuffers();
	}

	return 0;
}
