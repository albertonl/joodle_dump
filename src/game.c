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
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include "tigr.h"

#define WIDTH 480
#define HEIGHT 640
#define BLOCK_MAXN 10
#define JUMP_HEIGHT 200
#define PLATFORM_WIDTH 70
#define PLATFORM_HEIGHT 25
#define SCROLL_SPEED 20

#define height(x) x->h
#define width(x) x->w

typedef struct {
    int tipo; // 1 -> bloque solido, 2 -> bloque que se rompe, 3 -> bloque movil, etc.
    int x, y;
    int w, h;
} Bloque;

// Periodical functions
int update(Tigr*, Tigr*, int[HEIGHT][WIDTH], int, int, int*, float*, float*, Bloque*);
void drawScreen(Tigr*, Bloque*);
void generatePlatforms(Bloque*, int[HEIGHT][WIDTH]);
void bajarElementosPantalla(int, Bloque*);

// Movement functions
float y(int, int);
float dy(int, int);

// State checkers
bool isOnPlatform(Tigr*, int[HEIGHT][WIDTH], float, float);
bool isGoingUp(Tigr*, int, int);

// Utilities
float min(float, float);
float max(float, float);
int randrange(int, int);
int timeInMilliseconds();
void shiftBlockArray(Bloque*, int);

int main(int argc, char* argv[]) {
    Tigr* screen = tigrWindow(WIDTH, HEIGHT, "Joodle Dump", 0);
    Tigr* backdrop = tigrBitmap(WIDTH, HEIGHT);
    Tigr* player = tigrLoadImage("./img/obamium_min.png");

    // Fonts
    Tigr* smallFontImage = tigrLoadImage("./img/small_font.png");       // Size 20
    Tigr* regularFontImage = tigrLoadImage("./img/regular_font.png");   // Size 35
    Tigr* bigFontImage = tigrLoadImage("./img/big_font.png");           // Size 50
    TigrFont* smallFont = tigrLoadFont(smallFontImage, TCP_1252);
    TigrFont* regularFont = tigrLoadFont(regularFontImage, TCP_1252);
    TigrFont* bigFont = tigrLoadFont(bigFontImage, TCP_1252);

    if (!player) {
        tigrError(0, "Cannot load obamium_min.png");
    }

    float playerx = WIDTH/2, playery = HEIGHT-200.0f;
    int blocky = HEIGHT-60;
    int t, t0;

    long long score = 0;
    bool alive = true;
    bool restart;
    
    int screen_matrix[HEIGHT][WIDTH];
    Bloque bloques[BLOCK_MAXN];

    t = t0 = timeInMilliseconds();

    // Use current time as seed for random generator
    srand(time(NULL));

    do {
        restart = false;

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

        while (!tigrClosed(screen) && !tigrKeyDown(screen, TK_ESCAPE) && alive) {
            generatePlatforms(bloques, screen_matrix);
            drawScreen(backdrop, bloques);
            t = timeInMilliseconds();

            if (isOnPlatform(player, screen_matrix, playerx, playery) && !isGoingUp(player, t, t0)) {
                blocky = playery;
                t0 = t;
            }

            // Check if user is touching the bottom of the screen
            if (!isOnPlatform(player, screen_matrix, playerx, playery) && playery >= HEIGHT-height(player) && blocky < playery) {
                printf("YOU DIED. Your score: %lld\n", score);
                alive = false;
            } else {
                score += update(screen, player, screen_matrix, t0, t, &blocky, &playerx, &playery, bloques);
            }

            // Composite the backdrop and sprite onto the screen.
            tigrBlit(screen, backdrop, 0, 0, 0, 0, WIDTH, HEIGHT);
            tigrBlitAlpha(screen, player, playerx, playery, 0, 0, width(player),
                        height(player), 1.0f);
            
            tigrPrint(screen, smallFont, 30, 30, tigrRGB(0, 0, 0), "%lld", score);

            tigrUpdate(screen);
        }

        // Show death and restart screen
        while (!tigrClosed(screen) && !alive) {
            tigrClear(backdrop, tigrRGB(200, 200, 200));

            tigrPrint(screen, bigFont, 30, HEIGHT/2-50, tigrRGB(0, 0, 0), "YOU DIED.");
            tigrPrint(screen, smallFont, 30, HEIGHT/2+15, tigrRGB(0, 0, 0), "Your score: %lld", score);
            tigrPrint(screen, smallFont, 30, HEIGHT/2+50, tigrRGB(0, 0, 0), "Restart? [Y/N]");

            if (tigrKeyDown(screen, 'Y')) {
                // Reset initial values
                alive = true;
                restart = true;
                score = 0;
                playerx = WIDTH/2;
                playery = HEIGHT-200.0f;
                blocky = HEIGHT-60;

                // Quick-reset matrix and block array
                memset(screen_matrix, 0, HEIGHT*sizeof(screen_matrix[0]));
                memset(bloques, 0, BLOCK_MAXN*sizeof(bloques[0]));
            } else if (tigrKeyDown(screen, 'N')) {
                // Set 'alive' to true to exit loop and quit the program.
                alive = true;
                restart = false;
            }

            tigrUpdate(screen);
        }
    } while (restart);

    tigrFree(screen);
    tigrFree(player);
    tigrFree(smallFontImage);
    tigrFree(regularFontImage);
    tigrFree(bigFontImage);
    return 0;
}

/*
    Function: update
    -----------------
    Updates the game state. It is called every tick.

    screen: the Tigr* variable representing the screen.
    player: the Tigr* variable representing the moveable sprite.
    matrix: a matrix with HEIGHT rows and WIDTH cols representing all the pixels in the screen.
    t0: timestamp of the last time the sprite was in contact with a solid block, in milliseconds.
    dt: the current timestamp, in milliseconds.
    blocky: the Y coordinate of the block on which the sprite is jumping.
    playerx: pointer to the X position (left side) of the sprite.
    playery: pointer to the Y position (upper side) of the sprite.
    bloques: an array with descriptions of all of the blocks' position and type.

    returns: the variation in the game score.
*/
int update(Tigr* screen, Tigr* player, int matrix[HEIGHT][WIDTH], int t0, int dt, int *blocky, float* playerx, float* playery, Bloque* bloques) {
    bool on_platform;
    float yvariation = 0.0;

    // X-axis movement (check if key is pressed and move accordingly)
    if (tigrKeyHeld(screen, TK_LEFT) || tigrKeyHeld(screen, 'A'))
        *playerx -= 5; // move 5 pixels to the left
    if (tigrKeyHeld(screen, TK_RIGHT) || tigrKeyHeld(screen, 'D'))
        *playerx += 5; // move 5 pixels to the right

    // Correct collisions with side walls
    if (*playerx < 0) *playerx = 0;
    else if (*playerx > WIDTH-(width(player))) *playerx = WIDTH-(width(player));

    // Y-axis movement (simple harmonic motion)
    // y(t) = JUMP_HEIGHT * sin(2*M_PI*0.5(t-t0))

    on_platform = isOnPlatform(player, matrix, *playerx, *playery);

    if (*playery >= *blocky && !on_platform) {
        *playery += 5;
    } else {
        if (!on_platform && *playery < HEIGHT-height(player) && y(dt, t0) < 0 && dy(dt, t0) < 0) *playery += 5;
        else *playery = *blocky-fabsf(y(dt, t0));

        if (on_platform) (*playery)--;

        if(*playery<HEIGHT/2 && isGoingUp(player, dt, t0)){
            yvariation = SCROLL_SPEED*dy(dt, t0);

            bajarElementosPantalla(yvariation, bloques);
            *playery += yvariation;
            *blocky += yvariation;
        }
    }

    // Correct collisions with top and bottom ends of the screen
    if (*playery < 0) *playery = 0;
    else if (*playery > HEIGHT-(height(player))-bloques[0].h && bloques[0].w == WIDTH) {
        *playery = HEIGHT-(height(player))-bloques[0].h;
    }

    return yvariation;
}

/*
    Procedure: drawScreen
    ---------------------
    Fills the background of the screen with a solid color and draws
    rectangles of different colors in the position of the generated blocks.

    backdrop: a Tigr* bitmap covering the size of the screen.
    bloques: an array with descriptions of all of the blocks' position and type.
*/
void drawScreen(Tigr* backdrop, Bloque* bloques) {
    tigrClear(backdrop, tigrRGB(255, 255, 255));

    for (int i = 0; i < BLOCK_MAXN; i++) {
        if (bloques[i].tipo == 1) {
            tigrFill(backdrop, bloques[i].x, bloques[i].y, bloques[i].w, bloques[i].h, tigrRGB(0, 255, 0));
        }
    }
}

/*
    Procedure: generatePlatforms
    ----------------------------
    Fills the block array with newly-generated platforms. Platforms have a 30%
    chance of appearing 60 pixels above the previously generated platform, and
    a 70% chance of appearing 120 pixels above the last platform.

    bloques: an array with descriptions of all of the blocks' position and type.
    matrix: a matrix with HEIGHT rows and WIDTH cols representing all the pixels in the screen.
*/
void generatePlatforms(Bloque* bloques, int matrix[HEIGHT][WIDTH]) {
    Bloque generated_block;
    int overflow = 0;

    // Get the number of platforms that overflow the height of the screen.
    while (overflow < BLOCK_MAXN && bloques[overflow].y > HEIGHT) {
        overflow++;
    }

    // Shift the block array to the left by that number of elements.
    shiftBlockArray(bloques, overflow);

    // Quick-reset matrix to 0.
    memset(matrix, 0, HEIGHT*sizeof(matrix[0]));

    // Generate new blocks on the right.
    generated_block.w = PLATFORM_WIDTH;
    generated_block.h = PLATFORM_HEIGHT;
    for (int idx = 0; idx < BLOCK_MAXN; idx++) {
        if (bloques[idx].tipo == 0) {
            if (idx == 0) {
                // Bottom solid line
                for (int i = HEIGHT-50; i < HEIGHT; i++) {
                    for (int j = 0; j < WIDTH; j++) {
                        matrix[i][j] = 1;
                    }
                }

                bloques[idx].tipo = 1;
                bloques[idx].x = 0;
                bloques[idx].y = HEIGHT-50;
                bloques[idx].h = 50;
                bloques[idx].w = WIDTH;
            } else {
                // Generate a block 60 pixels above the previous one with 30% chance.
                if (randrange(0, 10) < 3) {
                    generated_block.tipo = 1;
                    generated_block.y = bloques[idx-1].y - 60;
                    generated_block.x = randrange(0, WIDTH-PLATFORM_WIDTH);
                } else {
                    // Otherwise generate a block 120 pixels above the previous one with 100% chance.
                    generated_block.tipo = 1;
                    generated_block.y = bloques[idx-1].y - 120;
                    generated_block.x = randrange(0, WIDTH-PLATFORM_WIDTH);
                }

                bloques[idx] = generated_block;
            }
        }

        for (int i = max(bloques[idx].y, 0); i < min(bloques[idx].y + bloques[idx].h, HEIGHT); i++) {
            for (int j = 0; j < bloques[idx].w; j++) {
                matrix[i][bloques[idx].x + j] = bloques[idx].tipo;
            }
        }
    }
}

/*
    Procedure: bajarElementosPantalla
    ---------------------------------
    Moves all of the defined platforms down by a specified number of pixels.

    n: the number of pixels to lower the platforms.
    bloques: an array with descriptions of all of the blocks' position and type.
*/
void bajarElementosPantalla(int n, Bloque* bloques) {
    for (int i = 0; i < BLOCK_MAXN; i++) {
        // Check for potential overflow or underflow before updating y
        if ((n > 0 && INT_MAX - bloques[i].y < n) || (n < 0 && INT_MIN - bloques[i].y > n)) {
            // Handle overflow or underflow case appropriately
            if (n > 0) {
                bloques[i].y = INT_MAX;
            } else {
                bloques[i].y = INT_MIN;
            }
        } else {
            bloques[i].y += n;
        }
    }
}

// ----------------------------------- //
//         MOVEMENT FUNCTIONS          //
// ----------------------------------- //

/*
    Function: y
    -----------
    Returns the y-coordinate variation of the sprite given a time differential.
    This value is calculated as the following simple harmonic motion function:
        y(t) = JUMP_HEIGHT * sin(2 * pi * 0.5 * (t-t0))
    
    The movement is constant and has an angular velocity of PI rad/s (f = 0.5 Hz).

    t: the current timestamp, in milliseconds.
    t0: the timestamp of the beginning of the oscillation, in milliseconds.

    returns: the y-coordinate variation in the (t-t0) time variation.
*/
float y(int t, int t0) {
    return JUMP_HEIGHT*sin(M_PI*0.001*(t-t0));
}

/*
    Function: dy
    ------------
    Returns the derivative of y(t), i.e. the vertical speed in pixels/s.

    t: the current timestamp, in milliseconds.
    t0: the timestamp of the beginning of the oscillation, in milliseconds.

    returns: the current vertical speed of the sprite, in pixels/s.
*/
float dy(int t, int t0) {
    return JUMP_HEIGHT*M_PI*0.001*cos(M_PI*0.001*(t-t0));
}

// ----------------------------------- //
//          STATUS FUNCTIONS           //
// ----------------------------------- //

/*
    Function: isOnPlatform
    ----------------------
    Returns true if the sprite is standing on top of a solid platform.

    player: the Tigr* variable representing the moveable sprite.
    matrix: a matrix with HEIGHT rows and WIDTH cols representing all the pixels in the screen.
    playerx: value of the X coordinate (left side) of the sprite.
    playery: value of the Y coordinate (upper side) of the sprite.

    returns: true if the sprite is on a platform, false otherwise.
*/
bool isOnPlatform(Tigr* player, int matrix[HEIGHT][WIDTH], float playerx, float playery) {
    int px = (int)playerx, py = (int)min(playery, HEIGHT-height(player)-1);
    if (matrix[py+(height(player))][px+((width(player))/2)] != 0
        && matrix[py+(height(player))-10][px+((width(player))/2)] == 0)
        return true;
    return false;
}

/*
    Function: isGoingUp
    -------------------
    Returns true if the sprite is moving upward, i.e. the velocity is positive.

    player: the Tigr* variable representing the moveable sprite.
    t: the current timestamp, in milliseconds.
    t0: the timestamp of the beginning of the oscillation, in milliseconds.

    returns: true if y(t) > 0 and d(y)/dt > 0, false otherwise.
*/
bool isGoingUp(Tigr* player, int t, int t0) {
    return dy(t, t0) > 0;
    // return (y(t, t0) > 0 && dy(t, t0) > 0);
}

// ---------------------------------- //
//         UTILITY FUNCTIONS          //
// ---------------------------------- //

/*
    Function: min
    -------------
    Returns the minimum of two floating-point numbers.

    a: the first number to be evaluated.
    b: the second number to be evaluated.

    returns: the minimum value between a and b.
*/
float min(float a, float b) {
    if (a < b) return a;
    return b;
}

/*
    Function: max
    -------------
    Returns the maximum of two floating-point numbers.

    a: the first number to be evaluated.
    b: the second number to be evaluated.

    returns: the maximum value between a and b.
*/
float max(float a, float b) {
    if (a > b) return a;
    return b;
}

/*
    Function: randrange
    -------------------
    Generates a random integer number in the range [lower, upper) (non-inclusive).

    lower: the minimum value of the interval.
    upper: the first value after the maximum value of the interval.

    returns: the generated random number.
*/
int randrange(int lower, int upper) {
    return (rand() % (upper - lower)) + lower;
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

/*
    Procedure: shiftBlockArray
    --------------------------
    Shifts the elements of an array by n positions to the left. The first n
    elements are discarded.

    bloques: an array with descriptions of all of the blocks' position and type.
    n: the number of positions to shift the array.
*/
void shiftBlockArray(Bloque* bloques, int n) {
    if (n > 0) {
        for (int i = 0; i < BLOCK_MAXN-n; i++) {
            bloques[i] = bloques[i+n];
            bloques[i+n].tipo = 0;
        }
    }
}