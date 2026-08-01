#include "header.h"

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

int chip_8_arr[16] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
char keyboard_arr[16] = {'1', '2', '3', '4', 'Q', 'W', 'E', 'R', 'A', 'S', 'D','F', 'Z', 'X', 'C', 'V'};

// Must read font data
void draw_byte(int x, int y, int scale, SDL_Renderer* renderer)
{
    for (int x_add = 0; x_add < 4; x_add++)
    {
        for (int y_add = 0; y_add < 5; y_add++)
        {
            SDL_FRect rect = {(x+x_add) * scale, (y+y_add) * scale, 1, 1};
            SDL_RenderRect(renderer, &rect);
        }
    }
}

// Stack operations: push, pop, peek - all O(1)
bool is_empty(Stack *stack)
{
    return (stack->top == -1);
}

void initialise(Stack *stack)
{
    stack->top = -1;
}

// Inserts element to top of stack
void push(Stack *stack, uint16_t new_value)
{
    stack->arr[stack->top+1] = new_value;
    stack->top += 1;
}

// Removes top element of stack (and returns the popped integer)
int pop(Stack *stack)
{
    int popped_number = stack->arr[stack->top];
    stack->top -= 1;
    return popped_number;
}

// Returns the top element of stack
uint16_t peek(uint16_t *ptr)
{
    return 0;
}

void printNum(u_int16_t x)
{
    printf("%" PRIu16 "\n", x);
}

void clear_pixels(uint8_t pixels[][32], int i, int j)
{
    for (int a = 0; a < i; a++)
    {
        for (int b = 0; b < j; b++)
        {
            pixels[a][b] = 0;
        }
    }
}

int chip_8_to_keyboard(int chip_8)
{
    for (int i = 0; i < 16; i++)
    {
        if (chip_8 == chip_8_arr[i])
        {
            return keyboard_arr[i];
        }
    }
    return -1;
}

int keyboard_to_chip_8(int keyboard)
{
    for (int i = 0; i < 16; i++)
    {
        if (keyboard == keyboard_arr[i])
        {
            return chip_8_arr[i];
        }
    }
    return -1;
}
