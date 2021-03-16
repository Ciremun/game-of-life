//Copyright (c) 2011-2020 <>< Charles Lohr - Under the MIT/x11 or NewBSD License you choose.
// NO WARRANTY! NO GUARANTEE OF SUPPORT! USE AT YOUR OWN RISK

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "os_generic.h"
#include <GLES3/gl3.h>
#include <asset_manager.h>
#include <asset_manager_jni.h>
#include <android_native_app_glue.h>
#include <android/sensor.h>
#include "CNFGAndroid.h"

#define CNFG_IMPLEMENTATION
#define CNFG3D

#include "CNFG.h"

#define ROWS 32
#define COLS 32
#define ALIVE 1
#define DEAD 0

short grid[ROWS][COLS] = {{0}};
short w, h, cell_width, cell_height;

volatile int suspended;

void HandleKey(int keycode, int bDown) {}

void GetCellIndex(int x, int y, short *cell_x, short *cell_y)
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
	short cell_x, cell_y;
	GetCellIndex(x, y, &cell_x, &cell_y);
	if (OnGrid(cell_x, cell_y))
		grid[cell_x][cell_y] = val;
}

void HandleButton(int x, int y, int button, int bDown)
{
	ToggleCell(x, y, ALIVE);
}

void HandleMotion(int x, int y, int mask)
{
	ToggleCell(x, y, ALIVE);
}

void HandleDestroy() {}

void HandleSuspend()
{
	suspended = 1;
}

void HandleResume()
{
	suspended = 0;
}

void DrawCell(int x, int y)
{
	short cell_x = x * cell_width;
	short cell_y = y * cell_height;

	CNFGTackRectangle(cell_x, cell_y, cell_x + cell_width, cell_y + cell_height);
}

int CountNeighbours(int x, int y)
{
	short neighbours = 0;

	for (int i = x - 1; i <= x + 1; i++)
		for (int j = y - 1; j <= y + 1; j++)
			if (OnGrid(i, j) && (i != x || j != y) &&grid[i][j] == ALIVE)
				neighbours++;

	return neighbours;
}

void ApplyRules(int x, int y, short next_gen[ROWS][COLS])
{
	int neighbours_count = CountNeighbours(x, y);
	if (grid[x][y] == ALIVE)
	{
		if (neighbours_count < 2)
			next_gen[x][y] = DEAD;
		else if (neighbours_count == 2 || neighbours_count == 3) {}
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
	short next_gen[ROWS][COLS];
	memcpy(next_gen, grid, sizeof(short[ROWS][COLS]));
	for (int y = 0; y < ROWS; ++y)
		for (int x = 0; x < COLS; ++x)
		{
			ApplyRules(x, y, next_gen);
			if (grid[x][y] == ALIVE)
				DrawCell(x, y);
		}
	memcpy(grid, next_gen, sizeof(short[ROWS][COLS]));
}

int main()
{
	CNFGBGColor = 0x000080ff;
	CNFGSetupFullscreen("gol", 0);
	CNFGGetDimensions(&w, &h);
	cell_width = w / COLS;
	cell_height = h / ROWS;

	while (1)
	{
		CNFGClearFrame();
		CNFGHandleInput();

		usleep(50000);
		if (suspended) continue;

		CNFGColor(0xff00ffff);
		DrawCells();

		CNFGSwapBuffers();
	}

	return 0;
}
