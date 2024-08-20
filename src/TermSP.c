#include "TermSP.h"

int parseArgs(TERM_Config *cfg, int argc, char **argv);
int main(int argc, char *argv[]) {
    TERM_State  state;
    TERM_Config cfg = {
        .args            = NULL,
        .fontpattern     = "/mnt/SDCARD/Apps/Terminal/fonts/DejaVuSansMono.ttf",
        .boldfontpattern = "/mnt/SDCARD/Apps/Terminal/fonts/DejaVuSansMono-Bold.ttf",
        .virtkb          = 0,
        .refreshrate     = 30,
        .fontsize        = 18,
        .width           = 1280,
        .height          = 720,
        .rows            = 0,
        .columns         = 0};
    if (parseArgs(&cfg, argc, argv)) return -1;
    if (TERM_Init(&state, &cfg)) return -1;
    if (KEYB_Init(&state, &cfg)) return -1;

    while (!EV_HandleEvents(&state)) {
        TERM_Update(&state);
        SDL_Delay(1000 / cfg.refreshrate);
    }

    TERM_DeinitializeTerminal(&state);
    return 0;
}

static const char help[] = {
    "sdlterm usage:\n"
    "\tsdlterm [option...]* [child options...]*\n"
    "Options:\n"
    "  -h\tDisplay help text\n"
    "  -k\tShow virtual keyboard at start\n"
    "  -r\tSet the refresh rate (default 20hz)\n"
    "  -f\tSet regular font via path (fontconfig pattern not yet supported)\n"
    "  -b\tSet bold font via path (fontconfig pattern not yet supported)\n"
    "  -s\tSet fontsize\n"
    "  -e\tSet child process executable path\n"};

static const char options[] = "hkmcf:s:r:b:e:";
extern char      *optarg;
extern int        optind;

int parseArgs(TERM_Config *cfg, int argc, char **argv) {
    int option;
    int status = 0;

    while ((option = getopt(argc, argv, options)) != -1) {
        switch (option) {
            case 'h':
                puts(help);
                status = 1;
                break;
            case 'k': cfg->virtkb = 1; break;
            case 'r':
                if (optarg != NULL) cfg->refreshrate = atoi(optarg);
                break;
            case 'f':
                if (optarg != NULL) cfg->fontpattern = optarg;
                break;
            case 'b':
                if (optarg != NULL) cfg->boldfontpattern = optarg;
                break;
            case 's':
                if (optarg != NULL) cfg->fontsize = atoi(optarg);
                break;
            case 'e':
                if (optarg != NULL) cfg->args = &argv[optind - 1];
                optind = argc;
                break;
            default: status = 1; break;
        }
    }

    return status;
}

void swap(int *a, int *b) {
    int tmp = *a;
    *a      = *b;
    *b      = tmp;
}
