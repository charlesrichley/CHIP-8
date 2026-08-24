#include "header.h"

int chip_8_arr[16] = {0x1, 0x2, 0x3, 0xC, 0x4, 0x5, 0x6, 0xD, 0x7, 0x8, 0x9, 0xE, 0xA, 0x0, 0xB, 0xF};
char keyboard_arr[16] = {'1', '2', '3', '4', 'q', 'w', 'e', 'r', 'a', 's', 'd','f', 'z', 'x', 'c', 'v'};
SDL_Scancode scancode_arr[16] = {SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4, SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R, SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F, SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_C, SDL_SCANCODE_V};

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

// Checks if a stack is empty 
bool is_empty(Stack *stack)
{
    return (stack->top == -1);
}

// Initialises a stack
void initialise(Stack *stack)
{
    stack->top = -1;
}

// Inserts element to top of stack
void push(Stack *stack, uint16_t new_value)
{
    stack->arr[(stack->top) + 1] = new_value;
    stack->top += 1;
}

// Removes top element of stack (and returns the popped integer)
int pop(Stack *stack)
{
    int popped_number = stack->arr[stack->top];
    stack->top -= 1;
    return popped_number;
}

// Turns every pixel on (sets value to 1)
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

// Returns rectangle with top left at coordinates (i,j) width w and height h
SDL_FRect get_frect_TL(int i, int j, int w, int h)
{
    SDL_FRect rect;
    rect.x = i; 
    rect.y = j;
    rect.w = w;
    rect.h = h;
    return rect;
}

// Returns rectangle centered at coordinates (i, j) width w height h
SDL_FRect get_frect_centered(int i, int j, int w, int h)
{
    SDL_FRect rect;
    rect.x = i - w/2;
    rect.y = j - h/2;
    rect.w = w;
    rect.h = h;
    return rect;
}

// Gets CHIP-8 key from keyboard key
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

// Gets keyboard key from CHIP-8 key
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

// Given keyboard key return index
int keyboard_to_index(char c)
{
    for (int i = 0; i < 16; i++)
    {
        if (keyboard_arr[i] == c)
        {
            return i;
        }
    }
    return -1;
}

// Returns index of scancode value in keypad array
int scancode_to_index(Keypad keypad[16], SDL_Scancode scancode)
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
