# Joodle Dump

Joodle Dump is a simple platform game, where the user has to climb as high as possible through a series of randomly generated platforms until the player falls and dies. This game has a similar functionality to the one in the classic "Doodle Jump" game.

## Compilation and execution

You can easily compile this project using the provided Makefile.

```
make
./joodle
```

### Build requirements

- GCC compiler
- [TIGR](https://github.com/erkkah/tigr/tree/master) C graphical library source files (included in the source of this project)
- `make`

## Controls

- `A`/`Left arrow` (tap/hold): move the character to the left.
- `D`/`Right arrow` (tap/hold): move the character to the right.
- `N`: start a new game.
- `ESC`: quit the program.

## Game mechanics

The only action that the user can take is to move along the X-axis using the arrow keys or the A-D keys. The character will jump automatically at every defined time interval, as long as it is standing on top of a platform.

When the character jumps at a height greater than or equal to half of the screen height, the existing platforms will move down by the number of pixels determined by the absolute difference of these two heights, which we will call $h$.

$$h = \left|y_{\mathrm{character}}-\frac{\text{screen height}}{2}\right|$$

For that purpose, the bottom $h$ pixels of the screen will first be cleared, and all of the existing platforms above that height will move down by $h$ pixels. After that, the top $h$ pixels of the screen will be filled with newly generated platforms.

If the user fails to jump above the middle of the screen, the platform structure remains the same.

Lastly, if the character touches the bottom of the screen at any point, it dies and the game is over.
