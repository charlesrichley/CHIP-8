# CHIP-8 emulator with quirk customizability and fast game launching 

I developed a CHIP-8 emulator using C (and the SDL3 library), with extensive support across the range of quirks that CHIP-8 has. A launch screen allows users to select a ROM from a default menu, or upload their own ROM. There are 8 quirks that can be customised to ensure compatability.

## Quirks
| Name | Functionality | Default |
|---|---|---|
| RESET VF | 8XY1, 8XY2, 8XY3 reset VF to 0 | OFF |
| MEMORY | FX55 and FX65 increment index register | OFF |
| DRAW WAIT | DXYN only called once per frame | OFF |
| CLIPPING | Sprites get clipped instead of wrapping in DXYN | ON |
| SHIFTING | 8XY6 and 8XYE only operate on VX instead of storing shifted VY in VX | ON |
| JUMPIUNG | BNNN doesn't use V0, but VX instead (X is first nibble in NNN) | OFF |
| FX1E OVERFLOW | FX1E sets VF to 0 if the index registers overflow | OFF |
| FX0A PRESSED AND RELEASED | FX0A resumes execution if the key is both pressed and released or simply pressed | ON |

## Compiler message 
"clang main.c helpers.c welcome.c -o main $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf) && ./main \"/Users/charlesrichley/chip_8/welcome.c\""

## ROM sources
[CHIP-8 Archive](https://johnearnest.github.io/chip8Archive/?sort=platform) —  `roms/`
[Zophar PD ROM Pack](https://www.zophar.net/pdroms/chip8/chip-8-games-pack.html) — `tests/test_roms/`
