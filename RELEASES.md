# Version ...

* mc68000 rom: putchar, putstring and puthexbyte added
* color per tile mode added for 0b11 bit patterns
* addition of bitmapped mode for layers
* tileset address configurable for layers and sprites
* mc6809 upgraded from v0.17 to v0.18
* Host: Scanline effect on/off with <Alt+S>
* Host: Scanline intensity with <Alt+D>
* Host: osd notifications
* Host: Contrast adjustable with <Alt+A>

# Version 0.11.20251125-alpha

* Move from SDL2 to SDL3
* Using SDL_SCALEMODE_PIXELART for texture rendering, new in SDL 3.4, for better retro pixel look when textures are not magnified with an integer scale
* CMakeLists.txt supports both SDL vendored and system supplied using a switch
