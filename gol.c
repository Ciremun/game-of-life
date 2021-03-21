//Copyright (c) 2011-2020 <>< Charles Lohr - Under the MIT/x11 or NewBSD License you choose.
// NO WARRANTY! NO GUARANTEE OF SUPPORT! USE AT YOUR OWN RISK

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

#define WINDOW_NAME "Cellular Automatons"
#define MAX_MESSAGE_SIZE 256

#define ALIVE 1
#define DEAD 0
#define DYING 2
#define DEFAULT_GRID_SIZE 32
#define GRID_SIZE_CHANGE_STEP 8

#define GAME_OF_LIFE 0
#define BRIANS_BRAIN 1

#define WHITE 0xffffffff

#define FADE_IN 0
#define FADE_OUT 1
#define IDLE 2
#define HIDDEN 3

#define SPACE_KEY 32
#define R_KEY 114
#define ONE_KEY 49
#define TWO_KEY 50

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
int gamemode = GAME_OF_LIFE;
int paused = 0;
int reset_t = 0;
int message_t = 0;

short w, h;
int cell_width, cell_height;
double absolute_time;
char message[MAX_MESSAGE_SIZE];

volatile int suspended;

#ifdef __ANDROID__
static int keyboard_up;
int font_size = 20;
int paused_t_width = 350;
#else
int font_size = 10;
int paused_t_width = 200;
#endif

typedef struct
{
    uint32_t color;
    double duration;
    double start;
    int state;
} Animation;

Animation pause_a = {.color = 0xffffffff, .duration = .5, .start = 0.0, .state = HIDDEN};
Animation message_a = {.color = 0xffffffff, .duration = 1.0, .start = 0.0, .state = HIDDEN};

void toggle_cell(int x, int y, int val);

void change_animation_state(Animation *a, int new_state)
{
    a->start = OGGetAbsoluteTime();
    a->state = new_state;
}

void display_message(char *msg)
{
    size_t msg_size = strlen(msg) + 1;
    memset(message, 0, MAX_MESSAGE_SIZE);
    memcpy(message, msg, msg_size);
    message_t = OGGetAbsoluteTime();
    change_animation_state(&message_a, FADE_IN);
}

void set_fade_color(Animation *a)
{
    uint32_t new_color;
    switch (a->state)
    {
    case FADE_IN:
    {
        double s_passed = absolute_time - a->start;
        if (s_passed >= a->duration)
        {
            a->state = IDLE;
            new_color = a->color;
        }
        else
        {
            new_color = (a->color & 0xffffff00) + (s_passed / a->duration) * 255;
        }
    }
    break;
    case FADE_OUT:
    {
        double s_passed = absolute_time - a->start;
        if (s_passed >= a->duration)
        {
            a->state = HIDDEN;
            new_color = a->color & 0xffffff00;
        }
        else
        {
            new_color = (a->color & 0xffffff00) + ((a->duration - s_passed) / a->duration) * 255;
        }
    }
    break;
    case IDLE:
        new_color = a->color;
        break;
    default:
        printf("invalid animation state\n");
        exit(1);
    }
    CNFGColor(new_color);
}

void change_grid_size(int new_size)
{
    grid_size = new_size;
    cell_width = w / grid_size;
    cell_height = h / grid_size;
    grid = realloc(grid, GRID_SIZE(grid_size));
    next_grid = realloc(next_grid, GRID_SIZE(grid_size));
    memset(grid, 0, GRID_SIZE(grid_size));
    memset(next_grid, 0, GRID_SIZE(grid_size));
    snprintf(message, 256, "%s: %d", "Grid Size", grid_size);
    message_t = OGGetAbsoluteTime();
    change_animation_state(&message_a, FADE_IN);
}

void HandleKey(int keycode, int bDown)
{
    if (bDown)
        switch (keycode)
        {
        case SPACE_KEY:
            paused = !paused;
            switch (pause_a.state)
            {
            case FADE_IN:
                pause_a.state = FADE_OUT;
                break;
            case FADE_OUT:
                pause_a.state = FADE_IN;
                break;
            case IDLE:
                change_animation_state(&pause_a, FADE_OUT);
                break;
            case HIDDEN:
                change_animation_state(&pause_a, FADE_IN);
                break;
            }
            break;
        case R_KEY:
            memset(grid, 0, GRID_SIZE(grid_size));
            reset_t = OGGetAbsoluteTime();
            break;
        case MINUS_KEY:
        {
            int new_size = grid_size - GRID_SIZE_CHANGE_STEP;
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
            change_grid_size(grid_size + GRID_SIZE_CHANGE_STEP);
            break;
        case ONE_KEY:
            gamemode = GAME_OF_LIFE;
            display_message("Game Of Life");
            break;
        case TWO_KEY:
            gamemode = BRIANS_BRAIN;
            display_message("Brian's Brain");
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
        toggle_cell(x, y, ALIVE);
    }
}

void HandleMotion(int x, int y, int mask)
{
#ifndef __ANDROID__
    if (!mask)
        return;
#endif
    toggle_cell(x, y, ALIVE);
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

void setup_window()
{
#ifdef __ANDROID__
    CNFGSetupFullscreen(WINDOW_NAME, 0);
    CNFGGetDimensions(&w, &h);
#else
    w = 1024;
    h = 768;
    CNFGSetup(WINDOW_NAME, w, h);
#endif
}

void cell_index(int x, int y, int *cell_x, int *cell_y)
{
    *cell_x = x / (w / grid_size);
    *cell_y = y / (h / grid_size);
}

int on_grid(int cell_x, int cell_y)
{
    return (0 <= cell_x && cell_x <= grid_size - 1) && (0 <= cell_y && cell_y <= grid_size - 1);
}

void toggle_cell(int x, int y, int val)
{
    int cell_x, cell_y;
    cell_index(x, y, &cell_x, &cell_y);
    if (on_grid(cell_x, cell_y))
        grid[cell_x * grid_size + cell_y] = val;
}

void draw_message(int x, int y, const char *t)
{
    CNFGPenX = x;
    CNFGPenY = y;
    CNFGDrawText(t, font_size);
}

void draw_cell(int x, int y)
{
    int cell_x = x * cell_width;
    int cell_y = y * cell_height;

    CNFGTackRectangle(cell_x, cell_y, cell_x + cell_width, cell_y + cell_height);
}

int count_neighbours(int x, int y)
{
    int neighbours = 0;

    for (int i = x - 1; i <= x + 1; i++)
        for (int j = y - 1; j <= y + 1; j++)
            if (on_grid(i, j) && (i != x || j != y) && grid[i * grid_size + j] == ALIVE)
                neighbours++;

    return neighbours;
}

void game_of_life_rules(int x, int y, int nbors)
{
    switch (grid[x * grid_size + y])
    {
        case ALIVE:
            if (!(nbors == 2 || nbors == 3)) next_grid[x * grid_size + y] = DEAD;
            break;
        case DEAD:
            if (nbors == 3) next_grid[x * grid_size + y] = ALIVE;
            break;
    }
}

void brians_brain_rules(int x, int y, int nbors)
{
    switch (grid[x * grid_size + y])
    {
        case ALIVE:
            next_grid[x * grid_size + y] = DYING;
            break;
        case DYING:
            next_grid[x * grid_size + y] = DEAD;
            break;
        case DEAD:
            if (nbors == 2) next_grid[x * grid_size + y] = ALIVE;
            break;
    }
}

void apply_game_rules(int x, int y)
{
    int neighbours_count = count_neighbours(x, y);
    switch (gamemode)
    {
    case GAME_OF_LIFE:
        game_of_life_rules(x, y, neighbours_count);
        break;
    case BRIANS_BRAIN:
        brians_brain_rules(x, y, neighbours_count);
        break;
    }
}

void draw_cells()
{
    memcpy(next_grid, grid, GRID_SIZE(grid_size));
    for (int y = 0; y < grid_size; ++y)
        for (int x = 0; x < grid_size; ++x)
        {
            if (!paused)
                apply_game_rules(x, y);
            if (grid[x * grid_size + y] == ALIVE)
                draw_cell(x, y);
        }
    memcpy(grid, next_grid, GRID_SIZE(grid_size));
}

void draw_messages()
{
    if (pause_a.state != HIDDEN)
    {
        set_fade_color(&pause_a);
        draw_message(w - paused_t_width, 10, "Paused");
    }
    if (message_a.state != HIDDEN)
    {
        if (message_t && absolute_time - message_t > 2)
        {
            message_t = 0;
            change_animation_state(&message_a, FADE_OUT);
        }
        set_fade_color(&message_a);
        draw_message(w / 2 - strlen(message) * 30, 120, message);
    }
    if (reset_t)
    {
        if (absolute_time - reset_t <= 1)
        {
            CNFGColor(WHITE);
            draw_message(10, 10, "Reset");
        }
        else
        {
            reset_t = 0;
        }
    }
}

int main()
{
    CNFGBGColor = 0x000080ff;
    setup_window();

    cell_width = w / grid_size;
    cell_height = h / grid_size;

    grid = calloc(1, GRID_SIZE(grid_size));
    next_grid = calloc(1, GRID_SIZE(grid_size));

    display_message("Game Of Life");

    while (1)
    {
        CNFGClearFrame();
        CNFGHandleInput();

        if (suspended)
            continue;

        if (!paused)
            sleep_ms(50);

        CNFGColor(0xff00ffff);
        draw_cells();

        absolute_time = OGGetAbsoluteTime();
        draw_messages();

        CNFGSwapBuffers();
    }

    return 0;
}
