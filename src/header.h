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
    bool is_on;

} Keypad;

extern int chip_8_arr[16];
extern char keyboard_arr[16];

void draw_byte(int x, int y, int scale, SDL_Renderer* renderer);
bool is_empty(Stack *stack);
void initialise(Stack *stack);
void push(Stack *stack, uint16_t new_value);
int pop(Stack *stack);
uint16_t peek(uint16_t *ptr);
void printNum(u_int16_t x);
void clear_pixels(uint8_t pixels[64][32], int w, int h);
int chip_8_to_keyboard(int chip_8);
int keyboard_to_chip_8(int keyboard);
SDL_FRect find_rect(int i, int j, int w, int h);
