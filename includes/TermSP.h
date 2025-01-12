#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <pty.h>
#include <linux/fb.h>


#include "SDL_fox.h"
#include "vterm.h"
#include "vterm_keycodes.h"

typedef struct {
  char **args;
  const char *fontpattern;
  const char *boldfontpattern;
  int virtkb;
  int refreshrate;
  int cursorinterval;
  int fontsize;
  int width;
  int height;
  int rows;
  int columns;
  int gnuscreen;
} TERM_Config;
extern TERM_Config cfg;

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Cursor *pointer;
  SDL_Joystick *joystick;
  SDL_Surface *vscreen;
  const Uint8 *keys;
  VTerm *vterm;
  VTermScreen *screen;
  VTermState *termstate;
  struct termios initial_tio;
  struct {
    const FOX_FontMetrics *metrics;
    FOX_Font *regular;
    FOX_Font *bold;
    FOX_Font *virt_kb;
  } font;
  struct {
    SDL_Point position;
    bool visible;
    bool active;
    Uint32 ticks;
  } cursor;
  struct {
    SDL_Rect rect;
    bool clicked;
  } mouse;
  struct {
    Uint32 ticks;
    bool active;
  } bell;
  Uint32 ticks;
  bool dirty;
  pid_t child;
  int childfd;
} TERM_State;
extern TERM_State term;

int EV_HandleEvents();

int TERM_Init();
void TERM_Resize(int width, int height);
void TERM_Update();
void TERM_DeinitializeTerminal();

int KEYB_Init();
void KEYB_RenderVirtualKeyboard();
int KEYB_MoveCursor(int dx, int dy);
void KEYB_ClickKey();
void KEYB_Toggle();
void KEYB_SwitchLocation();
void KEYB_Shift(int state);
void KEYB_ToggleMod();

void KEYB_UpdateModstate(int key, int state);
void KEYB_SimulateKey(int key, int state);
int KEYB_VisualOffset(int col, int row);
int KEYB_NewCol(int visual_offset, int old_row, int new_row);

void swap(int *a, int *b);
