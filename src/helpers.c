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

// Returns rectangle at coordinates (i, j) width w height h
SDL_FRect get_frect(int i, int j, int w, int h, bool is_centered)
{
    SDL_FRect rect;
    if (is_centered == true)
    {
        rect.x = i - w/2;
        rect.y = j - h/2;
    }
    else 
    {
        rect.x = i;
        rect.y = j;
    }

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

// <----- UI FUNCTIONS ----->

// Surface -> Texture -> Render text
Button get_button(int x, int y, char *string, bool is_on, TTF_Font *font, SDL_Color font_color, SDL_Renderer *renderer, SDL_FRect rect, SDL_FRect title_rect)
{
    // Initialise button
    Button button;

    SDL_Surface *surface = TTF_RenderText_Blended(font, string, strlen(string), font_color);
    if (!surface)
    {
        SDL_Log("Could not initialise surface. Reason: %s\n", SDL_GetError());
    }

    // Initialsing texture for title
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture)
    {
        SDL_Log("Could not initialise texture. Reason: %s\n", SDL_GetError());
    }

    button.button_rect = rect;
    button.title_rect = title_rect;
    button.surface = surface;
    button.texture = texture;
    button.string = string;
    button.is_on = is_on;
    button.x = x;
    button.y = y;

    return button;
}
