#include "EVENTS.h"
#include "WHEEL.h"
#include <SDL2/SDL_stdinc.h>
#define PI 3.14159265358979323846 /* pi */

int _getWheelCharIndex(int axisXid, int axisYid, int nbChars)
{
#define THRESHOLD 300

    float x = SDL_JoystickGetAxis(term.joystick, axisXid);
    float y = -SDL_JoystickGetAxis(term.joystick, axisYid);
    if ((x < THRESHOLD && x > -THRESHOLD) || (y < THRESHOLD && y > -THRESHOLD))
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
#if defined(TEST)
    return 2;
#endif
    return _getWheelCharIndex(AXIS_LEFT_X, AXIS_LEFT_Y, NB_CHARS_LEFT);
}
int WHEEL_GetSelectedCharIndexRight()
{
#if defined(TEST)
    return 5;
#endif
    return _getWheelCharIndex(AXIS_RIGHT_X, AXIS_RIGHT_Y, NB_CHARS_RIGHT);
}

void _drawWheel(int select_char_index, int char_start, int nbChars, int centerX, int centerY)
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
        FOX_RenderChar(term.font.virt_kb_wheel, vKeyboardChars[i + char_start], 0, &pos);
        if (i == select_char_index)
        {
            SDL_SetRenderDrawColor(term.renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
            FOX_RenderChar(term.font.virt_kb_wheel, vKeyboardChars[i + char_start], 0, &pos);
            SDL_SetRenderDrawColor(term.renderer, 93, 226, 231, SDL_ALPHA_OPAQUE);
        }
    }
}
void WHEEL_Draw()
{
    int center = cfg.width / 2;
    int xIndex = WHEEL_GetSelectedCharIndexLeft();
    int yIndex = WHEEL_GetSelectedCharIndexRight();
    if (xIndex >= 0 || yIndex >= 0)
    {
        _drawWheel(xIndex, 0, NB_CHARS_LEFT, center - 200, 300);
        _drawWheel(yIndex, NB_CHARS_RIGHT_OFFSET, NB_CHARS_RIGHT, center + 200, 300);
    }
}

int WHEEL_PressKey()
{
    int iLeft = WHEEL_GetSelectedCharIndexLeft();
    if (iLeft >= 0)
    {
        KEYB_SimulateKey(vKeyboardChars[iLeft], STATE_TYPED);
        return 1;
    }
    else
    {
        int iRight = WHEEL_GetSelectedCharIndexRight();
        if (iRight >= 0)
        {
            KEYB_SimulateKey(vKeyboardChars[iRight + NB_CHARS_RIGHT_OFFSET], STATE_TYPED);
            return 1;
        }
    }
    return 0;
}