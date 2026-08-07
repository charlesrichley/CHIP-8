#include "header.h"

#define WIDTH 64
#define HEIGHT 32

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

// Memory is 4 kB (4096 bytes). 
// Initial space can be left empty (except for fonts)
uint8_t memory[4096];
uint8_t pixels[WIDTH][HEIGHT]; // indexing: pixels[x][y]
Keypad keypad[16];
uint8_t registers[16];
Stack stack;
SDL_FRect rects_arr[WIDTH][HEIGHT];

int main(void)
{
    // Open file (for loading ROM data into memory)
    FILE *ROM_file = fopen("ibm_logo.ch8", "r");
    if (ROM_file == NULL)
    {
        printf("Could not open file (NULL).\n");
        return 1;
    }

    // Determine size of file
    // CHIP-8 program starting at 0x200 (512 in decimal)
    fseek(ROM_file, 0, SEEK_END);
    long size = ftell(ROM_file); // size of file in bytes

    if (size > 3584) // 4096 - 0x200
    {

        printf("File is too large (memory exceeded).\n");
        return 2;
    }

    // Read file into memory starting at index 0x200 (512 in decimal)
    rewind(ROM_file);
    fread(&memory[0x200], 1, size, ROM_file);
    fclose(ROM_file);

    // Allocate memory for fonts, starting at 0x50
    memcpy(&memory[0x50], sprite_arr, sizeof(sprite_arr));

    // Initialise random number generator
    srand(time(NULL));

    // Initialising stack
    initialise(&stack);

    // Initialising keypad
    for (int i = 0; i < 16; i++)
    {
        keypad[i].chip_8 = chip_8_arr[i];
        keypad[i].keyboard = keyboard_arr[i];
        keypad[i].is_on = false;
    }

    // Initialise rect array for rendering pixels
    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            rects_arr[i][j] = find_rect(i, j, 1, 1);
        }
    }

    // Initialise SDL
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Window initialisation failed! Reason: %s\n", SDL_GetError());
    }

    const int scale = 15;  // Scale factor to increase size visually

    SDL_Window* window = SDL_CreateWindow("CHIP-8", WIDTH * scale, HEIGHT * scale, 0);
    if (window == NULL)
    {
        SDL_Log("Window creation failed. Reason: %s\n", SDL_GetError());
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL){
        SDL_Log("Renderer creation failed. Reason: %s\n", SDL_GetError());
    }

    SDL_SetRenderLogicalPresentation(renderer, WIDTH, HEIGHT, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    uint16_t pc = 0x200;
    uint16_t index_register = 0;

    // Timers decremented once per frame (60 Hz)
    uint8_t delay_timer = 0;
    uint8_t sound_timer = 0; // Beeps as long as above zero

    // Event loop
    const float target_frame_time = 1000.0f / 60.0f;  // 60 FPS, so 0.017 seconds per frame.
    bool quitting = false;
    bool key_released = false;
    SDL_Event event;
    SDL_zero(event);

    while (!quitting)
    {
        uint64_t frame_start = SDL_GetPerformanceCounter();
        SDL_RenderClear(renderer);

        while (SDL_PollEvent(&event))
        {
            switch(event.type){
                case SDL_EVENT_QUIT:
                    quitting = true;

                case SDL_EVENT_KEY_UP:
                    key_released = true;
            }
        }

        // Reset pixels to black, which are later rendered based off pixels array
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        for (int i = 0; i < WIDTH; i++)
            for (int j = 0; j < HEIGHT; j++)
            {
                SDL_FRect rect = rects_arr[i][j];
                SDL_RenderFillRect(renderer, &rect);
            }

        // Update timers
        if (sound_timer > 0)
        {
            sound_timer -= 1;
        }
        if (delay_timer > 0)
        {
            delay_timer -= 1;
        }

        // Fetch
        uint16_t instruction_1 = memory[pc] << 8;
        uint16_t instruction_2 = memory[pc+1];
        uint16_t opcode = instruction_1 | instruction_2;
        pc += 2;

        // Decode

        // Opcode is made up of 4 nibbles (each 4 bits), so we use masking and shifting to get each nibble

        uint8_t mask = 0xF; // all 4 bits (1111 in binary)
        uint8_t nibble_1 = (opcode << 12) & mask; // kind of instruction
        uint8_t nibble_2 = (opcode << 8) & mask; // look up one of 16 registers (VX) from V0 to VF
        uint8_t nibble_3 = (opcode << 4) & mask; // look up one of 16 registers (VY) 
        uint8_t nibble_4 = opcode & mask;
        uint8_t N = opcode & 0x000F;
        uint8_t NN = opcode & 0x00FF;
        uint16_t NNN = opcode & 0x0FFF;

        uint8_t x = nibble_2;
        uint8_t y = nibble_3;

        // Execute

        // Keyboard input
        uint8_t curr_key = chip_8_to_keyboard(registers[nibble_2]);
        const bool *keys = SDL_GetKeyboardState(NULL);
        SDL_Scancode scancode = SDL_GetScancodeFromKey(curr_key, NULL);

        // Type of instruction
        switch (nibble_1){
            case 0x0:
                switch(nibble_4){
                    // 00E0: clear screen
                    case 0x0:
                        clear_pixels(pixels, WIDTH, HEIGHT);
                        break;

                    // 00EE: returning from subroutine
                    case 0xE:
                        // Remove last address from stack, set PC to it
                        pc = pop(&stack);
                }

            // 1NNN: jump (set pc = NNN)
            case 0x1:
                pc = NNN;

            // 2NNN: call (push pc to stack and set pc = NNN)
            case 0x2:
            push(&stack, pc);
            pc = NNN;

            // 3XNN: skip conditionally (if Vx == NN, skip)
            case 0x3:
                if (registers[x] == NN)
                {
                    pc += 2;
                }

            // 4XNN: skip conditionally (if Vx != NN, skip)
            case 0x4:
                if (registers[x] != NN)
                {
                    pc += 2;
                }
            
            // 5XY0 skip conditionally (if nibble_1 == x, skip)
            case 0x5:
                if (nibble_1 == x)
                {
                    pc += 2;
                }

            // 6XNN: set (register VX to NN)
            case 0x6:
                registers[x] = NN;

            // 7XNN: add (add NN to VX)
            case 0x7:
                registers[x] += NN;

            case 0x8:
                // decide instruction based on final nibble
                switch(nibble_4){
                    // 8XY0: set
                    case 0x0:
                        registers[x] = registers[y];
                    
                    // 8XY1: binary OR
                    case 0x1:
                        registers[x] = (registers[x] | registers[y]);
                    
                    // 8XY2: binary AND
                    case 0x2:
                        registers[x] = (registers[x] & registers[y]);

                    // 8XY3: logical XOR
                    case 0x3:
                        registers[x] = registers[x] & registers[y];

                    // 8XY4: add
                    case 0x4:
                        // If overflows: V_x + V_y > 255
                        if (registers[x] > 255 - registers[y])
                        {
                            registers[0xF] = 1;
                        }
                        else
                        {
                            registers[0xF] = 0;
                        }
                        registers[x] += registers[y];

                    // 8XY5: subtract
                    case 0x5:
                        registers[x] -= registers[y]; 
                    
                    // 8XY6 and 8XYE: shift (ambigious instruction)
                    case 0x6:{
                        registers[x] = registers[y]; // Optional
                        uint8_t shifted_out = (registers[x] & 1);
                        registers[x] = registers[x] >> 1;

                        if (shifted_out == 1)
                        {
                            registers[0xF] = 1;
                        }
                        else
                        {
                            registers[0xF] = 0;
                        }
                    }

                    // 8XY7: subtract
                    case 0x7:
                        registers[x] = registers[y] - registers[x];

                    // 8XYE: shift
                    case 0xE: {
                        registers[x] = registers[y]; // Optional (ambigious instruction)
                        uint8_t shifted_out = (registers[x] & 1);
                        registers[x] = registers[x] << 1;

                        if (shifted_out == 1)
                        {
                            registers[0xF] = 1;
                        }
                        else
                        {
                            registers[0xF] = 0;
                        }
                    }
                }

            // 9XY0: skip conditionally
            case 0x9:
                if (nibble_1 != x)
                {
                    pc += 2;
                }
            
            // ANNN: set index
            case 0xA:
                index_register = NNN;

            // BNNN: jump with offset (ambiguous)
            case 0xB:
                pc += (NNN + registers[0x0]);

            // CXNN: random
            case 0xC:
                registers[x] = (rand() & NN);
            
            // DXYN: display
            case 0xD:{
                uint8_t x_coord = registers[x] % WIDTH;
                uint8_t y_coord = registers[y] % HEIGHT;
                registers[0XF] = 0;

                for (int row = 0; row < N; row++)
                {
                    if (y_coord >= HEIGHT)
                    {
                        break;
                    }

                    uint8_t sprite = sprite_arr[index_register + row];
                    uint8_t x_max = x_coord + 8;

                    for (int i = 7; i >= 0; i--)
                    {
                        uint8_t sprite_pixel = (sprite >> i) & 0x1;
                        uint8_t screen_pixel = pixels[x_coord][y_coord];

                        if (sprite_pixel == 1 && screen_pixel == 1)
                        {
                            pixels[x_coord][y_coord] = 0;
                            registers[0xF] = 1;
                        }
                        else if (sprite_pixel == 1 && screen_pixel == 0)
                        {
                            pixels[x_coord][y_coord] = 1;
                        }
                        
                        x_coord += 1;
                    }
                    y_coord += 1;
                }
            } 

            case 0xE:
                switch(nibble_4){
                    // EX9E: skip if key
                    case 0xE:
                        if (keys[scancode])
                        {
                            pc += 2;
                        }
                        
                    // EXA1: skip if key
                    case 0x1:
                        if (!(keys[scancode]))
                        {
                            pc += 2;
                        }
                }

            case 0xF:
                switch(nibble_4){
                    // FX07: timer
                    case 0x7:
                        registers[x] = delay_timer;

                    case 0x5:
                        switch(nibble_3){
                            // FX15: timer
                            case 0x1:
                                delay_timer = registers[x];
                            
                            // FX55: store memory
                            case 0x5:
                                for (int i = 0; i <= x; i++)
                                {
                                    memory[index_register + i] = registers[i];
                                }

                            // FX65: load memory (opposite of FX55)
                            case 0x6:
                                for (int i = 0; i <= x; i++)
                                {
                                    registers[i] = memory[index_register + i];
                                }
                        }
                    
                    // FX18: timer
                    case 0x8:
                        sound_timer = registers[x];

                    // FX1E: add to index
                    case 0xE:
                        index_register += registers[x];
                        if (index_register > 0x0FFF)
                        {
                            registers[0xF] = 1;
                        }

                    // FX0A: get key (stops execution, waiting for key input)
                    case 0xA:
                        if (key_released)
                        {
                            registers[x] = keyboard_to_chip_8(curr_key);
                        }
                        else
                        {
                            pc -= 2;
                        }

                    // FX29: font character
                    case 0x9:
                        index_register = 0x50 + registers[x];

                    // FX33: binary-coded decimal conversion
                    case 0x3:{
                        memory[index_register] =  (registers[x] / 100) % 10;
                        memory[index_register + 1] = (registers[x] / 10) % 10;
                        memory[index_register + 2] = registers[x] % 10;
                    }
                }
            }
            
        // Render pixels that are turned on
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i < WIDTH; i++)
        {
            for (int j = 0; j < HEIGHT; j++)
            {
                if (pixels[i][j] == 1)
                {
                    SDL_FRect rect = rects_arr[i][j];
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }
        
        // Measuring time of frame in order to manage FPS
        Uint64 frame_end = SDL_GetPerformanceCounter();
        float elapsed = (frame_end - frame_start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
        if (target_frame_time > elapsed)
        {
            SDL_Delay((Uint32)(target_frame_time - elapsed));
        }

        SDL_RenderPresent(renderer);
    }
    
    if (quitting)
    {
            SDL_DestroyWindow(window);
            SDL_DestroyRenderer(renderer);
            window = NULL;
            SDL_Quit();
    }

    return 0;
}
