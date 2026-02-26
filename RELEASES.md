# Version ...

* 20260220 Host: Save screenshot in user homedir with <Alt+P>

# Version 0.12.20260131-alpha

* vdc: proper scaling for layers and sprites (1x, 2x, 4x, 8x) in both directions
* vdc: transparency added for sprites in flags0, bits 6 and 7
* vdc: 320 x 176 resolution (in stead of 180), 22 x 8 vertical
* mc68000 rom: putchar, putstring and puthexbyte added
* mc68000 rom: putchar and putstring have trap #15 calling as well (document this!)
* mc68000 rom: basic binary loading of adapted decb format enabled
* color per tile mode added for 0b11 bit patterns
* addition of bitmapped mode for layers
* tileset address configurable for layers and sprites
* mc6809 upgraded from v0.17 to v0.18
* Host: Scanline effect on/off with <Alt+S>
* Host: Scanline intensity with <Alt+D>
* Host: osd notifications
* Host: Contrast adjustable with <Alt+A>
* Examples (decb format): Added

# Version 0.11.20251125-alpha

* Move from SDL2 to SDL3
* Using SDL_SCALEMODE_PIXELART for texture rendering, new in SDL 3.4, for better retro pixel look when textures are not magnified with an integer scale
* CMakeLists.txt supports both SDL vendored and system supplied using a switch
