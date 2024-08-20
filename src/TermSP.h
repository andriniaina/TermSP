#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <fcntl.h>
#include <pty.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <vterm.h>

#include "SDL_fox.h"

typedef struct {
    char      **args;
    const char *fontpattern;
    const char *boldfontpattern;
    int         virtkb;
    int         refreshrate;
    int         fontsize;
    int         width;
    int         height;
    int         rows;
    int         columns;
} TERM_Config;

typedef struct {
    SDL_Window    *window;
    SDL_Renderer  *renderer;
    SDL_Cursor    *pointer;
    SDL_Joystick  *joystick;
    SDL_Surface   *vscreen;
    const Uint8   *keys;
    VTerm         *vterm;
    VTermScreen   *screen;
    VTermState    *termstate;
    struct termios initial_tio;
    struct {
        const FOX_FontMetrics *metrics;
        FOX_Font              *regular;
        FOX_Font              *bold;
        FOX_Font              *virt_kb;
    } font;
    struct {
        SDL_Point position;
        bool      visible;
        bool      active;
        Uint32    ticks;
    } cursor;
    struct {
        SDL_Rect rect;
        bool     clicked;
    } mouse;
    struct {
        Uint32 ticks;
        bool   active;
    } bell;
    Uint32      ticks;
    bool        dirty;
    pid_t       child;
    int         childfd;
    TERM_Config cfg;
} TERM_State;


int EV_HandleEvents(TERM_State *state);

int  TERM_Init(TERM_State *state, TERM_Config *cfg);
void TERM_Resize(TERM_State *state, int width, int height);
void TERM_Update(TERM_State *state);
void TERM_DeinitializeTerminal(TERM_State *state);

int  KEYB_Init(TERM_State *state, TERM_Config *cfg);
void KEYB_RenderVirtualKeyboard(TERM_State *state);
int  KEYB_MoveCursor(int dx, int dy);
void KEYB_ClickKey();
void KEYB_Toggle();
void KEYB_SwitchLocation();
void KEYB_Shift(int state);
void KEYB_ToggleMod();

void KEYB_UpdateModstate(int key, int state);
void KEYB_SimulateKey(int key, int state);
int  KEYB_VisualOffset(int col, int row);
int  KEYB_NewCol(int visual_offset, int old_row, int new_row);

void swap(int *a, int *b);
