#include "EVENTS.h"
#include "KEYB_wheel.h"

extern int childState;

int EV_HandleEvents()
{
    SDL_Event event;
    int status = 0;

    if (childState == 0)
    {
        SDL_Event event;
        event.type = SDL_QUIT;
        SDL_PushEvent(&event);
    }

    handleChild();

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            status = 1;
            break;

        case SDL_WINDOWEVENT:
            handleWin(&event);
            break;

        case SDL_KEYDOWN:
            handleKeyboard(&event);
            break;

        case SDL_JOYBUTTONUP:
        case SDL_JOYBUTTONDOWN:
            if (handleJoyButtons(&event) == -1)
                status = 1;
            break;
        case SDL_JOYAXISMOTION:
            handleJoyAxis(&event);
            break;
        case SDL_JOYHATMOTION:
            handleJoyHat(&event);
            break;

        case SDL_TEXTINPUT:
            write(term.childfd, event.edit.text, SDL_strlen(event.edit.text));
            break;
        }

        switch (event.type)
        {
        case SDL_MOUSEMOTION:
            if (term.mouse.clicked)
            {
                term.mouse.rect.w = event.motion.x - term.mouse.rect.x;
                term.mouse.rect.h = event.motion.y - term.mouse.rect.y;
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (term.mouse.clicked)
            {
            }
            else if (event.button.button == SDL_BUTTON_LEFT)
            {
                term.mouse.clicked = true;
                SDL_GetMouseState(&term.mouse.rect.x, &term.mouse.rect.y);
                term.mouse.rect.w = 0;
                term.mouse.rect.h = 0;
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_RIGHT)
            {
                char *clipboard = SDL_GetClipboardText();
                write(term.childfd, clipboard, SDL_strlen(clipboard));
                SDL_free(clipboard);
            }
            else if (event.button.button == SDL_BUTTON_LEFT)
            {
                VTermRect rect = {
                    .start_col = term.mouse.rect.x / term.font.metrics->max_advance,
                    .start_row = term.mouse.rect.y / term.font.metrics->height,
                    .end_col = (term.mouse.rect.x + term.mouse.rect.w) /
                               term.font.metrics->max_advance,
                    .end_row = (term.mouse.rect.y + term.mouse.rect.h) /
                               term.font.metrics->height};
                if (rect.start_col > rect.end_col)
                    swap(&rect.start_col, &rect.end_col);
                if (rect.start_row > rect.end_row)
                    swap(&rect.start_row, &rect.end_row);
                size_t n = vterm_screen_get_text(term.screen, clipboardbuffer,
                                                 sizeof(clipboardbuffer), rect);
                if (n >= sizeof(clipboardbuffer))
                    n = sizeof(clipboardbuffer) - 1;
                clipboardbuffer[n] = '\0';
                SDL_SetClipboardText(clipboardbuffer);
                term.mouse.clicked = false;
            }
            break;

        case SDL_MOUSEWHEEL:
        {
            int size = cfg.fontsize;
            size += event.wheel.y;
            FOX_CloseFont(term.font.regular);
            FOX_CloseFont(term.font.bold);
            term.font.regular = FOX_OpenFont(term.renderer, cfg.fontpattern, size);
            if (term.font.regular == NULL)
                return -1;

            term.font.bold = FOX_OpenFont(term.renderer, cfg.boldfontpattern, size);
            if (term.font.bold == NULL)
                return -1;
            term.font.metrics = FOX_QueryFontMetrics(term.font.regular);
            cfg.fontsize = size;
            TERM_Resize(cfg.width, cfg.height);
            break;
        }
        }
    }

    return status;
}

static void handleWin(SDL_Event *event)
{
    switch (event->window.event)
    {
    case SDL_WINDOWEVENT_SIZE_CHANGED:
        TERM_Resize(event->window.data1, event->window.data2);
        break;
    }
}

static void handleKeyboard(SDL_Event *event)
{
    const char *cmd = NULL;

    if (SDL_GetModState() & KMOD_CTRL)
    {
        int mod = SDL_toupper(event->key.keysym.sym);
        if (mod >= 'A' && mod <= 'Z')
        {
            char ch = mod - 'A' + 1;
            write(term.childfd, &ch, sizeof(ch));
            return;
        }
    }

    switch (event->key.keysym.sym)
    {
    case SDLK_ESCAPE:
        cmd = "\033";
        break;

    case SDLK_LEFT:
        cmd = "\033[D";
        break;

    case SDLK_RIGHT:
        cmd = "\033[C";
        break;

    case SDLK_UP:
        cmd = "\033[A";
        break;

    case SDLK_DOWN:
        cmd = "\033[B";
        break;

    case SDLK_PAGEDOWN:
        cmd = "\033[6~";
        break;

    case SDLK_PAGEUP:
        cmd = "\033[5~";
        break;

    case SDLK_RETURN:
        cmd = "\r";
        break;

    case SDLK_INSERT:
        cmd = "\033[2~";
        break;

    case SDLK_DELETE:
        cmd = "\033[3~";
        break;

    case SDLK_BACKSPACE:
        cmd = "\b";
        break;

    case SDLK_TAB:
        cmd = "\t";
        break;

    case SDLK_F1:
        cmd = "\033OP";
        break;

    case SDLK_F2:
        cmd = "\033OQ";
        break;

    case SDLK_F3:
        cmd = "\033OR";
        break;

    case SDLK_F4:
        cmd = "\033OS";
        break;

    case SDLK_F5:
        cmd = "\033[15~";
        break;

    case SDLK_F6:
        cmd = "\033[17~";
        break;

    case SDLK_F7:
        cmd = "\033[18~";
        break;

    case SDLK_F8:
        cmd = "\033[19~";
        break;

    case SDLK_F9:
        cmd = "\033[20~";
        break;

    case SDLK_F10:
        cmd = "\033[21~";
        break;

    case SDLK_F11:
        cmd = "\033[23~";
        break;

    case SDLK_F12:
        cmd = "\033[24~";
        break;
    }

    if (cmd)
    {
        write(term.childfd, cmd, SDL_strlen(cmd));
    }
}

int handleJoyButtons(SDL_Event *ev)
{
    switch (ev->type)
    {
    case SDL_JOYBUTTONDOWN:
        switch (ev->jbutton.button)
        {
        case 0: // B button (b0)
            KEYB_SimulateKey(SDLK_BACKSPACE, STATE_DOWN);
            break;
        case 1: // A button (b1)
            KEYB_ClickKey();
            break;
        case 2: // X button (b2)
            KEYB_Toggle();
            return 1;
            break;
        case 3: // Y button (b3)
            KEYB_SwitchLocation();
            break;
        case 4: // Left Shoulder (L1) (b4)
            KEYB_Shift(1);
            break;
        case 5: // Right Shoulder (R1) (b5)
            KEYB_ToggleMod();
            break;
        case 6: // Back button (b6)
            KEYB_SimulateKey(SDLK_TAB, STATE_DOWN);
            break;
        case 7: // Start button (b7)
            KEYB_SimulateKey(SDLK_RETURN, STATE_DOWN);
            break;
        case 8: // Guide button (Home) (b8)
            return -1;
            break;
        case 9: // Left Stick Button (L3) (b9)
            break;
        case 10: // Right Stick Button (R3) (b10)
            break;
        }
        break;

    case SDL_JOYBUTTONUP:
        switch (ev->jbutton.button)
        {
        case 4:
            KEYB_Shift(0);
            break;
        }
        break;
    }
    return 0;
}

int handleJoyHat(SDL_Event *ev)
{
    switch (ev->jhat.value)
    {
    case SDL_HAT_UP: // D-Pad Up
        if (!KEYB_MoveCursor(0, 1))
            KEYB_SimulateKey(SDLK_UP, STATE_DOWN);
        break;
    case SDL_HAT_DOWN: // D-Pad Down
        if (!KEYB_MoveCursor(0, -1))
            KEYB_SimulateKey(SDLK_DOWN, STATE_DOWN);
        break;
    case SDL_HAT_LEFT: // D-Pad Left
        if (!KEYB_MoveCursor(-1, 0))
            KEYB_SimulateKey(SDLK_LEFT, STATE_DOWN);
        break;
    case SDL_HAT_RIGHT: // D-Pad Right
        if (!KEYB_MoveCursor(1, 0))
            KEYB_SimulateKey(SDLK_RIGHT, STATE_DOWN);
        break;
    }
    return 0;
}

int handleJoyAxis(SDL_Event *ev)
{
    // static Uint32 last_event_time = 0;
    // if (ev->type == SDL_JOYAXISMOTION) {
    //     Uint32 current_time = SDL_GetTicks();
    //     if (current_time - last_event_time < 100) {  // 100ms threshold
    //         return 0;                                // Ignore this event
    //     }
    //     last_event_time = current_time;
    // }
    SDL_Event event;

    switch (ev->jaxis.axis)
    {
    case 0: // Left Stick X-Axis
        // if (ev->jaxis.value < -16000) KEYB_SimulateKey(SDLK_LEFT, STATE_TYPED);
        // else if (ev->jaxis.value > 16000) KEYB_SimulateKey(SDLK_RIGHT, STATE_TYPED);
        break;
    case 1: // Left Stick Y-Axis
        // if (ev->jaxis.value < -16000) KEYB_SimulateKey(SDLK_UP, STATE_TYPED);
        // else if (ev->jaxis.value > 16000) KEYB_SimulateKey(SDLK_DOWN, STATE_TYPED);
        break;
    case 2: // Left Trigger
        int iLeft = GetWheelSelectedCharIndexLeft();
        if (iLeft > 0)
        {
            KEYB_SimulateKey(vKeyboardChars[iLeft], STATE_TYPED);
        }
        else
        {
            int iRight = GetWheelSelectedCharIndexLeft();
            if (iRight > 0)
            {
                KEYB_SimulateKey(vKeyboardChars[iRight + NB_CHARS_RIGHT_OFFSET], STATE_TYPED);
            }
        }
        break;
    case 3:
        event.type = SDL_MOUSEMOTION;
        event.motion.xrel = ev->jaxis.value / 1000;
        event.motion.yrel = 0;
        SDL_PushEvent(&event);
        break;
    case 4: // Right Stick X-Axis
        event.type = SDL_MOUSEMOTION;
        event.motion.xrel = 0;
        event.motion.yrel = ev->jaxis.value / 1000;
        SDL_PushEvent(&event);
        break;
    case 5: // Right Trigger
        if (ev->jaxis.value == 32767)
            KEYB_SimulateKey(SDLK_F4, STATE_TYPED);
        break;
    }
    return 0;
}

static void handleChild()
{
    fd_set rfds;
    struct timeval tv = {0};

    FD_ZERO(&rfds);
    FD_SET(term.childfd, &rfds);

    tv.tv_sec = 0;
    tv.tv_usec = 50000;

    if (select(term.childfd + 1, &rfds, NULL, NULL, &tv) > 0)
    {
        char line[4096];
        int n;
        if ((n = read(term.childfd, line, sizeof(line))) > 0)
        {
            vterm_input_write(term.vterm, line, n);
            term.dirty = true;
            // vterm_screen_flush_damage(term.screen);
        }
    }
}
