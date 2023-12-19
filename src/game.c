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
#define PLATFORM_MOV_TICKS 2

#define height(x) x->h
#define width(x) x->w

/*
    Block types:
        1. Regular solid block
        2. Single-use platform
        3. Mobile platform
    Block direction (only applicable to type 3 blocks):
        0. Left
        1. Right
*/
typedef struct {
    int tipo;
    int x, y;
    int w, h;
    int direction;
} Bloque;

// Periodical functions
int update(Tigr*, Tigr*, int[HEIGHT][WIDTH], int, int, int*, float*, float*, Bloque*);
void drawScreen(Tigr*, Bloque*, long long);
void generatePlatforms(Bloque*, int[HEIGHT][WIDTH], long long);
void bajarElementosPantalla(int, Bloque*);
void updatePlatforms(Bloque*, float, bool, bool, bool);

// Movement functions
float y(int, int);
float dy(int, int);

// State checkers
bool isOnPlatform(Tigr*, int[HEIGHT][WIDTH], float, float);
bool isGoingUp(int, int);

// Utilities
float min(float, float);
float max(float, float);
int randrange(int, int);
int timeInMilliseconds();
void shiftBlockArray(Bloque*, int);
int randomPlatformType(long long);
Bloque randomPlatform(int, int);
TPixel getBackgroundColor(long long);
TPixel getTextColor(long long);

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
    int elapsedTicks = 0;
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
            generatePlatforms(bloques, screen_matrix, score);
            drawScreen(backdrop, bloques, score);
            t = timeInMilliseconds();

            updatePlatforms(bloques, playery+height(player), ++elapsedTicks >= PLATFORM_MOV_TICKS,
                isOnPlatform(player, screen_matrix, playerx, playery), !isGoingUp(t, t0));

            // Reset elapsedTicks to 0 after every PLATFORM_MOV_TICKS.
            if (elapsedTicks >= PLATFORM_MOV_TICKS) elapsedTicks = 0;

            if (isOnPlatform(player, screen_matrix, playerx, playery) && !isGoingUp(t, t0)) {
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
            
            tigrPrint(screen, smallFont, 30, 30, getTextColor(score), "%lld", score);

            tigrUpdate(screen);
        }

        // Show death and restart screen
        while (!tigrKeyDown(screen, TK_ESCAPE) && !tigrClosed(screen) && !alive) {
            tigrClear(backdrop, tigrRGB(200, 200, 200));

            tigrPrint(screen, bigFont, 30, HEIGHT/2-50, getTextColor(score), "YOU DIED.");
            tigrPrint(screen, smallFont, 30, HEIGHT/2+15, getTextColor(score), "Your score: %lld", score);
            tigrPrint(screen, smallFont, 30, HEIGHT/2+50, getTextColor(score), "Restart? [Y/N]");

            if (tigrKeyDown(screen, 'Y')) {
                // Reset initial values
                alive = true;
                restart = true;
                score = 0;
                playerx = WIDTH/2;
                playery = HEIGHT-200.0f;
                blocky = HEIGHT-60;
                elapsedTicks = 0;
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

        if(*playery<HEIGHT/2 && isGoingUp(dt, t0)){
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
    score: the current score.
*/
void drawScreen(Tigr* backdrop, Bloque* bloques, long long score) {
    tigrClear(backdrop, getBackgroundColor(score));
    TPixel platformColor;

    for (int i = 0; i < BLOCK_MAXN; i++) {
        if (bloques[i].tipo > 0) {
            switch (bloques[i].tipo) {
                case 1: // Fixed platform (#9BDA84: Pistachio)
                    platformColor = tigrRGB(155, 218, 69);
                    break;
                case 2: // Single-use platform (#D5765A: Burnt Sienna)
                    platformColor = tigrRGB(213, 118, 90);
                    break;
                case 3: // Mobile platform (#E9D758: Arylide Yellow)
                    platformColor = tigrRGB(233, 215, 88);
                    break;
                default: // Other (#ffffff: White) - for debugging purposes
                    platformColor = tigrRGB(255, 255, 255);
            }

            tigrFill(backdrop, bloques[i].x, bloques[i].y, bloques[i].w, bloques[i].h, platformColor);
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
    score: the current score.
*/
void generatePlatforms(Bloque* bloques, int matrix[HEIGHT][WIDTH], long long score) {
    int overflow = 0;

    // Get the number of platforms that overflow the height of the screen.
    while (overflow < BLOCK_MAXN && bloques[overflow].y > HEIGHT) {
        overflow++;
    }

    // Shift the block array to the left by that number of elements.
    shiftBlockArray(bloques, overflow);

    // Quick-reset matrix to 0.
    memset(matrix, 0, HEIGHT*sizeof(matrix[0]));

    // Generate new blocks
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
                    bloques[idx] = randomPlatform(randomPlatformType(score), bloques[idx-1].y-60);
                } else {
                    // Otherwise generate a block 120 pixels above the previous one with 100% chance.
                    bloques[idx] = randomPlatform(randomPlatformType(score), bloques[idx-1].y-120);
                }
            }
        }

        for (int i = max(bloques[idx].y, 0); i < min(bloques[idx].y + bloques[idx].h, HEIGHT); i++) {
            for (int j = 0; j < bloques[idx].w; j++) {
                if (bloques[idx].tipo > 0) matrix[i][bloques[idx].x + j] = bloques[idx].tipo;
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

/*
    Procedure: updatePlatforms
    --------------------------
    This function updates the state of the platforms in the current tick. It
    essentially does two different things:
        1. If movePlatforms is true, the x position of all mobile platforms in
           the block array will be shifted 1 pixel to the right in the
           specified direction (unless colliding with a wall).
        2. If the sprite is standing on a single-use platform, delete it so
           that it cannot be used again.
    
    bloques: an array with descriptions of all of the blocks' position and type.
    y: the y position of the sprite's lower side, i.e. (player->y + player->h).
    movePlatforms: if true, all mobile platforms will move 1 pixel towards their
        specified direction. Only applied after the number of ticks in PLATFORM_MOV_TICKS.
    onPlatform: boolean indicating if the sprite is in contact with a solid block.
    goingDown: boolean indicating if the sprite is moving downwards.
*/
void updatePlatforms(Bloque* bloques, float y, bool movePlatforms, bool onPlatform, bool goingDown) {
    for (int i = 0; i < BLOCK_MAXN; i++) {
        if (bloques[i].tipo == 2 && onPlatform && goingDown) {
            // Check if the sprite is standing on top of a single-use platform.
            if (y >= bloques[i].y && y <= bloques[i].y + bloques[i].h) {
                // If so, then delete it. We use an arbitrary value of -1
                // to avoid generating new platforms in this position.
                bloques[i].tipo = -1;
            }
        } else if (bloques[i].tipo == 3 && movePlatforms) {
            if (bloques[i].direction == 0) { // Move to the left
                bloques[i].x -= 1;
                
                if (bloques[i].x <= 0) { // Reached wall, change direction
                    bloques[i].x = 0;
                    bloques[i].direction = 1;
                }
            } else { // Move to the right
                bloques[i].x += 1;

                if (bloques[i].x >= WIDTH-PLATFORM_WIDTH) {
                    bloques[i].x = WIDTH-PLATFORM_WIDTH;
                    bloques[i].direction = 0;
                }
            }
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

    t: the current timestamp, in milliseconds.
    t0: the timestamp of the beginning of the oscillation, in milliseconds.

    returns: true if y(t) > 0 and d(y)/dt > 0, false otherwise.
*/
bool isGoingUp(int t, int t0) {
    return dy(t, t0) > 0;
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

/*
    Function: randomPlatformType
    ----------------------------
    Returns a randomized platform type for platform generation, based on the
    current score.
        - Single-use platforms (type 2) start appearing at score 1500 and have
          a 25% chance of appearing.
        - Mobile platforms (type 3) start appearing at score 3000 and have a
          20% chance of appearing.
    If none of these conditions are met, the return value defaults to type 1
    (regular solid block).

    score: the current score.

    returns: a platform type (value between 1 and 3).
*/
int randomPlatformType(long long score) {
    int random = randrange(0, 100);
    int type = 1;
   
    if (score >= 1500 && random >= 0 && random < 25) type = 2;
    else if (score >= 3000 && random >= 25 && random < 45) type = 3;
    
    return type;
}

/*
    Function: randomPlatform
    ------------------------
    Generates a platform of a given type at a random x-axis position given
    its y-axis location.

    type: the type of the platform (value between 1 and 3).
    y: the vertical position of the platform.

    returns: the generated block.
*/
Bloque randomPlatform(int type, int y) {
    Bloque generatedBlock;

    // Set platform type, width and height
    generatedBlock.tipo = type;
    generatedBlock.w = PLATFORM_WIDTH;
    generatedBlock.h = PLATFORM_HEIGHT;

    // Set desired y and random x
    generatedBlock.y = y;
    generatedBlock.x = randrange(0, WIDTH-PLATFORM_WIDTH);

    // Set random direction (only if type = 2)
    if (type == 2 && randrange(0, 10) < 5) generatedBlock.direction = 0;
    else generatedBlock.direction = 1;

    return generatedBlock;
}

/*
    Function: getBackgroundColor
    ----------------------------
    Returns the background color of the window given the current score. This
    color begins at a light cyan, and after 6000 points, it shifts to a dark
    gray by means of a 500-point-long color gradient. Then, it remains dark
    for 6000 more points, and finally it returns to the light setting with a
    new gradient.

    Color palette: https://coolors.co/caf0f8-3d3d49-d5765a-e9d758-9bda84

    score: the current score.

    returns: a TPixel object with the RGB values of the background color.
*/
TPixel getBackgroundColor(long long score) {
    long long mod = score % 13000LL;
    unsigned char red = 0, green = 0, blue = 0;

    if (mod < 6000) {
        // Solid #CAF0F8 (Light Cyan)

        red = 202;
        green = 240;
        blue = 248;
    } else if (mod <= 6500) {
        // Get clear-to-dark color gradient
        // Starting color: #CAF0F8 (Light Cyan) -- RGB(202, 240, 248)
        // Ending color: #3D3D49 (Onyx) -- RGB(61, 61, 73)

        red = 202 + ((float)(mod - 6000) / 500.0f) * (61 - 202);
        green = 240 + ((float)(mod - 6000) / 500.0f) * (61 - 240);
        blue = 248 + ((float)(mod - 6000) / 500.0f) * (73 - 248);
    } else if (mod < 12500) {
        // Solid #3D3D49 (Onyx)

        red = green = 61;
        blue = 73;
    } else {
        // Get dark-to-clear color gradient
        // Starting color: #3D3D49 (Onyx) -- RGB(61, 61, 73)
        // Ending color: #CAF0F8 (Light Cyan) -- RGB(202, 240, 248)

        red = 61 + ((float)(mod - 12500) / 500.0f) * (202 - 61);
        green = 61 + ((float)(mod - 12500) / 500.0f) * (240 - 61);
        blue = 73 + ((float)(mod - 12500) / 500.0f) * (248 - 73);
    }

    return tigrRGB(red, green, blue);
}

/*
    Function: getTextColor
    ----------------------
    Returns the RGB color for optimal contrast between the text and the
    background. It applies the concept of relative luminance to determine
    the brightness of the background and thus choose a lighter or darker
    color accordingly.

    Adapted from: https://stackoverflow.com/a/1855903 (CC BY-SA 4.0)
    See also: https://www.w3.org/TR/AERT/#color-contrast

    score: the current score.

    returns: a TPixel object with the RGB values of the text color.
*/
TPixel getTextColor(long long score) {
    TPixel bgColor = getBackgroundColor(score);
    unsigned char color;

    double luminance = (0.299 * bgColor.r + 0.587 * bgColor.g + 0.114 * bgColor.b) / 255;

    if (luminance > 0.5) color = 0; // Black
    else color = 255;               // White

    return tigrRGB(color, color, color);
}