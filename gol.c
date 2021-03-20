//Copyright (c) 2011-2020 <>< Charles Lohr - Under the MIT/x11 or NewBSD License you choose.
// NO WARRANTY! NO GUARANTEE OF SUPPORT! USE AT YOUR OWN RISK

// TODO(#2): action popup animation
// currently: paused, reset
// TODO(#4): Brian's Brain

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

#define ALIVE 1
#define DEAD 0
#define DEFAULT_GRID_SIZE 32

#define SPACE_KEY 32
#define R_KEY 114

#ifdef _WIN32
#define MINUS_KEY 189
#define PLUS_KEY 187
#else
#define MINUS_KEY 45
#define PLUS_KEY 43
#define EQ_KEY 61
#endif

#define GRID_SIZE(gs) (sizeof(int) * gs * gs)

int *grid = NULL;
int *next_grid = NULL;
int grid_size = DEFAULT_GRID_SIZE;
int paused = 0;
int reset_t = 0;

short w, h;
int cell_width, cell_height;

volatile int suspended;

#ifdef __ANDROID__
static int keyboard_up;
int font_size = 20;
int paused_t_width = 350;
#else
int font_size = 10;
int paused_t_width = 200;
#endif

void ToggleCell(int x, int y, int val);

void change_grid_size(int new_size)
{
    grid_size = new_size;
    cell_width = w / grid_size;
    cell_height = h / grid_size;
    grid = realloc(grid, GRID_SIZE(grid_size));
    next_grid = realloc(next_grid, GRID_SIZE(grid_size));
    memset(grid, 0, GRID_SIZE(grid_size));
    memset(next_grid, 0, GRID_SIZE(grid_size));
}

void HandleKey(int keycode, int bDown)
{
    if (bDown)
        switch (keycode)
        {
        case SPACE_KEY:
            paused = !paused;
            break;
        case R_KEY:
            memset(grid, 0, GRID_SIZE(grid_size));
            reset_t = OGGetAbsoluteTime();
            break;
        case MINUS_KEY:
        {
            int new_size = grid_size - 4;
            if (new_size <= 0)
            {
                return;
            }
            change_grid_size(new_size);
        }
        break;
#ifndef _WIN32
        case EQ_KEY:
#endif
        case PLUS_KEY:
            change_grid_size(grid_size + 4);
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
    *cell_x = x / (w / grid_size);
    *cell_y = y / (h / grid_size);
}

int OnGrid(int cell_x, int cell_y)
{
    return (0 <= cell_x && cell_x <= grid_size - 1) && (0 <= cell_y && cell_y <= grid_size - 1);
}

void ToggleCell(int x, int y, int val)
{
    int cell_x, cell_y;
    GetCellIndex(x, y, &cell_x, &cell_y);
    if (OnGrid(cell_x, cell_y))
        grid[cell_x * grid_size + cell_y] = val;
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
            if (OnGrid(i, j) && (i != x || j != y) && grid[i * grid_size + j] == ALIVE)
                neighbours++;

    return neighbours;
}

void ApplyRules(int x, int y)
{
    int neighbours_count = CountNeighbours(x, y);
    if (grid[x * grid_size + y] == ALIVE)
    {
        if (neighbours_count < 2)
            next_grid[x * grid_size + y] = DEAD;
        else if (neighbours_count == 2 || neighbours_count == 3)
        {
        }
        else if (neighbours_count > 3)
            next_grid[x * grid_size + y] = DEAD;
    }
    else if (grid[x * grid_size + y] == DEAD)
    {
        if (neighbours_count == 3)
            next_grid[x * grid_size + y] = ALIVE;
    }
}

void DrawCells()
{
    memcpy(next_grid, grid, GRID_SIZE(grid_size));
    for (int y = 0; y < grid_size; ++y)
        for (int x = 0; x < grid_size; ++x)
        {
            if (!paused)
                ApplyRules(x, y);
            if (grid[x * grid_size + y] == ALIVE)
                DrawCell(x, y);
        }
    memcpy(grid, next_grid, GRID_SIZE(grid_size));
}

void DrawMessages(int t)
{
    if (paused)
    {
        DrawMessage(w - paused_t_width, 10, "Paused");
    }
    if (reset_t)
    {
        if (t - reset_t <= 1)
        {
            DrawMessage(10, 10, "Reset");
        }
        else
        {
            reset_t = 0;
        }
    }
}

int main()
{
    int absolute_time;
    CNFGBGColor = 0x000080ff;
    GolSetup();
    cell_width = w / grid_size;
    cell_height = h / grid_size;
    grid = calloc(1, GRID_SIZE(grid_size));
    next_grid = calloc(1, GRID_SIZE(grid_size));

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
