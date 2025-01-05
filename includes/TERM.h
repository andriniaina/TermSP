#pragma once
#include "TermSP.h"

int  TERM_Init();
void TERM_Resize(int width, int height);
void TERM_Update();
void TERM_DeinitializeTerminal();

void signalHandler(int signum);

int damage(VTermRect rect, void *user);
int moveRect(VTermRect dest, VTermRect src, void *user);
int moveCursor(VTermPos pos, VTermPos oldpos, int visible, void *user);
int setTermProp(VTermProp prop, VTermValue *val, void *user);
int bell(void *user);
int sb_pushline(int cols, const VTermScreenCell *cells, void *user);
int sb_popline(int cols, VTermScreenCell *cells, void *user);

void renderCell(int x, int y);
void renderScreen();
void updateLibPath(const char *new_path);
