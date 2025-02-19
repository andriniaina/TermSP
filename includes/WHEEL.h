#include "TermSP.h"

#define NB_CHARS_LEFT 16
#define NB_CHARS_RIGHT 16
#define NB_CHARS_RIGHT_OFFSET NB_CHARS_LEFT
#define AXIS_LEFT_X 0
#define AXIS_LEFT_Y 1
#define AXIS_RIGHT_X 3
#define AXIS_RIGHT_Y 4

// ? L2->shift
static SDL_KeyCode vKeyboardChars[] = {
    SDLK_a,
    SDLK_b,
    SDLK_c,
    SDLK_d,
    SDLK_e,
    SDLK_f,
    SDLK_g,
    SDLK_h,
    SDLK_i,
    SDLK_j,
    SDLK_k,
    SDLK_l,
    SDLK_m,
    SDLK_n,
    SDLK_o,
    SDLK_p,
    SDLK_q,
    SDLK_r,
    SDLK_s,
    SDLK_t,
    SDLK_u,
    SDLK_v,
    SDLK_x,
    SDLK_y,
    SDLK_z,
    SDLK_SPACE,
    SDLK_SLASH,
    SDLK_PERIOD,
    SDLK_GREATER,
    SDLK_MINUS,
    SDLK_EQUALS,
    SDLK_UNDERSCORE,
};
int WHEEL_GetSelectedCharIndexLeft();
int WHEEL_GetSelectedCharIndexRight();
void WHEEL_Draw();
