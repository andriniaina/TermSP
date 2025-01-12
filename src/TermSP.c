#include "TermSP.h"

TERM_State term = {0};
TERM_Config cfg = {
    .args = NULL,
    .fontpattern = "/mnt/SDCARD/Apps/Terminal/resources/Hack-Regular.ttf",
    .boldfontpattern = "/mnt/SDCARD/Apps/Terminal/resources/Hack-Bold.ttf",
    .virtkb = 0,
    .refreshrate = 30,
    .cursorinterval = 250,
    .fontsize = 18,
    .width = 0,
    .height = 0,
    .rows = 0,
    .columns = 0,
    .gnuscreen = 0};

int parseArgs(int argc, char **argv);
int main(int argc, char *argv[]) {
  int fh = open("/dev/fb0", O_RDONLY);
  if (fh < 0) {
    fprintf(stderr, "Couldn't open framebuffer.\n");
    return -1;
  }
  struct fb_var_screeninfo vinfo;
  ioctl(fh, FBIOGET_VSCREENINFO, &vinfo);
  close(fh);
  cfg.width = vinfo.xres;
  cfg.height = vinfo.yres;

  if (parseArgs(argc, argv)) return -1;
  if (TERM_Init()) return -1;
  if (KEYB_Init()) return -1;

  while (!EV_HandleEvents()) {
    TERM_Update();
    SDL_Delay(1000 / cfg.refreshrate);
  }

  TERM_DeinitializeTerminal();
  return 0;
}

static const char help[] = {
    "sdlterm usage:\n"
    "\tsdlterm [option...]* [child options...]*\n"
    "Options:\n"
    "  -h\tDisplay help text\n"
    "  -S\tStart with gnu-screen\n"
    "  -k\tShow virtual keyboard at start\n"
    "  -r\tSet the refresh rate (default 30hz)\n"
    "  -c\tSet the cursor blinking interval (default 250)\n"
    "  -f\tSet regular font via path (fontconfig pattern not yet supported)\n"
    "  -b\tSet bold font via path (fontconfig pattern not yet supported)\n"
    "  -s\tSet fontsize\n"
    "  -e\tSet child process executable path\n"};

static const char options[] = "hSkr:c:f:b:s:e:";
extern char *optarg;
extern int optind;

int parseArgs(int argc, char **argv) {
  int option;
  int status = 0;

  while ((option = getopt(argc, argv, options)) != -1) {
    switch (option) {
      case 'h':
        puts(help);
        status = 1;
        break;
      case 'S':
        cfg.gnuscreen = 1;
        break;
      case 'k':
        cfg.virtkb = 1;
        break;
      case 'r':
        if (optarg != NULL) cfg.refreshrate = atoi(optarg);
        break;
      case 'c':
        if (optarg != NULL) cfg.cursorinterval = atoi(optarg);
        break;
      case 'f':
        if (optarg != NULL) cfg.fontpattern = optarg;
        break;
      case 'b':
        if (optarg != NULL) cfg.boldfontpattern = optarg;
        break;
      case 's':
        if (optarg != NULL) cfg.fontsize = atoi(optarg);
        break;
      case 'e':
        if (optarg != NULL) cfg.args = &argv[optind - 1];
        optind = argc;
        break;
      default:
        status = 1;
        break;
    }
  }
  return status;
}

void swap(int *a, int *b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}
