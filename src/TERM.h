#pragma once
#include "TermSP.h"

int  TERM_Init(TERM_State *state, TERM_Config *cfg);
void TERM_Resize(TERM_State *state, int width, int height);
void TERM_Update(TERM_State *state);
void TERM_DeinitializeTerminal(TERM_State *state);

void signalHandler(int signum);

int damage(VTermRect rect, void *user);
int moveRect(VTermRect dest, VTermRect src, void *user);
int moveCursor(VTermPos pos, VTermPos oldpos, int visible, void *user);
int setTermProp(VTermProp prop, VTermValue *val, void *user);
int bell(void *user);
int sb_pushline(int cols, const VTermScreenCell *cells, void *user);
int sb_popline(int cols, VTermScreenCell *cells, void *user);

void renderCell(TERM_State *state, int x, int y);
void renderScreen(TERM_State *state);
void updateLibPath(const char *new_path);
