#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "tigr.h"

#define WIDTH 480
#define HEIGHT 640

void update(Tigr*, Tigr*, float, float*, float*);
float min(float, float);

int main(int argc, char* argv[]) {
    Tigr* screen = tigrWindow(WIDTH, HEIGHT, "Joodle Dump", 0);
    Tigr* backdrop = tigrBitmap(WIDTH, HEIGHT);
    Tigr* player = tigrLoadImage("./img/obamium_min.png");

    float playerx = WIDTH/2.0f, playery = 0;

    if (!player) {
        tigrError(0, "Cannot load obamium_min.png");
    }

    while (!tigrClosed(screen) && !tigrKeyDown(screen, TK_ESCAPE)) {
        float dt = tigrTime();
        update(screen, player, dt, &playerx, &playery);

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
void update(Tigr* screen, Tigr* player, float dt, float* playerx, float* playery) {

    if (tigrKeyHeld(screen, TK_LEFT) || tigrKeyHeld(screen, 'A'))
        *playerx -= 5;
        // *playerx += dt * (-10) * exp(-10.0f * dt);
    if (tigrKeyHeld(screen, TK_RIGHT) || tigrKeyHeld(screen, 'D'))
        *playerx += 5;
        // *playerx += dt * 10 * exp(-10.0f * dt);
    
    if (tigrKeyHeld(screen, TK_SPACE)) {
        *playery -= 20;
        // *playery = dt * ((-20) * exp(-2.0f * dt) + dt * 20);
    } else if (*playery < HEIGHT-(player->h)) {
        *playery = min(HEIGHT-(player->h), *playery + 10);
        // *playery = min((float)HEIGHT-(player->h), dt * (exp(-2.0f * dt) + dt * 20));
    }

    if (*playerx < 0) *playerx = 0;
    else if (*playerx > WIDTH-(player->w)) *playerx = WIDTH-(player->w);
}

float min(float a, float b) {
    if (a < b) return a;
    return b;
}