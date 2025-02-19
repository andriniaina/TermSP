#include "KEYB.h"

static char *syms[2][6][18] = {
    {{"Esc", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", NULL},
     {"` ", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "Bsp", "Ins", "Del", " ^ ",
      NULL},
     {"Tab", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "\\", "Home", "End",
      " \xde ", NULL},
     {"Caps", "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "Enter", "Pg Up", " < ",
      NULL},
     {"Shift", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", " Shift", "Pg Dn", " > ", NULL},
     {"Ctrl", " ", "Alt", "    Space    ", "Alt", " ", "Fn", "Ctrl", "PsS", NULL}},
    {{"Esc", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", NULL},
     {"~ ", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "Bsp", "Ins", "Del", " ^ ",
      NULL},
     {"Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "|", "Home", "End",
      " \xde ", NULL},
     {"Caps", "A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\"", "Enter", "Pg Up", " < ",
      NULL},
     {"Shift", "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", " Shift", "Pg Dn", " > ", NULL},
     {"Ctrl", " ", "Alt", "    Space    ", "Alt", " ", "Fn", "Ctrl", "PsS", NULL}}};

char *help =
    "How to use:\n"
    "  ARROWS:     select key from keyboard\n"
    "  A:          press key\n"
    "  B:          backspace\n"
    "  L1:         shift\n"
    "  R1:         toggle key (for shift/ctrl...)\n"
    "  Y:          change keyboard location\n"
    "  X:          show / hide keyboard\n"
    "  START:      enter\n"
    "  SELECT:     tab\n"
    "  L2:         left\n"
    "  R2:         right\n"
    "  MENU:       quit\n\n"
    "Cheatcheet (tutorial at www.shellscript.sh):\n"
    "  TAB key         complete path\n"
    "  UP/DOWN keys    navigate history\n"
    "  pwd             print current directory\n"
    "  ls              list files (-l for file size)\n"
    "  cd <d>          change directory (.. = go up)\n"
    "  cp <f> <d>      copy files (dest can be dir)\n"
    "  mv <f> <d>      move files (dest can be dir)\n"
    "  rm <f>          remove files (use -rf for dir)\n\n";

int KEYB_Init() {
    for (int j = 0; j < 6; j++)
        for (int i = 0; i < 18; i++) toggled[j][i] = 0;
    active = cfg.virtkb ? 1 : 0;
    return 0;
}

void KEYB_RenderVirtualKeyboard() {
    if (!active) return;
    // Render the virtual keyboard
    SDL_Rect rect;

    if (show_help) {
        SDL_Rect rect = {.x = 0, .y = 0, .w = cfg.width, .h = cfg.height};
        SDL_SetRenderDrawColor(term.renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(term.renderer, &rect);
        SDL_SetRenderDrawColor(term.renderer, 255, 255, 255, 255);
        SDL_Point pos = {.x = 8, .y = 30};
        FOX_RenderText(term.font.virt_kb, (const Uint8 *)help, &pos);
        pos.y += 16 * 22;
        FOX_RenderText(term.font.virt_kb, (const Uint8 *)CREDIT, &pos);
        return;
    }
    rect.w = (4 + total_length) * 16;
    rect.h = (6 * 2 + 1) * 16;
    rect.x = (cfg.width - rect.w) / 2;
    rect.y = location ? cfg.height - rect.h - 16 * 2 : 16 * 2;

    SDL_SetRenderDrawColor(term.renderer, 64, 64, 64, 255);
    SDL_RenderFillRect(term.renderer, &rect);
    int x;
    int y = rect.y + 10;

    for (int j = 0; j < 6; j++) {
        x = rect.x + 10;
        for (int i = 0; i < row_length[j]; i++) {
            int      length = strlen(syms[shifted][j][i]);
            SDL_Rect rect2  = {.x = x, .y = y, .w = (length + 1) * 16 - 4, .h = 16 * 2 - 2};
            if (toggled[j][i]) {
                if (selected_i == i && selected_j == j)
                    SDL_SetRenderDrawColor(term.renderer, 255, 255, 128, 255);
                else SDL_SetRenderDrawColor(term.renderer, 192, 192, 0, 255);
            } else if (selected_i == i && selected_j == j) {
                SDL_SetRenderDrawColor(term.renderer, 128, 255, 128, 255);
            } else {
                SDL_SetRenderDrawColor(term.renderer, 128, 128, 128, 255);
            }
            SDL_RenderFillRect(term.renderer, &rect2);
            SDL_SetRenderDrawColor(term.renderer, 0, 0, 0, 255);
            SDL_Point pos = {.x = x + 8, .y = y + 3};
            FOX_RenderText(term.font.virt_kb, (const Uint8 *)syms[shifted][j][i], &pos);
            x += (length + 1) * 16;
        }
        y += 16 * 2;
    }
    SDL_SetRenderDrawColor(term.renderer, 255, 255, 255, 255);
}

int KEYB_MoveCursor(int dx, int dy) {
    static int visual_offset = 0;

    if (!active) return 0;

    if (dy == 1) {
        if (selected_j > 0) {
            selected_i = KEYB_NewCol(visual_offset, selected_j, selected_j - 1);
            selected_j--;
        } else selected_j = 5;
        if (selected_i >= row_length[selected_j]) { selected_i = row_length[selected_j] - 1; }
    } else if (dy == -1) {
        if (selected_j < 5) {
            selected_i = KEYB_NewCol(visual_offset, selected_j, selected_j + 1);
            selected_j++;
        } else selected_j = 0;
        if (selected_i < 0) { selected_i = 0; }
    } else if (dx == -1) {
        if (selected_i > 0) selected_i--;
        else selected_i = row_length[selected_j] - 1;
        visual_offset = KEYB_VisualOffset(selected_i, selected_j);
    } else if (dx == 1) {
        if (selected_i < row_length[selected_j] - 1) selected_i++;
        else selected_i = 0;
        visual_offset = KEYB_VisualOffset(selected_i, selected_j);
    }
    return 1;
}
void KEYB_ClickKey() {
    int key = keys[shifted][selected_j][selected_i];
    if (mod_state & KMOD_CTRL) {
        if (key >= 64 && key < 64 + 32) KEYB_SimulateKey(key - 64, STATE_TYPED);
        else if (key >= 97 && key < 97 + 31) KEYB_SimulateKey(key - 96, STATE_TYPED);
    } else if ((mod_state & KMOD_SHIFT || mod_state & KMOD_CAPS) &&
               (key >= SDLK_a && key <= SDLK_z)) {
        KEYB_SimulateKey(key - SDLK_a + 'A', STATE_TYPED);
    } else if (key == SDLK_CAPSLOCK) {
        KEYB_ToggleMod();
    } else {
        KEYB_SimulateKey(key, STATE_TYPED);
    }
}
void KEYB_Toggle() { active = !active; }

void KEYB_SwitchLocation() { location = !location; }
void KEYB_Cycle_LocationActive() { 
    if(++m>3) m=1;
    location = (m>>1) & 1;
    active = (m) & 1;
    fprintf(stderr, "m=%d,location=%d,active=%d\n", m,location,active);
}

void KEYB_Shift(int state) {
    shifted       = state;
    toggled[4][0] = state;
    KEYB_UpdateModstate(SDLK_LSHIFT, state ? STATE_DOWN : STATE_UP);
}
void KEYB_ToggleMod() {
    toggled[selected_j][selected_i] = 1 - toggled[selected_j][selected_i];
    KEYB_SimulateKey(keys[shifted][selected_j][selected_i], 1 + toggled[selected_j][selected_i]);
    if (selected_j == 4 && (selected_i == 0 || selected_i == 11))
        shifted = toggled[selected_j][selected_i];
}

void KEYB_UpdateModstate(int key, int state) {
    // SDLMod mod_state = SDL_GetModState();
    if (state == STATE_DOWN) {
        if (key == SDLK_LSHIFT) mod_state |= KMOD_LSHIFT;
        else if (key == SDLK_RSHIFT) mod_state |= KMOD_RSHIFT;
        else if (key == SDLK_LCTRL) mod_state |= KMOD_LCTRL;
        else if (key == SDLK_RCTRL) mod_state |= KMOD_RCTRL;
        else if (key == SDLK_LALT) mod_state |= KMOD_LALT;
        else if (key == SDLK_RALT) mod_state |= KMOD_RALT;
        else if (key == SDLK_LGUI) mod_state |= KMOD_LGUI;
        else if (key == SDLK_RGUI) mod_state |= KMOD_RGUI;
        // else if(key == SDLK_NUM) mod_state |= KMOD_NUM;
        else if (key == SDLK_CAPSLOCK) mod_state |= KMOD_CAPS;
        else if (key == SDLK_MODE) mod_state |= KMOD_MODE;
    } else if (state == STATE_UP) {
        if (key == SDLK_LSHIFT) mod_state &= ~KMOD_LSHIFT;
        else if (key == SDLK_RSHIFT) mod_state &= ~KMOD_RSHIFT;
        else if (key == SDLK_LCTRL) mod_state &= ~KMOD_LCTRL;
        else if (key == SDLK_RCTRL) mod_state &= ~KMOD_RCTRL;
        else if (key == SDLK_LALT) mod_state &= ~KMOD_LALT;
        else if (key == SDLK_RALT) mod_state &= ~KMOD_RALT;
        else if (key == SDLK_LGUI) mod_state &= ~KMOD_LGUI;
        else if (key == SDLK_RGUI) mod_state &= ~KMOD_RGUI;
        // else if(key == SDLK_NUM) mod_state &= ~KMOD_NUM;
        else if (key == SDLK_CAPSLOCK) mod_state &= ~KMOD_CAPS;
        else if (key == SDLK_MODE) mod_state &= ~KMOD_MODE;
    }
    SDL_SetModState(mod_state);
}

void KEYB_SimulateKey(int key, int state) {
    KEYB_UpdateModstate(key, state);
    SDL_Event event = {.key = {.type   = SDL_KEYDOWN,
                               .state  = SDL_PRESSED,
                               .keysym = {
                                   .scancode = 0,
                                   .sym      = key,
                                   .mod      = KMOD_SYNTHETIC,
                               }}};
    if (state == STATE_TYPED) {
        if (key < 128) {
            char text[2] = {0};
            text[0]      = (char)key;  // Assuming key is an ASCII value
            event.type   = SDL_TEXTINPUT;
            strcpy(event.text.text, text);
        }
        SDL_PushEvent(&event);
        event.key.type  = SDL_KEYUP;
        event.key.state = SDL_RELEASED;
    } else if (state == STATE_UP) {
        event.key.type  = SDL_KEYUP;
        event.key.state = SDL_RELEASED;
    }
    SDL_PushEvent(&event);
    // printf("%d\n", key);
}

int KEYB_VisualOffset(int col, int row) {
    int sum = 0;
    for (int i = 0; i < col; i++) sum += 1 + strlen(syms[0][row][i]);
    sum += (1 + strlen(syms[0][row][col])) / 2;
    return sum;
}

int KEYB_NewCol(int visual_offset, int old_row, int new_row) {
    int new_sum = 0;
    int new_col = 0;
    while (new_col < row_length[new_row] - 1 &&
           new_sum + (1 + strlen(syms[0][new_row][new_col])) / 2 < visual_offset) {
        new_sum += 1 + strlen(syms[0][new_row][new_col]);
        new_col++;
    }
    return new_col;
}
