#include "TermSP.h"
#include "WHEEL.h"
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_image.h>
#define PI 3.14159265358979323846 /* pi */
#define NB_ELEMENTS(arr) (sizeof(arr) / sizeof(*arr))

static SDL_Surface *image_LB;
static SDL_Surface *image_RB;
static SDL_Texture *texture_LB;
static SDL_Texture *texture_RB;
static SDL_Rect rectLB = {0, 100, 64, 64};
static SDL_Rect rectRB = {500, 100, 64, 64};
static int center = 0;

static SDL_KeyCode layer_alpha[] = {
    SDLK_SPACE,
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
};
static SDL_KeyCode layer_numeric[] = {
    SDLK_SPACE,
    SDLK_0,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_4,

    SDLK_SPACE,
    SDLK_5,
    SDLK_6,
    SDLK_7,
    SDLK_8,
    SDLK_9,
};
static SDL_KeyCode layer_symbols[] = {
    SDLK_SPACE,
    SDLK_SLASH,
    SDLK_PERIOD,
    SDLK_GREATER,
    SDLK_MINUS,

    SDLK_EQUALS,
    SDLK_UNDERSCORE,
    SDLK_ASTERISK,
    SDLK_LESS,
    SDLK_SEMICOLON,
};
static SDL_KeyCode layer_fn[] = {
    SDLK_F1,
    SDLK_F2,
    SDLK_F3,
    SDLK_F4,
    SDLK_F5,
    SDLK_F6,

    SDLK_F7,
    SDLK_F8,
    SDLK_F9,
    SDLK_F10,
    SDLK_F11,
    SDLK_F12,
};

typedef enum
{
    ALPHA,
    NUMERIC,
    SYMBOL,
    FN
} SelectedLayer;
static int NB_LAYERS = FN - ALPHA;
static SelectedLayer selectedLayer = ALPHA;

typedef  SDL_Keycode * p_SDL_Keycode;
typedef struct LayerDefinition
{
    int nbCharsLeft;
    int nbCharsRight;
    p_SDL_Keycode layer;
    char *name;
} LayerDefinition;

int WHEEL_init()
{
    center = cfg.width / 2;
    rectLB.x = center - 300 - rectLB.w;
    rectRB.x = center + 300;

    image_LB = IMG_Load("res/Light/T_X_LB_Light.png");
    if (image_LB == NULL)
        fprintf(stderr, "Could not load resource %s", SDL_GetError());

    image_RB = IMG_Load("res/Light/T_X_RB_Light.png");
    if (image_RB == NULL)
        fprintf(stderr, "Could not load resource %s", SDL_GetError());

    texture_LB = SDL_CreateTextureFromSurface(term.renderer, image_LB);
    texture_RB = SDL_CreateTextureFromSurface(term.renderer, image_RB);
    return 0; // TODO
}
LayerDefinition getSelectedLayer()
{
    LayerDefinition result;
    if (selectedLayer == ALPHA)
    {
        result.name = "alpha";
        result.layer = (p_SDL_Keycode)layer_alpha;
        result.nbCharsLeft = NB_ELEMENTS(layer_alpha) / 2;
        result.nbCharsRight = NB_ELEMENTS(layer_alpha) / 2;
    }
    if (selectedLayer == NUMERIC)
    {
        result.name = "numeric";
        result.layer = (p_SDL_Keycode)layer_numeric;
        result.nbCharsLeft = NB_ELEMENTS(layer_numeric) / 2;
        result.nbCharsRight = NB_ELEMENTS(layer_numeric) / 2;
    }
    if (selectedLayer == SYMBOL)
    {
        result.name = "symbols";
        result.layer = (p_SDL_Keycode)layer_symbols;
        result.nbCharsLeft = NB_ELEMENTS(layer_symbols) / 2;
        result.nbCharsRight = NB_ELEMENTS(layer_symbols) / 2;
    }
    if (selectedLayer == FN)
    {
        result.name = "Fn";
        result.layer = (p_SDL_Keycode)layer_fn;
        result.nbCharsLeft = NB_ELEMENTS(layer_fn) / 2;
        result.nbCharsRight = NB_ELEMENTS(layer_fn) / 2;
    }

    return result;
};
int WHEEL_deinit()
{
    SDL_DestroyTexture(texture_LB);
    SDL_DestroyTexture(texture_RB);

    SDL_FreeSurface(image_LB);
    SDL_FreeSurface(image_RB);
    return 0; // TODO
}
int _getWheelCharIndex(int axisXid, int axisYid, int nbChars)
{

    float x = SDL_JoystickGetAxis(term.joystick, axisXid);
    float y = -SDL_JoystickGetAxis(term.joystick, axisYid);
    if ((x < TRIGGER_THRESHOLD && x > -TRIGGER_THRESHOLD) || (y < TRIGGER_THRESHOLD && y > -TRIGGER_THRESHOLD))
        return -1;

    //  0 is full right, angle =[0;2PI]
    float angle = 0;
    if (x == 0 && y > 0)
        angle = PI / 2;
    else if (x == 0 && y < 0)
        angle = 3 * PI / 2;
    else
    {
        angle = atan(y / x);
    }
    if (x < 0 && y > 0)
        angle = PI + angle;
    else if (x < 0 && y < 0)
        angle = PI + angle;
    else if (x > 0 && y < 0)
        angle = (2 * PI) + angle;
    float p = angle / (2 * PI);
    int i = p * nbChars;
    return i;
}

int WHEEL_GetSelectedCharIndexLeft()
{
    LayerDefinition def = getSelectedLayer();
    int NB_CHARS_LEFT = def.nbCharsLeft;

#if defined(TEST)
    return 2;
#endif
    return _getWheelCharIndex(AXIS_LEFT_X, AXIS_LEFT_Y, NB_CHARS_LEFT);
}
int WHEEL_GetSelectedCharIndexRight()
{
    LayerDefinition def = getSelectedLayer();
    int NB_CHARS_RIGHT = def.nbCharsRight;

#if defined(TEST)
    return 5;
#endif
    return _getWheelCharIndex(AXIS_RIGHT_X, AXIS_RIGHT_Y, NB_CHARS_RIGHT);
}

void _drawWheel(int select_char_index, p_SDL_Keycode keys, int char_start, int nbChars, int centerX, int centerY)
{
    SDL_SetRenderDrawColor(term.renderer, 93, 226, 231, SDL_ALPHA_OPAQUE);
    if (select_char_index >= 0)
    {
        double angle = 2 * PI * (float)select_char_index / nbChars;
#define LINE_LENGTH 100
        int x = LINE_LENGTH * cos(angle);
        int y = LINE_LENGTH * sin(angle);
        SDL_RenderDrawLine(term.renderer, centerX, centerY, centerX + x, centerY - y);
    }
    for (int i = 0; i < nbChars; i++)
    {
        double a = 2 * PI * (float)i / nbChars;
        int x = LINE_LENGTH * cos(a);
        int y = LINE_LENGTH * sin(a);
#define OFFSET_X -6
#define OFFSET_Y -24
        SDL_Point pos = {centerX + x + OFFSET_X, centerY - y + OFFSET_Y};

        FOX_RenderChar(term.font.virt_kb_wheel, keys[i + char_start], 0, &pos);
        if (i == select_char_index)
        {
            SDL_SetRenderDrawColor(term.renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
            FOX_RenderChar(term.font.virt_kb_wheel, keys[i + char_start], 0, &pos);
            SDL_SetRenderDrawColor(term.renderer, 93, 226, 231, SDL_ALPHA_OPAQUE);
        }
    }
}

void WHEEL_Draw()
{
    int xIndex = WHEEL_GetSelectedCharIndexLeft();
    int yIndex = WHEEL_GetSelectedCharIndexRight();
    if (xIndex >= 0 || yIndex >= 0)
    {
        LayerDefinition def = getSelectedLayer();
        SDL_Point point = {center - (8 * sizeof(def.name) / 2), rectLB.y};
        FOX_RenderText(term.font.virt_kb_wheel, (Uint8*)def.name, &point);
        SDL_RenderCopy(term.renderer, texture_LB, NULL, &rectLB);
        SDL_RenderCopy(term.renderer, texture_RB, NULL, &rectRB);
        _drawWheel(xIndex, def.layer, 0, def.nbCharsLeft, center - 200, 300);
        _drawWheel(yIndex, def.layer, def.nbCharsLeft, def.nbCharsRight, center + 200, 300);
    }
}

int WHEEL_PressKey()
{
    int iLeft = WHEEL_GetSelectedCharIndexLeft();
    LayerDefinition def = getSelectedLayer();
    if (iLeft >= 0)
    {
        KEYB_SimulateKey(def.layer[iLeft], STATE_TYPED);
        return 1;
    }
    else
    {
        int iRight = WHEEL_GetSelectedCharIndexRight();
        if (iRight >= 0)
        {
            KEYB_SimulateKey(def.layer[def.nbCharsRight + def.nbCharsLeft], STATE_TYPED);
            return 1;
        }
    }
    return 0;
}

int WHEEL_ShiftLayer(int shift)
{
    selectedLayer = (selectedLayer + shift + NB_LAYERS) % NB_LAYERS;
    int iLeft = WHEEL_GetSelectedCharIndexLeft();
    int iRight = WHEEL_GetSelectedCharIndexRight();
    return iLeft >= 0 && iRight >= 0;
}