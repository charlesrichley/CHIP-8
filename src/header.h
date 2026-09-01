#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <inttypes.h>

// Define constants for CHIP-8 emulation
#define WIDTH 64
#define HEIGHT 32
#define SCALE 16
#define INSRUCTIONS_PER_FRAME 12

// CHIP-8 has ambigious specification, so different ROMS rely on different quirks
#define NUMBER_OF_QUIRKS 8

// Constants for buttons to toggle quirks on and off
#define BUTTON_WIDTH 11
#define BUTTON_HEIGHT 4

// Number of games in the top left menu
#define NUMBER_OF_GAMES 3

typedef struct 
{
    uint16_t arr[16];
    int top;
} Stack;

typedef struct 
{
    char *quirk_string;
    bool is_on;
} Quirk;

typedef struct 
{
    int chip_8;
    char keyboard;
    SDL_Scancode scancode;
    bool is_down;
} Keypad;

typedef struct
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_FRect button_rect;
    SDL_FRect text_rect;
    SDL_FRect title_rect;
    char *string;
    int x;
    int y;
    bool is_on;
    bool just_changed;
} Button;

typedef struct 
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_FRect text_rect;
    SDL_FRect box_rect;
    int x;
    int y;
    char *name;
    char *file_name;
} Game;

typedef struct 
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_FRect text_rect;
    char *file_name;
    char *user_file_name;
    char *display_name;
    bool user_input_changed;
    bool has_changed;
} File;

enum {
    RESET_VF,
    MEMORY,
    DRAW_WAIT,
    CLIPPING,
    SHIFTING,
    JUMPING,
    FX1E_OVERFLOW,
    FX0A_PRESSED_AND_RELEASED
};

enum {
    PONG,
    TETRIS,
    SPACE_INVADERS
};

extern int chip_8_arr[16];
extern char keyboard_arr[16];
extern SDL_Scancode scancode_arr[16];
extern uint8_t sprite_arr[80];

extern Quirk quirks[NUMBER_OF_QUIRKS];
extern Button buttons[NUMBER_OF_QUIRKS];

// Main function in helpers.c that runs the welcome screen
char *open_welcome_window(void);

// Callback function for opening a file
void callback(void *userdata, const char * const *filelist, int filter);

// Initialises a stack
void initialise(Stack *stack);

// Inserts element to top of stack
void push(Stack *stack, uint16_t new_value);

// Removes top element of stack (and returns the popped integer)
int pop(Stack *stack);

// Returns index when given keyboard key
int keyboard_to_index(char c);

// Returns index when given scancode
int scancode_to_index(Keypad keypad[16], SDL_Scancode scancode);

// Returns keyboard key when given CHIP-8 key
int chip_8_to_keyboard(int chip_8);

// Returns a rect at coordinates i, j, with width w height h
// if is_centered is false if i, ) is the top left of the rect
SDL_FRect get_frect(int i, int j, int w, int h, bool is_centered);

// Creates a button when given all necessary features
Button get_button(int x, int y, char *string, bool is_on, TTF_Font *font, SDL_Color font_color, SDL_Renderer *renderer, SDL_FRect rect, SDL_FRect title_rect);

// Checks NULL for all SDL types
void check_renderer(SDL_Renderer *renderer, char *message);
void check_window(SDL_Window *window, char *message);
void check_surface(SDL_Surface *surface, char *message);
void check_texture(SDL_Texture *texture, char *message);
void check_font(TTF_Font *font, char *message);
