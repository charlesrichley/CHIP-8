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

extern int chip_8_arr[16];
extern char keyboard_arr[16];
extern SDL_Scancode scancode_arr[16];
extern uint8_t sprite_arr[80];

bool is_empty(Stack *stack);
void initialise(Stack *stack);
void push(Stack *stack, uint16_t new_value);
int pop(Stack *stack);

SDL_FRect get_frect_TL(int i, int j, int w, int h);
SDL_FRect get_frect_centered(int i, int j, int w, int h);
void pixels_on(uint8_t pixels[WIDTH][HEIGHT]);

int keyboard_to_index(char c);
int scancode_to_index(Keypad keypad[16], SDL_Scancode scancode);
int chip_8_to_keyboard(int chip_8);
int keyboard_to_chip_8(int keyboard);

void print_memory(uint8_t memory[4096]);
void print_keypad(Keypad keypad[16]);
void print_chip_8_arr(int chip_8_arr[16]);
void printNum(u_int16_t x);
void print_pixels(uint8_t pixels[WIDTH][HEIGHT]);
