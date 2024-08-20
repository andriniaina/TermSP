#include "TERM.h"

static VTermScreenCallbacks callbacks = {
    .movecursor = moveCursor, .sb_pushline = sb_pushline, .bell = bell, .damage = damage};
static void renderCursor(TERM_State *state);

/*###########################################################################*/
/*External Functions*/
/*###########################################################################*/

int childState = 0;

int TERM_Init(TERM_State *state, TERM_Config *cfg) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) return -1;
    if (FOX_Init() != FOX_INITIALIZED) return -1;

    SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN, &state->window, &state->renderer);

    if (state->window == NULL) { return -1; }
    if (state->renderer == NULL) {
        SDL_DestroyWindow(state->window);
        return -1;
    }

    state->joystick = SDL_JoystickOpen(0);
    if (state->joystick == NULL) {
        fprintf(stderr, "Couldn't open joystick 0: %s\n", SDL_GetError());
    } else {
        printf("Joystick 0 opened successfully.\n");
    }
    SDL_JoystickEventState(SDL_ENABLE);

    state->keys = SDL_GetKeyboardState(NULL);
    SDL_StartTextInput();

    state->font.regular = FOX_OpenFont(state->renderer, cfg->fontpattern, cfg->fontsize);
    if (state->font.regular == NULL) return -1;

    state->font.bold = FOX_OpenFont(state->renderer, cfg->boldfontpattern, cfg->fontsize);
    if (state->font.bold == NULL) return -1;

    state->font.virt_kb = FOX_OpenFont(state->renderer, cfg->boldfontpattern, 16);
    if (state->font.bold == NULL) return -1;

    state->pointer = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
    if (state->pointer) SDL_SetCursor(state->pointer);

    state->font.metrics   = FOX_QueryFontMetrics(state->font.regular);
    state->ticks          = SDL_GetTicks();
    state->cursor.visible = true;
    state->cursor.active  = true;
    state->cursor.ticks   = 0;
    state->bell.active    = false;
    state->bell.ticks     = 0;

    state->mouse.rect    = (SDL_Rect){0};
    state->mouse.clicked = false;

    state->vterm = vterm_new(cfg->rows, cfg->columns);
    vterm_set_utf8(state->vterm, 1);
    state->screen = vterm_obtain_screen(state->vterm);
    vterm_screen_reset(state->screen, 1);
    state->termstate = vterm_obtain_state(state->vterm);
    vterm_screen_set_callbacks(state->screen, &callbacks, state);

    state->cfg = *cfg;

    state->child = forkpty(&state->childfd, NULL, NULL, NULL);
    if (state->child < 0) return -1;
    else if (state->child == 0) {
        setenv("PS1", "\\[\\033[32m\\]\\w\\[\\033[00m\\]\\$ ", 1);
        updateLibPath("/mnt/SDCARD/Apps/Terminal/lib");

        char *env_shell = getenv("SHELL");
        if (env_shell == NULL) env_shell = "/bin/sh";
        char **args = cfg->args ? cfg->args : (char *[]){env_shell, "-i", NULL};

        execvp(args[0], args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        struct sigaction action = {0};
        action.sa_handler       = signalHandler;
        action.sa_flags         = 0;
        sigemptyset(&action.sa_mask);
        sigaction(SIGCHLD, &action, NULL);
        childState = 1;
    }

    TERM_Resize(state, cfg->width, cfg->height);
    state->dirty = true;
    return 0;
}

void TERM_Resize(TERM_State *state, int width, int height) {
    int cols          = width / (state->font.metrics->max_advance);
    int rows          = height / state->font.metrics->height;
    state->cfg.width  = width;
    state->cfg.height = height;
    if (rows != state->cfg.rows || cols != state->cfg.columns) {
        state->cfg.rows    = rows;
        state->cfg.columns = cols;
        vterm_set_size(state->vterm, state->cfg.rows, state->cfg.columns);

        struct winsize ws = {0};
        ws.ws_col         = state->cfg.columns;
        ws.ws_row         = state->cfg.rows;
        ioctl(state->childfd, TIOCSWINSZ, &ws);
    }
}

void TERM_Update(TERM_State *state) {
    state->ticks = SDL_GetTicks();

    if (state->dirty) {
        SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
        SDL_RenderClear(state->renderer);
        renderScreen(state);
    }

    if (state->ticks > (state->cursor.ticks + 250)) {
        state->cursor.ticks   = state->ticks;
        state->cursor.visible = !state->cursor.visible;
        state->dirty          = true;
    }

    if (state->bell.active && (state->ticks > (state->bell.ticks + 250))) {
        state->bell.active = false;
    }

    if (state->mouse.clicked) { SDL_RenderDrawRect(state->renderer, &state->mouse.rect); }

    SDL_RenderPresent(state->renderer);
}

void TERM_DeinitializeTerminal(TERM_State *state) {
    pid_t wpid;
    int   wstatus;
    kill(state->child, SIGKILL);
    do {
        wpid = waitpid(state->child, &wstatus, WUNTRACED | WCONTINUED);
        if (wpid == -1) break;
    } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
    state->child = wpid;
    vterm_free(state->vterm);
    FOX_CloseFont(state->font.bold);
    FOX_CloseFont(state->font.regular);
    FOX_CloseFont(state->font.virt_kb);
    SDL_JoystickClose(state->joystick);
    SDL_FreeCursor(state->pointer);
    SDL_DestroyRenderer(state->renderer);
    SDL_DestroyWindow(state->window);
    FOX_Exit();
    SDL_Quit();
}

/*###########################################################################*/
/*Internal Functions*/
/*###########################################################################*/

void signalHandler(int signum) { childState = 0; }

/*###########################################################################*/
/* VTerm Callbacks */
/*###########################################################################*/

int damage(VTermRect rect, void *user) {
    // printf("damage: [%d, %d, %d, %d]\n", rect.start_col,
    //			rect.start_row, rect.end_col, rect.end_row);
    return 1;
}

int moveRect(VTermRect dest, VTermRect src, void *user) { return 1; }

int moveCursor(VTermPos pos, VTermPos oldpos, int visible, void *user) {
    TERM_State *state        = (TERM_State *)user;
    state->cursor.position.x = pos.col;
    state->cursor.position.y = pos.row;
    if (visible == 0) {
        // Works great for 'top' but not for 'nano'. Nano should have a cursor!
        // state->cursor.active = false;
    } else state->cursor.active = true;
    return 1;
}

int setTermProp(VTermProp prop, VTermValue *val, void *user) { return 1; }

int bell(void *user) {
    TERM_State *state  = (TERM_State *)user;
    state->bell.active = true;
    state->bell.ticks  = state->ticks;
    return 1;
}

int sb_pushline(int cols, const VTermScreenCell *cells, void *user) { return 1; }

int sb_popline(int cols, VTermScreenCell *cells, void *user) { return 1; }

void renderCell(TERM_State *state, int x, int y) {
    FOX_Font       *font = state->font.regular;
    VTermScreenCell cell;
    VTermPos        pos = {.row = y, .col = x};
    SDL_Point cursor    = {x * state->font.metrics->max_advance, y * state->font.metrics->height};

    vterm_screen_get_cell(state->screen, pos, &cell);
    Uint32 ch = cell.chars[0];
    if (ch == 0) return;

    vterm_state_convert_color_to_rgb(state->termstate, &cell.fg);
    vterm_state_convert_color_to_rgb(state->termstate, &cell.bg);
    SDL_Color color = {cell.fg.rgb.red, cell.fg.rgb.green, cell.fg.rgb.blue, 255};
    if (cell.attrs.reverse) {
        SDL_Rect rect = {cursor.x, cursor.y + 4, state->font.metrics->max_advance,
                         state->font.metrics->height};
        SDL_SetRenderDrawColor(state->renderer, color.r, color.g, color.b, color.a);
        color.r = ~color.r;
        color.g = ~color.g;
        color.b = ~color.b;
        SDL_RenderFillRect(state->renderer, &rect);
    }

    if (cell.attrs.bold) font = state->font.bold;
    else if (cell.attrs.italic)
        ;

    SDL_SetRenderDrawColor(state->renderer, color.r, color.g, color.b, color.a);
    FOX_RenderChar(font, ch, 0, &cursor);
}

static void renderCursor(TERM_State *state) {
    if (state->cursor.active && state->cursor.visible) {
        SDL_Rect rect = {state->cursor.position.x * state->font.metrics->max_advance,
                         4 + state->cursor.position.y * state->font.metrics->height, 4,
                         state->font.metrics->height};
        SDL_RenderFillRect(state->renderer, &rect);
    }
}

void renderScreen(TERM_State *state) {
    SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, 255);
    for (unsigned y = 0; y < state->cfg.rows; y++) {
        for (unsigned x = 0; x < state->cfg.columns; x++) { renderCell(state, x, y); }
    }

    renderCursor(state);
    KEYB_RenderVirtualKeyboard(state);
    state->dirty = false;

    if (state->bell.active) {
        SDL_Rect rect = {0, 0, state->cfg.width, state->cfg.height};
        SDL_RenderDrawRect(state->renderer, &rect);
    }
}
void updateLibPath(const char *new_path) {
    const char *env_var   = "LD_LIBRARY_PATH";
    char       *old_value = getenv(env_var);
    size_t      new_len;
    char       *new_value;

    if (old_value) {
        new_len   = strlen(new_path) + strlen(old_value) + 2;  // +2 for ':' and '\0'
        new_value = malloc(new_len);
        if (!new_value) { perror("malloc"); }
        snprintf(new_value, new_len, "%s:%s", new_path, old_value);
    } else {
        new_len   = strlen(new_path) + 1;
        new_value = malloc(new_len);
        if (!new_value) { perror("malloc"); }
        strcpy(new_value, new_path);
    }

    if (setenv(env_var, new_value, 1) != 0) {
        perror("setenv");
        free(new_value);
    }
    free(new_value);
}
