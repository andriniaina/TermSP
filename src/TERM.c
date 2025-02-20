#include "TERM.h"
#include "WHEEL.h"
#define PI 3.14159265358979323846264338327950288 /**< pi */

static VTermScreenCallbacks callbacks = {.movecursor = moveCursor,
                                         .sb_pushline = sb_pushline,
                                         .bell = bell,
                                         .damage = damage};
static void renderCursor();

/*###########################################################################*/
/*External Functions*/
/*###########################################################################*/

int childState = 0;

int TERM_Init()
{
#if defined(TEST)
  //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

#endif
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK))
    return -1;
  if (FOX_Init() != FOX_INITIALIZED)
    return -1;

  term.window =
      SDL_CreateWindow("TermSP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       cfg.width, cfg.height, SDL_WINDOW_SHOWN);
  term.renderer = SDL_CreateRenderer(
      term.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  // SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN, &term.window,
  //                             &term.renderer);
  //term.vscreen = SDL_GetWindowSurface(term.window);

  if (term.window == NULL)
  {
    return -1;
  }
  if (term.renderer == NULL)
  {
    SDL_DestroyWindow(term.window);
    return -1;
  }

  term.joystick = SDL_JoystickOpen(0);
  if (term.joystick == NULL)
  {
    fprintf(stderr, "Couldn't open joystick 0: %s\n", SDL_GetError());
  }
  else
  {
    printf("Joystick 0 opened successfully.\n");
  }
  SDL_JoystickEventState(SDL_ENABLE);

  term.keys = SDL_GetKeyboardState(NULL);
  SDL_StartTextInput();

  term.font.regular =
      FOX_OpenFont(term.renderer, cfg.fontpattern, cfg.fontsize);
  if (term.font.regular == NULL){
    fprintf(stderr, "Unable to open font cfg.fontpattern %s\n", cfg.fontpattern);
    return -1;
  }

  term.font.bold =
      FOX_OpenFont(term.renderer, cfg.boldfontpattern, cfg.fontsize);
  if (term.font.bold == NULL){
    fprintf(stderr, "Unable to open font cfg.boldfontpattern %s\n", cfg.boldfontpattern);
    return -1;
  }

  term.font.virt_kb_wheel = FOX_OpenFont(term.renderer, cfg.boldfontpattern, 24);
  term.font.virt_kb = FOX_OpenFont(term.renderer, cfg.boldfontpattern, 16);
  if (term.font.bold == NULL) {
    fprintf(stderr, "Unable to open font cfg.boldfontpattern %s\n", cfg.boldfontpattern);
    return -1;
  }

  term.pointer = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
  if (term.pointer)
    SDL_SetCursor(term.pointer);

  term.font.metrics = FOX_QueryFontMetrics(term.font.regular);
  term.ticks = SDL_GetTicks();
  term.cursor.visible = true;
  term.cursor.active = true;
  term.cursor.ticks = 0;
  term.bell.active = false;
  term.bell.ticks = 0;

  term.mouse.rect = (SDL_Rect){0};
  term.mouse.clicked = false;

  cfg.columns = cfg.width / (term.font.metrics->max_advance);
  cfg.rows = cfg.height / term.font.metrics->height;
  term.vterm = vterm_new(cfg.rows, cfg.columns);
  vterm_set_utf8(term.vterm, 1);
  term.screen = vterm_obtain_screen(term.vterm);
  vterm_screen_reset(term.screen, 1);
  term.termstate = vterm_obtain_state(term.vterm);
  vterm_screen_set_callbacks(term.screen, &callbacks, &term);

  char slave_name[128];
  term.child = forkpty(&term.childfd, slave_name, NULL, NULL);

  if (term.child < 0)
    return -1;
  else if (term.child == 0)
  {
    setenv("PS1", "\\[\\033[32m\\]\\w\\[\\033[00m\\]\\$ ", 1);
    updateLibPath("/userdata/system/TermSP");

    int slave_fd = open(slave_name, O_RDWR);
    struct termios terminal_attr;
    if (tcgetattr(slave_fd, &terminal_attr) == -1)
    {
      perror("tcgetattr");
      exit(EXIT_FAILURE);
    }
    terminal_attr.c_cc[VERASE] = 0x08;
    if (tcsetattr(slave_fd, TCSANOW, &terminal_attr) == -1)
    {
      perror("tcsetattr");
      close(slave_fd);
      exit(EXIT_FAILURE);
    }
    close(slave_fd);
    if (cfg.gnuscreen)
    {
      char **args = malloc(64 * sizeof(args));
      args[0] = "screen";
      args[1] = "-c";
      args[2] = SCREENRC;
      int i = 0;
      while (cfg.args && cfg.args[i])
      {
        args[i + 3] = cfg.args[i];
        i++;
      }
      args[i + 3] = NULL;
      execvp(args[0], args);
    }
    else if (!cfg.args)
    {
      char *env_shell = getenv("SHELL");
      if (env_shell == NULL)
        env_shell = "/bin/sh";
      char **args = (char *[]){env_shell, "-i", NULL};
      execvp(args[0], args);
    }
    else
    {
      execvp(cfg.args[0], cfg.args);
    }

    perror("execvp failed");
    exit(EXIT_FAILURE);
  }
  else
  {
    struct sigaction action = {0};
    action.sa_handler = signalHandler;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(SIGCHLD, &action, NULL);
    childState = 1;
  }

  TERM_Resize(cfg.width, cfg.height);
  term.dirty = true;
  return 0;
}

void TERM_Resize(int width, int height)
{
  term.font.metrics = FOX_QueryFontMetrics(term.font.regular);
  int cols = width / (term.font.metrics->max_advance);
  int rows = height / term.font.metrics->height;
  cfg.width = width;
  cfg.height = height;
  cfg.rows = rows;
  cfg.columns = cols;
  vterm_set_size(term.vterm, cfg.rows, cfg.columns);

  struct winsize ws = {0};
  ws.ws_col = cfg.columns;
  ws.ws_row = cfg.rows;
  ioctl(term.childfd, TIOCSWINSZ, &ws);
}

void TERM_Update()
{
  term.ticks = SDL_GetTicks();

  if (term.ticks > (term.cursor.ticks + cfg.cursorinterval))
  {
    term.cursor.ticks = term.ticks;
    term.cursor.visible = !term.cursor.visible;
    term.dirty = true;
  }

  if (term.bell.active && (term.ticks > (term.bell.ticks + 250)))
  {
    term.bell.active = false;
  }

  if (term.mouse.clicked)
  {
    SDL_RenderDrawRect(term.renderer, &term.mouse.rect);
  }

  if (term.dirty)
  {
    SDL_SetRenderDrawColor(term.renderer, 0, 0, 0, 255);
    SDL_RenderClear(term.renderer);
    renderScreen();
    if(cfg.WHEEL_enable)
      WHEEL_Draw();
    SDL_RenderPresent(term.renderer);
  }
}

void TERM_DeinitializeTerminal()
{
  pid_t wpid;
  int wstatus;
  kill(term.child, SIGKILL);
  do
  {
    wpid = waitpid(term.child, &wstatus, WUNTRACED | WCONTINUED);
    if (wpid == -1)
      break;
  } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
  term.child = wpid;
  vterm_free(term.vterm);
  FOX_CloseFont(term.font.bold);
  FOX_CloseFont(term.font.regular);
  FOX_CloseFont(term.font.virt_kb);
  FOX_CloseFont(term.font.virt_kb_wheel);
  SDL_JoystickClose(term.joystick);
  SDL_FreeCursor(term.pointer);
  SDL_DestroyRenderer(term.renderer);
  SDL_DestroyWindow(term.window);
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

int damage(VTermRect rect, void *user)
{
  // printf("damage: [%d, %d, %d, %d]\n", rect.start_col,
  //			rect.start_row, rect.end_col, rect.end_row);
  return 1;
}

int moveRect(VTermRect dest, VTermRect src, void *user) { return 1; }

int moveCursor(VTermPos pos, VTermPos oldpos, int visible, void *user)
{
  term.cursor.position.x = pos.col;
  term.cursor.position.y = pos.row;
  if (visible == 0)
  {
    // Works great for 'top' but not for 'nano'. Nano should have a cursor!
    // term.cursor.active = false;
  }
  else
    term.cursor.active = true;
  return 1;
}

int setTermProp(VTermProp prop, VTermValue *val, void *user) { return 1; }

int bell(void *user)
{
  term.bell.active = true;
  term.bell.ticks = term.ticks;
  return 1;
}

int sb_pushline(int cols, const VTermScreenCell *cells, void *user)
{
  return 1;
}

int sb_popline(int cols, VTermScreenCell *cells, void *user) { return 1; }

void renderCell(int x, int y)
{
  FOX_Font *font = term.font.regular;
  VTermScreenCell cell;
  VTermPos pos = {.row = y, .col = x};
  SDL_Point cursor = {x * term.font.metrics->max_advance,
                      y * term.font.metrics->height};

  vterm_screen_get_cell(term.screen, pos, &cell);
  Uint32 ch = cell.chars[0];
  if (ch == 0)
    return;

  vterm_state_convert_color_to_rgb(term.termstate, &cell.fg);
  vterm_state_convert_color_to_rgb(term.termstate, &cell.bg);
  SDL_Color fg;
  SDL_Color bg;
  if (cell.attrs.reverse)
  {
    fg = (SDL_Color){cell.bg.rgb.red, cell.bg.rgb.green, cell.bg.rgb.blue, 255};
    bg = (SDL_Color){cell.fg.rgb.red, cell.fg.rgb.green, cell.fg.rgb.blue, 255};
  }
  else
  {
    fg = (SDL_Color){cell.fg.rgb.red, cell.fg.rgb.green, cell.fg.rgb.blue, 255};
    bg = (SDL_Color){cell.bg.rgb.red, cell.bg.rgb.green, cell.bg.rgb.blue, 255};
  }
  if (bg.r + bg.g + bg.b != 765)
  {
    SDL_Rect rect = {cursor.x, cursor.y + 4, term.font.metrics->max_advance,
                     term.font.metrics->height};
    SDL_SetRenderDrawColor(term.renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(term.renderer, &rect);
  }

  if (cell.attrs.bold)
    font = term.font.bold;
  else if (cell.attrs.italic)
    ;

  SDL_SetRenderDrawColor(term.renderer, fg.r, fg.g, fg.b, fg.a);
  FOX_RenderChar(font, ch, 0, &cursor);
}

static void renderCursor()
{
  if (term.cursor.active && term.cursor.visible)
  {
    SDL_Rect rect = {term.cursor.position.x * term.font.metrics->max_advance,
                     4 + term.cursor.position.y * term.font.metrics->height, 4,
                     term.font.metrics->height};
    SDL_RenderFillRect(term.renderer, &rect);
  }
}

void renderScreen()
{
  SDL_SetRenderDrawColor(term.renderer, 255, 255, 255, 255);
  for (unsigned y = 0; y < cfg.rows; y++)
  {
    for (unsigned x = 0; x < cfg.columns; x++)
    {
      renderCell(x, y);
    }
  }

  renderCursor();
  KEYB_RenderVirtualKeyboard();
  term.dirty = false;

  if (term.bell.active)
  {
    SDL_Rect rect = {0, 0, cfg.width, cfg.height};
    SDL_RenderDrawRect(term.renderer, &rect);
  }
}
void updateLibPath(const char *new_path)
{
  const char *env_var = "LD_LIBRARY_PATH";
  char *old_value = getenv(env_var);
  size_t new_len;
  char *new_value;

  if (old_value)
  {
    new_len = strlen(new_path) + strlen(old_value) + 2; // +2 for ':' and '\0'
    new_value = malloc(new_len);
    if (!new_value)
    {
      perror("malloc");
    }
    snprintf(new_value, new_len, "%s:%s", new_path, old_value);
  }
  else
  {
    new_len = strlen(new_path) + 1;
    new_value = malloc(new_len);
    if (!new_value)
    {
      perror("malloc");
    }
    strcpy(new_value, new_path);
  }

  if (setenv(env_var, new_value, 1) != 0)
  {
    perror("setenv");
  }
  free(new_value);
}
