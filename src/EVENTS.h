#pragma once
#include "TermSP.h"

static char clipboardbuffer[1024];
enum { STATE_TYPED, STATE_UP, STATE_DOWN };

int         EV_HandleEvents(TERM_State *state);
static void handleWin(TERM_State *state, SDL_Event *event);
static void handleKeyboard(TERM_State *state, SDL_Event *event);
int         handleJoyButtons(TERM_State *state, SDL_Event *ev);
int         handleJoyHat(TERM_State *state, SDL_Event *ev);
int         handleJoyAxis(TERM_State *state, SDL_Event *ev);
static void handleChild(TERM_State *state);
