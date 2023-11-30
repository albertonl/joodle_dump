#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "tigr.h"

#define WIDTH 480
#define HEIGHT 640
#define BLOCK_MAXN 50

typedef struct {
    int tipo; // 1 -> bloque solido, 2 -> bloque que se rompe, 3 -> bloque movil, etc.
    int x, y;
    int w, h;
} Bloque;

void update(Tigr*, Tigr*, int[HEIGHT][WIDTH], int, int*, int*);
void drawScreen(Tigr*, Bloque*);
float min(float, float);

int main(int argc, char* argv[]) {
    Tigr* screen = tigrWindow(WIDTH, HEIGHT, "Joodle Dump", 0);
    Tigr* backdrop = tigrBitmap(WIDTH, HEIGHT);
    Tigr* player = tigrLoadImage("./img/obamium_min.png");

    int playerx = WIDTH/2.0f, playery = 0;
    int t0;

    int screen_matrix[HEIGHT][WIDTH];
    Bloque bloques[BLOCK_MAXN];

    // Initialize block matrix to 0
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

    if (!player) {
        tigrError(0, "Cannot load obamium_min.png");
    }

    while (!tigrClosed(screen) && !tigrKeyDown(screen, TK_ESCAPE)) {
        drawScreen(backdrop, bloques); // v = v0+a(t-t0)

        // PARA SACAR t0:
        // Cuando v > 0 y el monigote esté tocando un bloque

        int dt = tigrTime();
        update(screen, player, screen_matrix, dt, &playerx, &playery);

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

// Update the game
void update(Tigr* screen, Tigr* player, int matrix[HEIGHT][WIDTH], int dt, int* playerx, int* playery) {
    if (tigrKeyHeld(screen, TK_LEFT) || tigrKeyHeld(screen, 'A')) {
        *playerx -= 5;
    }
        // *playerx += dt * (-10) * exp(-10.0f * dt);
    if (tigrKeyHeld(screen, TK_RIGHT) || tigrKeyHeld(screen, 'D')) {
        *playerx += 5;
        // *playerx += dt * 10 * exp(-10.0f * dt);
    }

    if (*playerx < 0) *playerx = 0;
    else if (*playerx > WIDTH-(player->w)) *playerx = WIDTH-(player->w);
    
    if (tigrKeyHeld(screen, TK_SPACE)) {
        *playery -= 5;
        // *playery = dt * ((-20) * exp(-2.0f * dt) + dt * 20);
    } else if (
        matrix[*playery+(player->h)][*playerx+((player->w)/2)] == 0 ||
        (
            matrix[*playery+(player->h)][*playerx+((player->w)/2)] != 0 &&
            matrix[*playery+(player->h)-5][*playerx+((player->w)/2)] != 0
        )
    ) {
        *playery += 5;
    }

    if (*playery < 0) *playery = 0;
    else if (*playery > HEIGHT-(player->h)) *playery = HEIGHT-(player->h);
}

float min(float a, float b) {
    if (a < b) return a;
    return b;
}

void drawScreen(Tigr* backdrop, Bloque* bloques) {
    tigrClear(backdrop, tigrRGB(255, 255, 255));

    for (int i = 0; i < BLOCK_MAXN; i++) {
        if (bloques[i].tipo == 1) {
            tigrFill(backdrop, bloques[i].x, bloques[i].y, bloques[i].w, bloques[i].h, tigrRGB(0, 255, 0));
        }
    }
}