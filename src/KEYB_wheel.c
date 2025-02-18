#include "KEYB_wheel.h"
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

int GetWheelSelectedCharIndexLeft()
{
#if defined(TEST)
    return 2;
#endif
    return _getWheelCharIndex(AXIS_LEFT_X, AXIS_LEFT_Y, NB_CHARS_LEFT);
}
int GetWheelSelectedCharIndexRight()
{
#if defined(TEST)
    return 5;
#endif
    return _getWheelCharIndex(AXIS_RIGHT_X, AXIS_RIGHT_Y, NB_CHARS_RIGHT);
}

void _drawWheel(int select_char_index, int char_start, int nbChars, int centerX, int centerY)
{
    if (select_char_index > 0)
    {
        double angle = 2 * PI * (float)select_char_index / nbChars;
        SDL_SetRenderDrawColor(term.renderer, 93, 226, 231, SDL_ALPHA_OPAQUE);
#define LINE_LENGTH 100
        int x = LINE_LENGTH * cos(angle);
        int y = LINE_LENGTH * sin(angle);
        SDL_RenderDrawLine(term.renderer, centerX, centerY, centerX + x, centerY - y);
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
}
void DrawWheelVirtualKeyboard()
{
    int center = cfg.width / 2;
    _drawWheel(GetWheelSelectedCharIndexLeft(), 0, NB_CHARS_LEFT, center - 200, 300);
    _drawWheel(GetWheelSelectedCharIndexRight(), NB_CHARS_RIGHT_OFFSET, NB_CHARS_RIGHT, center + 200, 300);
}