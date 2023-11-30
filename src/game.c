/*
    Joodle Dump
    -----------
    Alberto Navalón Lillo
    Lucía Pelegrín Várez
    Israel León Iborra

    Programming 1 - Final Project
    December 2023
*/

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <sys/time.h>
#include "tigr.h"

#define WIDTH 480
#define HEIGHT 640
#define BLOCK_MAXN 50

typedef struct {
    int tipo; // 1 -> bloque solido, 2 -> bloque que se rompe, 3 -> bloque movil, etc.
    int x, y;
    int w, h;
} Bloque;

// Periodical functions
void update(Tigr*, Tigr*, int[HEIGHT][WIDTH], int, int, float*, float*);
void drawScreen(Tigr*, Bloque*);

// State checkers
bool isOnPlatform(Tigr*, int[HEIGHT][WIDTH], float, float);

// Utilities
float min(float, float);
int timeInMilliseconds();

int main(int argc, char* argv[]) {
    Tigr* screen = tigrWindow(WIDTH, HEIGHT, "Joodle Dump", 0);
    Tigr* backdrop = tigrBitmap(WIDTH, HEIGHT);
    Tigr* player = tigrLoadImage("./img/obamium_min.png");

    if (!player) {
        tigrError(0, "Cannot load obamium_min.png");
    }

    float playerx = WIDTH/2, playery = HEIGHT-(player->h);
    int t, t0;

    t = t0 = timeInMilliseconds();

    int screen_matrix[HEIGHT][WIDTH];
    Bloque bloques[BLOCK_MAXN];

    // Initialize block array to 0
    for (int i = 0; i < BLOCK_MAXN; i++) {
        bloques[i].tipo = 0;
    }

    // Initialize matrix to 0
    for (int i = 0; i < HEIGHT-1; i++) {
        for (int j = 0; j < WIDTH; j++) {
            screen_matrix[i][j] = 0;
        }
    }
    
    // Bottom solid line
    for (int i = HEIGHT-50; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            screen_matrix[i][j] = 1;
        }
    }

    bloques[0].tipo = 1;
    bloques[0].x = 0;
    bloques[0].y = HEIGHT-50;
    bloques[0].h = 50;
    bloques[0].w = WIDTH;
    
    for (int i = HEIGHT-150; i <= HEIGHT-120; i++) {
        for (int j = 40; j <= 140; j++) {
            screen_matrix[i][j] = 1;
        }
    }

    bloques[1].tipo = 1;
    bloques[1].x = 40;
    bloques[1].y = HEIGHT-150;
    bloques[1].w = 100;
    bloques[1].h = 30;

    while (!tigrClosed(screen) && !tigrKeyDown(screen, TK_ESCAPE)) {
        drawScreen(backdrop, bloques); // v = v0+a(t-t0)

        // PARA SACAR t0:
        // Cuando v > 0 y el monigote esté tocando un bloque

        t = timeInMilliseconds();
        if (isOnPlatform(player, screen_matrix, playerx, playery)) {
            playery--;
            t0 = t;
        }

        printf("x: %f\ty: %f\t", playerx, playery);

        update(screen, player, screen_matrix, t0, t, &playerx, &playery);

        // Composite the backdrop and sprite onto the screen.
        tigrBlit(screen, backdrop, 0, 0, 0, 0, WIDTH, HEIGHT);
        tigrBlitAlpha(screen, player, playerx, playery, 0, 0, player->w,
                      player->h, 1.0f);

        // tigrClear(screen, tigrRGB(0x80, 0x90, 0xa0));
        // tigrPrint(screen, tfont, 120, 110, tigrRGB(0xff, 0xff, 0xff), "Hello, world.");
        tigrUpdate(screen);
    }

    tigrFree(screen);
    tigrFree(player);
    return 0;
}

/*
    Procedure: update
    -----------------
    Updates the game state. It is called every tick.

    screen: the Tigr* variable representing the screen.
    player: the Tigr* variable representing the moveable sprite.
    matrix: a matrix with HEIGHT rows and WIDTH cols representing all the pixels in the screen.
    t0: timestamp of the last time the sprite was in contact with a solid block.
    dt: current timestamp.
    playerx: pointer to the X position (left side) of the sprite.
    playery: pointer to the Y position (upper side) of the sprite.
*/
void update(Tigr* screen, Tigr* player, int matrix[HEIGHT][WIDTH], int t0, int dt, float* playerx, float* playery) {
    // X-axis movement (check if key is pressed and move accordingly)
    if (tigrKeyHeld(screen, TK_LEFT) || tigrKeyHeld(screen, 'A'))
        *playerx -= 5; // move 5 pixels to the left
    if (tigrKeyHeld(screen, TK_RIGHT) || tigrKeyHeld(screen, 'D'))
        *playerx += 5; // move 5 pixels to the right

    // Correct collisions with side walls
    if (*playerx < 0) *playerx = 0;
    else if (*playerx > WIDTH-(player->w)) *playerx = WIDTH-(player->w);

    // Y-axis movement (simple harmonic motion)
    // y(t) = y0 - 20 * cos(2*pi*(t-t0))

    if (!isOnPlatform(player, matrix, *playerx, *playery)) {
        *playery = (2*HEIGHT/3)-fabs(150*cos(M_PI*0.001*(dt-t0)));
        printf("call timestamp: %d\n", dt);
    }

    // if (tigrKeyHeld(screen, TK_SPACE)) {
    //     *playery -= 5;
    //     // *playery = dt * ((-20) * exp(-2.0f * dt) + dt * 20);
    // } else if (
    //     matrix[*playery+(player->h)][*playerx+((player->w)/2)] == 0 ||
    //     (
    //         matrix[*playery+(player->h)][*playerx+((player->w)/2)] != 0 &&
    //         matrix[*playery+(player->h)-5][*playerx+((player->w)/2)] != 0
    //     )
    // ) {
    //     *playery += 5;
    // }

    // Correct collisions with top and bottom ends of the screen
    if (*playery < 0) *playery = 0;
    else if (*playery > HEIGHT-(player->h)) *playery = HEIGHT-(player->h);
}

float min(float a, float b) {
    if (a < b) return a;
    return b;
}

/*
    Function: timeInMilliseconds
    ----------------------------
    Returns the current POSIX timestamp in milliseconds.
    Adapted from https://stackoverflow.com/a/44896326 (CC BY-SA 3.0)

    returns: the timestamp in milliseconds.
*/
int timeInMilliseconds() {
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (((int)tv.tv_sec)*1000+(tv.tv_usec)/1000);
}

void drawScreen(Tigr* backdrop, Bloque* bloques) {
    tigrClear(backdrop, tigrRGB(255, 255, 255));

    for (int i = 0; i < BLOCK_MAXN; i++) {
        if (bloques[i].tipo == 1) {
            tigrFill(backdrop, bloques[i].x, bloques[i].y, bloques[i].w, bloques[i].h, tigrRGB(0, 255, 0));
        }
    }
}

bool isOnPlatform(Tigr* player, int matrix[HEIGHT][WIDTH], float playerx, float playery) {
    int px = (int)playerx, py = (int)playery;
    if (matrix[py+(player->h)][px+((player->w)/2)] != 0
        && matrix[py+(player->h)-5][px+((player->w)/2)] == 0)
        return true;
    return false;
}