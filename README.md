# CHIP-8 emulator with quirk customizability and fast game launching 

I developed a CHIP-8 emulator using C (and the SDL3 library), with extensive support across the range of quirks that CHIP-8 has. A launch screen allows users to select a ROM from a default menu, or upload their own ROM. There are 8 quirks that can be customised to ensure compatability.

## Quirks
| Name | Functionality | Default |
|---|---|---|
| RESET VF | ___ | OFF |
| MEMORY | ___ | OFF |
| DRAW WAIT | ___ | OFF |
| CLIPPING | ___ | ON |
| SHIFTING | ___ | ON |
| JUMPIUNG | ___ | OFF |
| FX1E OVERFLOW | ___ | OFF |
| FX0A PRESSED AND RELEASED | ____ | ON |

## Compiler message 
"clang main.c helpers.c -o main $(pkg-config --cflags --libs sdl3 sdl3-image) && ./main"

## Games sources (open source)
https://johnearnest.github.io/chip8Archive/?sort=platform
https://www.zophar.net/pdroms/chip8/chip-8-games-pack.html
