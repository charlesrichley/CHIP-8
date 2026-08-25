#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <inttypes.h>

#define WIDTH 64
#define HEIGHT 32
#define SCALE 16
#define INSRUCTIONS_PER_FRAME 12

#define MAX_FILE_NAME_SIZE 2000
#define QUIRK
#define NUMBER_OF_QUIRKS 8

typedef struct 
{
    uint16_t arr[16];
    int top;
} Stack;

typedef struct 
{
    int chip_8;
    char keyboard;
    SDL_Scancode scancode;
    bool is_down;
} Keypad;

typedef struct
{
    char input[MAX_FILE_NAME_SIZE];
    int length; 
} Input_String;

typedef struct
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    bool is_on;
    char *string;
    int x;
    int y;
} Button;

enum {
    RESET_VF,
    MEMORY,
    DISPLAY_WAIT,
    CLIPPING,
    SHIFTING,
    JUMPING,
    FX1E_OVERFLOW,
    FX0A_PRESSED_AND_RELEASED
};

extern int chip_8_arr[16];
extern char keyboard_arr[16];
extern SDL_Scancode scancode_arr[16];
extern uint8_t sprite_arr[80];

void initialise(Stack *stack);
void push(Stack *stack, uint16_t new_value);
int pop(Stack *stack);

SDL_FRect get_frect(int i, int j, int w, int h, bool is_centered);

int keyboard_to_index(char c);
int scancode_to_index(Keypad keypad[16], SDL_Scancode scancode);
int chip_8_to_keyboard(int chip_8);
int keyboard_to_chip_8(int keyboard);

Button get_button(int x, int y, char *string, bool is_on, TTF_Font *font, SDL_Color font_color, SDL_Renderer *renderer);
