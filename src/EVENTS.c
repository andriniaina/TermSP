#include "EVENTS.h"

extern int childState;

int EV_HandleEvents(TERM_State *state) {
    SDL_Event event;
    int       status = 0;

    if (childState == 0) {
        SDL_Event event;
        event.type = SDL_QUIT;
        SDL_PushEvent(&event);
    }

    handleChild(state);

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: status = 1; break;

            case SDL_WINDOWEVENT: handleWin(state, &event); break;

            case SDL_KEYDOWN: handleKeyboard(state, &event); break;

            case SDL_JOYBUTTONUP:
            case SDL_JOYBUTTONDOWN:
                if (handleJoyButtons(state, &event) == -1) status = 1;
                break;
            case SDL_JOYAXISMOTION: handleJoyAxis(state, &event); break;
            case SDL_JOYHATMOTION: handleJoyHat(state, &event); break;

            case SDL_TEXTINPUT:
                write(state->childfd, event.edit.text, SDL_strlen(event.edit.text));
                break;
        }

        switch (event.type) {
            case SDL_MOUSEMOTION:
                if (state->mouse.clicked) {
                    state->mouse.rect.w = event.motion.x - state->mouse.rect.x;
                    state->mouse.rect.h = event.motion.y - state->mouse.rect.y;
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (state->mouse.clicked) {
                } else if (event.button.button == SDL_BUTTON_LEFT) {
                    state->mouse.clicked = true;
                    SDL_GetMouseState(&state->mouse.rect.x, &state->mouse.rect.y);
                    state->mouse.rect.w = 0;
                    state->mouse.rect.h = 0;
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    char *clipboard = SDL_GetClipboardText();
                    write(state->childfd, clipboard, SDL_strlen(clipboard));
                    SDL_free(clipboard);
                } else if (event.button.button == SDL_BUTTON_LEFT) {
                    VTermRect rect = {
                        .start_col = state->mouse.rect.x / state->font.metrics->max_advance,
                        .start_row = state->mouse.rect.y / state->font.metrics->height,
                        .end_col   = (state->mouse.rect.x + state->mouse.rect.w) /
                                   state->font.metrics->max_advance,
                        .end_row = (state->mouse.rect.y + state->mouse.rect.h) /
                                   state->font.metrics->height};
                    if (rect.start_col > rect.end_col) swap(&rect.start_col, &rect.end_col);
                    if (rect.start_row > rect.end_row) swap(&rect.start_row, &rect.end_row);
                    size_t n = vterm_screen_get_text(state->screen, clipboardbuffer,
                                                     sizeof(clipboardbuffer), rect);
                    if (n >= sizeof(clipboardbuffer)) n = sizeof(clipboardbuffer) - 1;
                    clipboardbuffer[n] = '\0';
                    SDL_SetClipboardText(clipboardbuffer);
                    state->mouse.clicked = false;
                }
                break;

            case SDL_MOUSEWHEEL: {
                int size = state->cfg.fontsize;
                size += event.wheel.y;
                FOX_CloseFont(state->font.regular);
                FOX_CloseFont(state->font.bold);
                state->font.regular = FOX_OpenFont(state->renderer, state->cfg.fontpattern, size);
                if (state->font.regular == NULL) return -1;

                state->font.bold = FOX_OpenFont(state->renderer, state->cfg.boldfontpattern, size);
                if (state->font.bold == NULL) return -1;
                state->font.metrics = FOX_QueryFontMetrics(state->font.regular);
                TERM_Resize(state, state->cfg.width, state->cfg.height);
                state->cfg.fontsize = size;
                break;
            }
        }
    }

    return status;
}

static void handleWin(TERM_State *state, SDL_Event *event) {
    switch (event->window.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            TERM_Resize(state, event->window.data1, event->window.data2);
            break;
    }
}

static void handleKeyboard(TERM_State *state, SDL_Event *event) {
    const char *cmd = NULL;

    if (state->keys[SDL_SCANCODE_LCTRL]) {
        int mod = SDL_toupper(event->key.keysym.sym);
        if (mod >= 'A' && mod <= 'Z') {
            char ch = mod - 'A' + 1;
            write(state->childfd, &ch, sizeof(ch));
            return;
        }
    }

    switch (event->key.keysym.sym) {
        case SDLK_ESCAPE: cmd = "\033"; break;

        case SDLK_LEFT: cmd = "\033[D"; break;

        case SDLK_RIGHT: cmd = "\033[C"; break;

        case SDLK_UP: cmd = "\033[A"; break;

        case SDLK_DOWN: cmd = "\033[B"; break;

        case SDLK_PAGEDOWN: cmd = "\033[6~"; break;

        case SDLK_PAGEUP: cmd = "\033[5~"; break;

        case SDLK_RETURN: cmd = "\r"; break;

        case SDLK_INSERT: cmd = "\033[2~"; break;

        case SDLK_DELETE: cmd = "\033[3~"; break;

        case SDLK_BACKSPACE: cmd = "\b"; break;

        case SDLK_TAB: cmd = "\t"; break;

        case SDLK_F1: cmd = "\033OP"; break;

        case SDLK_F2: cmd = "\033OQ"; break;

        case SDLK_F3: cmd = "\033OR"; break;

        case SDLK_F4: cmd = "\033OS"; break;

        case SDLK_F5: cmd = "\033[15~"; break;

        case SDLK_F6: cmd = "\033[17~"; break;

        case SDLK_F7: cmd = "\033[18~"; break;

        case SDLK_F8: cmd = "\033[19~"; break;

        case SDLK_F9: cmd = "\033[20~"; break;

        case SDLK_F10: cmd = "\033[21~"; break;

        case SDLK_F11: cmd = "\033[23~"; break;

        case SDLK_F12: cmd = "\033[24~"; break;
    }

    if (cmd) { write(state->childfd, cmd, SDL_strlen(cmd)); }
}

int handleJoyButtons(TERM_State *state, SDL_Event *ev) {
    switch (ev->type) {
        case SDL_JOYBUTTONDOWN:
            switch (ev->jbutton.button) {
                case 0:  // B button (b0)
                    KEYB_SimulateKey(SDLK_BACKSPACE, STATE_DOWN);
                    break;
                case 1:  // A button (b1)
                    KEYB_ClickKey();
                    break;
                case 2:  // X button (b2)
                    KEYB_Toggle();
                    return 1;
                    break;
                case 3:  // Y button (b3)
                    KEYB_SwitchLocation();
                    break;
                case 4:  // Left Shoulder (L1) (b4)
                    KEYB_Shift(1);
                    break;
                case 5:  // Right Shoulder (R1) (b5)
                    KEYB_ToggleMod();
                    break;
                case 6:  // Back button (b6)
                    KEYB_SimulateKey(SDLK_TAB, STATE_DOWN);
                    break;
                case 7:  // Start button (b7)
                    KEYB_SimulateKey(SDLK_RETURN, STATE_DOWN);
                    break;
                case 8:  // Guide button (Home) (b8)
                    return -1;
                    break;
                case 9:  // Left Stick Button (L3) (b9)
                    break;
                case 10:  // Right Stick Button (R3) (b10)
                    break;
            }
            break;

        case SDL_JOYBUTTONUP:
            switch (ev->jbutton.button) {
                case 4: KEYB_Shift(0); break;
            }
            break;
    }
    return 0;
}

int handleJoyHat(TERM_State *state, SDL_Event *ev) {
    switch (ev->jhat.value) {
        case SDL_HAT_UP:  // D-Pad Up
            if (!KEYB_MoveCursor(0, 1)) KEYB_SimulateKey(SDLK_UP, STATE_DOWN);
            break;
        case SDL_HAT_DOWN:  // D-Pad Down
            if (!KEYB_MoveCursor(0, -1)) KEYB_SimulateKey(SDLK_DOWN, STATE_DOWN);
            break;
        case SDL_HAT_LEFT:  // D-Pad Left
            if (!KEYB_MoveCursor(-1, 0)) KEYB_SimulateKey(SDLK_LEFT, STATE_DOWN);
            break;
        case SDL_HAT_RIGHT:  // D-Pad Right
            if (!KEYB_MoveCursor(1, 0)) KEYB_SimulateKey(SDLK_RIGHT, STATE_DOWN);
            break;
    }
    return 0;
}

int handleJoyAxis(TERM_State *state, SDL_Event *ev) {
    // static Uint32 last_event_time = 0;
    // if (ev->type == SDL_JOYAXISMOTION) {
    //     Uint32 current_time = SDL_GetTicks();
    //     if (current_time - last_event_time < 100) {  // 100ms threshold
    //         return 0;                                // Ignore this event
    //     }
    //     last_event_time = current_time;
    // }
    SDL_Event event;

    switch (ev->jaxis.axis) {
        case 0:  // Left Stick X-Axis
            if (ev->jaxis.value < -16000) KEYB_SimulateKey(SDLK_LEFT, STATE_TYPED);
            else if (ev->jaxis.value > 16000) KEYB_SimulateKey(SDLK_RIGHT, STATE_TYPED);
            break;
        case 1:  // Left Stick Y-Axis
            if (ev->jaxis.value < -16000) KEYB_SimulateKey(SDLK_UP, STATE_TYPED);
            else if (ev->jaxis.value > 16000) KEYB_SimulateKey(SDLK_DOWN, STATE_TYPED);
        case 2:  // Left Trigger
            if (ev->jaxis.value == 32767) KEYB_SimulateKey(SDLK_SLASH, STATE_TYPED);
            break;
        case 3:
            event.type        = SDL_MOUSEMOTION;
            event.motion.xrel = ev->jaxis.value / 1000;
            event.motion.yrel = 0;
            SDL_PushEvent(&event);
            break;
        case 4:  // Right Stick X-Axis
            event.type        = SDL_MOUSEMOTION;
            event.motion.xrel = 0;
            event.motion.yrel = ev->jaxis.value / 1000;
            SDL_PushEvent(&event);
            break;
        case 5:  // Right Trigger
            break;
    }
    return 0;
}

static void handleChild(TERM_State *state) {
    fd_set         rfds;
    struct timeval tv = {0};

    FD_ZERO(&rfds);
    FD_SET(state->childfd, &rfds);

    tv.tv_sec  = 0;
    tv.tv_usec = 50000;

    if (select(state->childfd + 1, &rfds, NULL, NULL, &tv) > 0) {
        char line[4096];
        int  n;
        if ((n = read(state->childfd, line, sizeof(line))) > 0) {
            vterm_input_write(state->vterm, line, n);
            state->dirty = true;
            // vterm_screen_flush_damage(state->screen);
        }
    }
}
