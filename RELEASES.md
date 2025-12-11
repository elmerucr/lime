# Version ...

* Scanline effect on/off with <Alt+S>
* Scanline intensity with <Alt+D>
* osd notifications
* Contrast adjustable with <Alt+A>
* color per tile mode added for 0b11 bit patterns

# Version 0.11.20251125-alpha

* Move from SDL2 to SDL3
* Using SDL_SCALEMODE_PIXELART for texture rendering, new in SDL 3.4, for better retro pixel look when textures are not magnified with an integer scale
* CMakeLists.txt supports both SDL vendored and system supplied using a switch
