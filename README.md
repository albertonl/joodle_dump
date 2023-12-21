# Joodle Dump

![Joodle Dump Logo](img/logo/white_text_bgblack.png)

Joodle Dump is a simple platform game, where the user has to climb as high as possible through a series of randomly generated platforms until the player falls and dies. This game has a similar functionality to the one in the classic "Doodle Jump" game.

## Compilation and execution

You can easily compile this project using the provided Makefile. In the main directory of the project, run:

```
make
./joodle
```

### Build requirements

- GCC compiler
- [TIGR](https://github.com/erkkah/tigr/tree/master) C graphical library source files (included in the source of this project)
- GNU Make
- X11 library (install via `sudo apt install libx11-dev`)
- OpenGL library (install via `sudo apt install freeglut3-dev`)

A `Makefile` has been provided for convenience, and to facilitate portability between platforms. Although compilation flags have been written for all Windows, MacOS and Linux systems, the project has only been tested on Ubuntu 22.04 and MacOS Sonoma 14.1.2.

## Controls

During the game:

- `A`/`Left arrow` (tap/hold): move the character to the left.
- `D`/`Right arrow` (tap/hold): move the character to the right.
- `P`: pause the game.
- `ESC` / Close window: quit the program.
- `[1-3]`: switch character.

In the game over screen:

- `Y`: restart the game.
- `N` / `ESC` / Close window: quit the game.
- `Ctrl+R`: reset high score.

## Game mechanics

The only action that the user can take is to move along the X-axis using the arrow keys or the A-D keys. The character will jump automatically at every defined time interval, as long as it is standing on top of a platform.

When the character jumps at a height greater than or equal to half of the screen height, the existing platforms will move down by a number of pixels $n$, which is determined by the following formula:

$$n = \lambda\cdot\frac{dy}{dt}$$

Where $\lambda$ represents a constant value defined in the macro `SCROLL_SPEED` (20 by default). For that purpose, the bottom $h$ pixels of the screen will first be cleared, and all of the existing platforms above that height will move down by $h$ pixels. After that, the top $h$ pixels of the screen will be filled with newly generated platforms.

If the user fails to jump above the middle of the screen, the platform structure remains the same.

Lastly, if the character touches the bottom of the screen at any point, it dies and the game is over.
