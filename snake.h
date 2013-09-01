
#define UD_INTERVAL 1

#define UP 0
#define RIGHT 1
#define LEFT 2
#define DOWN 3

#define BODY_MAX_LEN 1920


void quit(struct termios* termios_p);

void initBody();
void printBody();

void checkGameOver();
void deploySnack(int height, int width);

void slither(char lengthen);

char bodyContains(int x, int y);