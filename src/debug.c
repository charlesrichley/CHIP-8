#include <header.h>

void print_keypad(Keypad keypad[16])
{
    for (int i = 0; i < 16; i++)
    {
        printf("chip_8: %d\n", keypad[i].chip_8);
        printf("keyboard: %c\n", keypad[i].keyboard);
        printf("SDL_scancode: %d\n", keypad[i].scancode);
        printf("\n");
    }
    printf("\n");
}

void print_chip_8_arr(int chip_8_arr[16])
{
    for (int i = 0; i < 16; i++)
    {
        printf("%d\n", chip_8_arr[i]);
    }
    printf("\n");
}

// Prints every value in CHIP-8 memory
void print_memory(uint8_t memory[4096])
{
    for (int i = 0; i < 4096; i++)
    {
        uint8_t curr = memory[i];
        printf("%u / ", curr);
    }
    printf("\n");
}

// Prints a 16 bit unsigned integer
void printNum(u_int16_t x)
{
    printf("%" PRIu16 "\n", x);
}

// Prints value of all pixels seperated by a forward slash /
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
