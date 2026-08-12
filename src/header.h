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
#include <inttypes.h>

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

} Keypad;

#define WIDTH 64
#define HEIGHT 32
#define SCALE 16
#define INSRUCTIONS_PER_SECOND 10

extern int chip_8_arr[16];
extern char keyboard_arr[16];
extern uint8_t sprite_arr[80];

void draw_byte(int x, int y, int scale, SDL_Renderer* renderer);
bool is_empty(Stack *stack);
void initialise(Stack *stack);
void push(Stack *stack, uint16_t new_value);
int pop(Stack *stack);
void clear_pixels(uint8_t pixels[WIDTH][HEIGHT]);
int chip_8_to_keyboard(int chip_8);
int keyboard_to_chip_8(int keyboard);
SDL_FRect get_frect(int i, int j, int w, int h);
SDL_Rect get_rect(int i, int j, int w, int h);
void printNum(u_int16_t x);
void print_pixels(uint8_t pixels[WIDTH][HEIGHT]);
void pixels_on(uint8_t pixels[WIDTH][HEIGHT]);
void print_memory(uint8_t memory[4096]);
int get_scancode_index(Keypad keypad[16], SDL_Scancode scancode);
void print_keypad(Keypad keypad[16]);
void print_chip_8_arr(int chip_8_arr[16]);
