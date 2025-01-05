#pragma once
#include "TermSP.h"

#define NUM_ROWS 6
#define NUM_KEYS 18
#define KMOD_SYNTHETIC (1 << 13)

enum { STATE_TYPED, STATE_UP, STATE_DOWN };

static int row_length[NUM_ROWS] = {13, 17, 17, 15, 14, 9};

static SDL_Keycode keys[2][NUM_ROWS][NUM_KEYS] = {
    {{SDLK_ESCAPE, SDLK_F1, SDLK_F2, SDLK_F3, SDLK_F4, SDLK_F5, SDLK_F6, SDLK_F8, SDLK_F9,
      SDLK_F10, SDLK_F11, SDLK_F12},
     {SDLK_BACKQUOTE, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7, SDLK_8, SDLK_9,
      SDLK_0, SDLK_MINUS, SDLK_EQUALS, SDLK_BACKSPACE, SDLK_INSERT, SDLK_DELETE, SDLK_UP},
     {SDLK_TAB, SDLK_q, SDLK_w, SDLK_e, SDLK_r, SDLK_t, SDLK_y, SDLK_u, SDLK_i, SDLK_o, SDLK_p,
      SDLK_LEFTBRACKET, SDLK_RIGHTBRACKET, SDLK_BACKSLASH, SDLK_HOME, SDLK_END, SDLK_DOWN},
     {SDLK_CAPSLOCK, SDLK_a, SDLK_s, SDLK_d, SDLK_f, SDLK_g, SDLK_h, SDLK_j, SDLK_k, SDLK_l,
      SDLK_SEMICOLON, SDLK_QUOTE, SDLK_RETURN, SDLK_PAGEUP, SDLK_LEFT},
     {SDLK_LSHIFT, SDLK_z, SDLK_x, SDLK_c, SDLK_v, SDLK_b, SDLK_n, SDLK_m, SDLK_COMMA, SDLK_PERIOD,
      SDLK_SLASH, SDLK_RSHIFT, SDLK_PAGEDOWN, SDLK_RIGHT},
     {SDLK_LCTRL, SDLK_LGUI, SDLK_LALT, SDLK_SPACE, SDLK_RALT, SDLK_RGUI, SDLK_MENU, SDLK_RCTRL,
      SDLK_PRINTSCREEN}},
    {{SDLK_ESCAPE, SDLK_F1, SDLK_F2, SDLK_F3, SDLK_F4, SDLK_F5, SDLK_F6, SDLK_F8, SDLK_F9,
      SDLK_F10, SDLK_F11, SDLK_F12},
     {'~', SDLK_EXCLAIM, SDLK_AT, SDLK_HASH, SDLK_DOLLAR, '%', SDLK_CARET, SDLK_AMPERSAND,
      SDLK_ASTERISK, SDLK_LEFTPAREN, SDLK_RIGHTPAREN, SDLK_UNDERSCORE, SDLK_PLUS, SDLK_BACKSPACE,
      SDLK_INSERT, SDLK_DELETE, SDLK_UP},
     {SDLK_TAB, SDLK_q, SDLK_w, SDLK_e, SDLK_r, SDLK_t, SDLK_y, SDLK_u, SDLK_i, SDLK_o, SDLK_p,
      '{', '}', '|', SDLK_HOME, SDLK_END, SDLK_DOWN},
     {SDLK_CAPSLOCK, SDLK_a, SDLK_s, SDLK_d, SDLK_f, SDLK_g, SDLK_h, SDLK_j, SDLK_k, SDLK_l,
      SDLK_COLON, SDLK_QUOTEDBL, SDLK_RETURN, SDLK_PAGEUP, SDLK_LEFT},
     {SDLK_LSHIFT, SDLK_z, SDLK_x, SDLK_c, SDLK_v, SDLK_b, SDLK_n, SDLK_m, SDLK_LESS, SDLK_GREATER,
      SDLK_QUESTION, SDLK_RSHIFT, SDLK_PAGEDOWN, SDLK_RIGHT},
     {SDLK_LCTRL, SDLK_LGUI, SDLK_LALT, SDLK_SPACE, SDLK_RALT, SDLK_RGUI, SDLK_MENU, SDLK_RCTRL,
      SDLK_PRINTSCREEN}}};

#define CREDIT "Update by @Nevrdid (c) 2024."

static unsigned char toggled[NUM_ROWS][NUM_KEYS];

static int show_help    = 0;
static int active       = 0;
static int location     = 1;
static int selected_i   = 0;
static int selected_j   = 0;
static int shifted      = 0;
static int mod_state    = 0;
static int total_length = 40;

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
