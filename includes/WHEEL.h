#define AXIS_LEFT_X 0
#define AXIS_LEFT_Y 1
#define AXIS_RIGHT_X 3
#define AXIS_RIGHT_Y 4
#define TRIGGER_THRESHOLD 300

int WHEEL_init();
int WHEEL_deinit();
int WHEEL_GetSelectedCharIndexLeft();
int WHEEL_GetSelectedCharIndexRight();
void WHEEL_Draw();
