#pragma once
#include "TermSP.h"

static char clipboardbuffer[1024];
enum { STATE_TYPED, STATE_UP, STATE_DOWN };

int         EV_HandleEvents();
static void handleWin(SDL_Event *event);
static void handleKeyboard(SDL_Event *event);
int         handleJoyButtons(SDL_Event *ev);
int         handleJoyHat(SDL_Event *ev);
int         handleJoyAxis(SDL_Event *ev);
static void handleChild();
