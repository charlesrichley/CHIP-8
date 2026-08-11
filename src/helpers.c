#include "header.h"

int chip_8_arr[16] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
char keyboard_arr[16] = {'1', '2', '3', '4', 'Q', 'W', 'E', 'R', 'A', 'S', 'D','F', 'Z', 'X', 'C', 'V'};

// Font characters are 4 pixels wide by 5 pixels tall
uint8_t sprite_arr[80] = {0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
0x20, 0x60, 0x20, 0x20, 0x70, // 1
0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
0x90, 0x90, 0xF0, 0x10, 0x10, // 4
0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
0xF0, 0x10, 0x20, 0x40, 0x40, // 7
0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
0xF0, 0x90, 0xF0, 0x90, 0x90, // A
0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
0xF0, 0x80, 0x80, 0x80, 0xF0, // C
0xE0, 0x90, 0x90, 0x90, 0xE0, // D
0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
0xF0, 0x80, 0xF0, 0x80, 0x80}; // F

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

void clear_pixels(uint8_t pixels[WIDTH][HEIGHT])
{
    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            pixels[i][j] = 0;
        }
    }
}

int chip_8_to_keyboard(int chip_8)
{
    for (int i = 0; i < 16; i++)
    {
        if (chip_8_arr[i] == chip_8)
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

SDL_FRect get_frect(int i, int j, int w, int h)
{
    SDL_FRect rect;
    rect.x = i; 
    rect.y = j;
    rect.w = w;
    rect.h = h;
    return rect;
}

SDL_Rect get_rect(int i, int j, int w, int h)
{
    SDL_Rect rect;
    rect.x = i; 
    rect.y = j;
    rect.w = w;
    rect.h = h;
    return rect;
}

void printNum(u_int16_t x)
{
    printf("%" PRIu16 "\n", x);
}

void print_pixels(uint8_t pixels[WIDTH][HEIGHT])
{
    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            printf("%i / ", pixels[i][j]);
        }
    }
}

void pixels_on(uint8_t pixels[WIDTH][HEIGHT])
{
    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            pixels[i][j] = 1;
        }
    }
}

void print_memory(uint8_t memory[4096])
{
    for (int i = 0; i < 4096; i++)
    {
        uint8_t curr = memory[i];
        printf("%u / ", curr);
    }
    printf("\n");
}

int get_scancode_index(Keypad keypad[16], SDL_Scancode scancode)
{
    for (int i = 0; i < 16; i++)
    {
        if (keypad[i].scancode == scancode)
        {
            return i;
        }
    }
    return -1;
}
