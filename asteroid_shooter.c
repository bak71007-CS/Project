/*
    Asteroid Shooter Game in C.
    Controls: A/D to move, SPACE to shoot, ESC to quit.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>  
#include <conio.h>     

/* Game settings */
#define FIELD_W        40
#define FIELD_H        20
#define MAX_ASTEROIDS  10
#define MAX_BULLETS    10
#define FRAME_MS          60
#define BULLET_SPEED       3  
#define HIT_POINTS        10

/* Asteroid movement speeds up as the score increases. */
#define SPEED_DELAY_MAX    6   
#define SPEED_DELAY_MIN    1   
#define SPAWN_MAX          8   
#define SPAWN_MIN          2   

/* Limits for active asteroids on screen. */
#define ASTEROID_CAP_MIN   2   
#define ASTEROID_CAP_MAX  10   

#define KEY_LEFT  'a'
#define KEY_RIGHT 'd'
#define KEY_SHOOT ' '
#define KEY_ESC    27
#define KEY_R     'r'

/* Structures */

typedef struct 
{
    int x, y;
    int active;
} Asteroid;

typedef struct 
{
    int x, y;
} Bullet;

/* Stack to manage active bullets. */
typedef struct 
{
    Bullet items[MAX_BULLETS];
    int    top;
} BulletStack;

/* Main game */
typedef struct 
{
    int         playerX;
    int         score;
    int         frame;
    int         over;
    BulletStack bstack;
    Asteroid    asteroids[MAX_ASTEROIDS];
} GameState;

/* Console help functions */

void GotoXY(int col, int row) 
{
    COORD pos;
    pos.X = (SHORT)col;
    pos.Y = (SHORT)row;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void HideCursor(void) 
{
    CONSOLE_CURSOR_INFO c;
    c.dwSize   = 1;
    c.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &c);
}

void RestoreCursor(void) 
{
    CONSOLE_CURSOR_INFO c;
    c.dwSize   = 1;
    c.bVisible = TRUE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &c);
}

/* Stack functions */

void PushBullet(BulletStack *bs, int x, int y)
{
    if (bs->top < MAX_BULLETS) {
        bs->items[bs->top].x = x;
        bs->items[bs->top].y = y;
        bs->top++;
    }
}

/* Fast removal by swapping with the last item. */
void RemoveBullet(BulletStack *bs, int i) 
{
    bs->top--;
    if (i != bs->top)
        bs->items[i] = bs->items[bs->top];
}

/* Initialization */

void Setup(GameState *gs) 
{
    int i;
    srand((unsigned int)time(NULL));

    gs->playerX    = FIELD_W / 2;
    gs->score      = 0;
    gs->frame      = 0;
    gs->over       = 0;
    gs->bstack.top = 0;

    for (i = 0; i < MAX_ASTEROIDS; i++)
        gs->asteroids[i].active = 0;
}

/* Input handling */

/* Read keyboard input instantly without blocking. */
void Input(GameState *gs)
{
    char key;
    while (_kbhit()) {
        key = (char)_getch();
        if (key >= 'A' && key <= 'Z') key += 32;

        if      (key == KEY_LEFT  && gs->playerX > 1)
            gs->playerX--;
        else if (key == KEY_RIGHT && gs->playerX < FIELD_W - 2)
            gs->playerX++;
        else if (key == KEY_SHOOT)
            PushBullet(&gs->bstack, gs->playerX, FIELD_H - 2);
        else if (key == KEY_ESC)
            gs->over = 1;
    }
}

/* Game logic */

/* Calculate delay between asteroid movements based on score. */
int GetAsteroidDelay(int score) 
{
    int range = SPEED_DELAY_MAX - SPEED_DELAY_MIN;
    int delay = SPEED_DELAY_MAX - (range * score) / (score + 50);
    if (delay < SPEED_DELAY_MIN) delay = SPEED_DELAY_MIN;
    return delay;
}

/* Calculate how often asteroids spawn based on score. */
int GetSpawnRate(int score) 
{
    int range = SPAWN_MAX - SPAWN_MIN;
    int rate  = SPAWN_MAX - (range * score) / (score + 80);
    if (rate < SPAWN_MIN) rate = SPAWN_MIN;
    return rate;
}

/* Calculate max allowed asteroids on screen based on score. */
int GetAsteroidCap(int score) 
{
    int range = ASTEROID_CAP_MAX - ASTEROID_CAP_MIN;
    int cap   = ASTEROID_CAP_MIN + (range * score) / (score + 60);
    if (cap > ASTEROID_CAP_MAX) cap = ASTEROID_CAP_MAX;
    return cap;
}

/* Spawn a new asteroid if below the current limit. */
void SpawnAsteroid(Asteroid *arr, int count, int cap) 
{
    int i;
    int alive;

    /* Count active asteroids. */
    alive = 0;
    for (i = 0; i < count; i++)
        if (arr[i].active) alive++;

    /* Stop if cap reached. */
    if (alive >= cap) return;

    /* Find an empty spot to spawn. */
    for (i = 0; i < count; i++) {
        if (!arr[i].active) {
            arr[i].x      = rand() % FIELD_W;
            arr[i].y      = 0;
            arr[i].active = 1;
            break;
        }
    }
}

void CheckCollisions(GameState *gs) 
{
    int i, j, row;
    BulletStack *bs  = &gs->bstack;
    Asteroid    *arr =  gs->asteroids;

    /* Check collision over the entire path the bullet moved this frame. */
    for (i = 0; i < bs->top; i++) 
    {
        for (j = 0; j < MAX_ASTEROIDS; j++) 
        {
            if (!arr[j].active) continue;
            if (bs->items[i].x != arr[j].x) continue;

            /* Check all skipped rows. */
            for (row = bs->items[i].y;
                 row <= bs->items[i].y + BULLET_SPEED - 1;
                 row++) 
                 {
                if (row == arr[j].y)
                {
                    arr[j].active = 0;
                    RemoveBullet(bs, i);
                    gs->score += HIT_POINTS;
                    i--;
                    goto next_bullet;
                }
            }
        }
        next_bullet:;
    }

    /* Check if asteroid hit the player ship. */
    for (j = 0; j < MAX_ASTEROIDS; j++) 
    {
        if (!arr[j].active) continue;
        if (arr[j].y >= FIELD_H - 1) 
        {
            if (arr[j].x >= gs->playerX - 1 &&
                arr[j].x <= gs->playerX + 1) 
                {
                gs->over = 1;
                return;
            }
            arr[j].active = 0;
        }
    }
}

void Logic(GameState *gs) 
{
    int i;
    int delay;
    int spawnRate;

    /* Move bullets up. */
    for (i = 0; i < gs->bstack.top; i++) 
    {
        gs->bstack.items[i].y -= BULLET_SPEED;
        if (gs->bstack.items[i].y < 0) 
        {
            RemoveBullet(&gs->bstack, i);
            i--;
        }
    }

    /* Move asteroids based on the calculated delay. */
    delay = GetAsteroidDelay(gs->score);
    if (gs->frame % delay == 0) {
        for (i = 0; i < MAX_ASTEROIDS; i++)
            if (gs->asteroids[i].active)
                gs->asteroids[i].y++;
    }

    CheckCollisions(gs);

    /* Spawn new asteroids based on current spawn rate. */
    spawnRate = GetSpawnRate(gs->score);
    if (gs->frame % spawnRate == 0)
        SpawnAsteroid(gs->asteroids, MAX_ASTEROIDS, GetAsteroidCap(gs->score));

    gs->frame++;
}

/* Rendering */

/* Overwrite the screen to avoid flickering. */
void Draw(const GameState *gs) 
{
    /* Declare variables at top. */
    char line[FIELD_W + 2];
    int  row, col, i;
    int  bx, by, ax, px;

    GotoXY(0, 0);

    printf("  Score: %-6d  Speed: %d  ESC = Quit     \n",
           gs->score,
           SPEED_DELAY_MAX - GetAsteroidDelay(gs->score) + 1);

    printf("+");
    for (col = 0; col < FIELD_W; col++) printf("-");
    printf("+\n");

    for (row = 0; row < FIELD_H; row++) 
    {
        memset(line, ' ', FIELD_W);
        line[FIELD_W] = '\0';

        /* Draw bullets. */
        for (i = 0; i < gs->bstack.top; i++) 
        {
            bx = gs->bstack.items[i].x;
            by = gs->bstack.items[i].y;
            if (by == row && bx >= 0 && bx < FIELD_W)
                line[bx] = '^';
        }

        /* Draw asteroids. */
        for (i = 0; i < MAX_ASTEROIDS; i++) 
        {
            if (gs->asteroids[i].active && gs->asteroids[i].y == row)
            {
                ax = gs->asteroids[i].x;
                if (ax >= 0 && ax < FIELD_W)
                    line[ax] = 'O';
            }
        }

        /* Draw player ship. */
        if (row == FIELD_H - 1) 
        {
            px = gs->playerX;
            if (px > 0 && px < FIELD_W - 1) 
            {
                line[px - 1] = '[';
                line[px]     = 'A';
                line[px + 1] = ']';
            }
        }

        printf("|%s|\n", line);
    }

    printf("+");
    for (col = 0; col < FIELD_W; col++) printf("-");
    printf("+\n");

    printf("  [A] Left   [D] Right   [SPACE] Shoot     \n");
}

/* Menu screens */

void ShowStartScreen(void) 
{
    system("cls");
    printf("\n\n");
    printf("   #############################################\n");
    printf("   #                                           #\n");
    printf("   #      A S T E R O I D   S H O O T E R     #\n");
    printf("   #                                           #\n");
    printf("   #          O  .  .  O  .  .  .  O          #\n");
    printf("   #        .  .  O  .  .  O  .  .  .         #\n");
    printf("   #                                           #\n");
    printf("   #               [A]  your ship              #\n");
    printf("   #                                           #\n");
    printf("   #############################################\n");
    printf("\n");
    printf("   HOW TO PLAY\n");
    printf("     A / D       move left / right\n");
    printf("     SPACEBAR    shoot\n");
    printf("     ESC         quit\n\n");
    printf("   Shoot asteroids before they reach you.\n");
    printf("   Each hit scores %d points.\n\n", HIT_POINTS);
    printf("   >>> Press any key to start <<<\n\n");
    while (!_kbhit());
    _getch();
}

/* Return 1 to restart, 0 to quit. */
int ShowGameOverScreen(int finalScore) 
{
    /* Declare variable at top. */
    char key;
    system("cls");
    printf("\n\n\n");
    printf("   +------------------------------------------+\n");
    printf("   |                                          |\n");
    printf("   |             ** GAME OVER  ** |\n");
    printf("   |                                          |\n");
    printf("   |          Final score :  %-6d           |\n", finalScore);
    printf("   |                                          |\n");
    printf("   |    R   = Restart                         |\n");
    printf("   |    ESC = Quit                            |\n");
    printf("   |                                          |\n");
    printf("   +------------------------------------------+\n\n");

    while (1) 
    {
        if (_kbhit()) 
        {
            key = (char)_getch();
            if (key >= 'A' && key <= 'Z') key += 32;
            if (key == KEY_R)   return 1;
            if (key == KEY_ESC) return 0;
        }
        Sleep(50);
    }
    return 0;   /* Required return for compiler. */
}


/*Main function*/
int main(void) 
{
    GameState gs;
    int restart = 1;

    HideCursor();
    ShowStartScreen();

    while (restart) 
    {
        Setup(&gs);
        system("cls");

        while (!gs.over) 
        {
            Input(&gs);
            Logic(&gs);
            Draw(&gs);
            Sleep(FRAME_MS);
        }

        restart = ShowGameOverScreen(gs.score);
    }

    RestoreCursor();
    system("cls");
    printf("\n  Thanks for playing!\n\n");

    return 0;
}