# Version ...

* Scanline effect on/off with <Alt + S>

# Version 0.11.20251124

* Move from SDL2 to SDL3
* Using SDL_SCALEMODE_PIXELART for texture rendering, new in SDL 3.4, better for retro look where textures are not magnified with an integer scale
* CMakeLists.txt supports both SDL vendored and system supplied using a switch
