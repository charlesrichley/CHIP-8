#include "header.h"

char *file_name = "4-flags.ch8";

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
    int INDEX_SS = 0;
    // memory[0x1FF] = 3; 

    // Open file (for loading ROM data into memory)
    FILE *ROM_file = fopen(file_name, "r");
    if (ROM_file == NULL)
    {
        SDL_Log("Could not open file (NULL).\n");
        return 1;
    }

    // Determine size of file
    fseek(ROM_file, 0, SEEK_END);
    long size = ftell(ROM_file); // size of file in bytes

    // CHIP-8 program starting at 0x200 - ensure file doesn't exceed capacity
    if (size > (4096 - 0x200))
    {
        SDL_Log("File is too large (memory exceeded).\n");
        return 1;
    }

    // Read file into memory starting at index 0x200 (512 in decimal)
    rewind(ROM_file);
    fread(&memory[0x200], sizeof(uint8_t), size, ROM_file);
    fclose(ROM_file);

    // Allocate memory for fonts, starting at 0x50
    memcpy(&memory[0x50], sprite_arr, sizeof(sprite_arr));

    // Initialise random number generator
    srand(time(NULL));

    // Initialise stack
    initialise(&stack);

    // Initialise keypad
    for (int i = 0; i < 16; i++)
    {
        keypad[i].chip_8 = chip_8_arr[i];
        keypad[i].keyboard = keyboard_arr[i];
        keypad[i].scancode = scancode_arr[i];
        keypad[i].is_down = false;
    }

    // Initialise rect array for rendering pixels
    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            rects_arr[i][j] = get_frect(i, j, 1, 1);
        }
    }

    // Initialise SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        SDL_Log("Window initialisation failed! Reason: %s\n", SDL_GetError());
    }

    SDL_Window* window = SDL_CreateWindow("CHIP-8", WIDTH * SCALE, HEIGHT * SCALE, 0);
    if (window == NULL)
    {
        SDL_Log("Window creation failed. Reason: %s\n", SDL_GetError());
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL){
        SDL_Log("Renderer creation failed. Reason: %s\n", SDL_GetError());
    }

    // Allow renderer to adjust to window size (adjusted by scale factor)
    SDL_SetRenderLogicalPresentation(renderer, WIDTH, HEIGHT, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    // Initialising program counter and index register
    uint16_t pc = 0x200;
    uint16_t index_register = 0;

    // Timers decremented once per frame (60 Hz)
    uint8_t delay_timer = 0;
    uint8_t sound_timer = 0;
    int curr_sample = 0;

    // Managing keyboard input
    bool fx0a_waiting = false;
    bool fx0a_completed = false;
    uint8_t fx0a_hexadecimal;

    // Event loop
    const float target_frame_time = 1000.0f / 60.0f;  // 60 FPS, so 0.017 seconds per frame.
    bool quitting = false;
    SDL_Event event;
    SDL_zero(event);

    while (!quitting)
    {
        uint64_t frame_start = SDL_GetTicksNS();
        SDL_RenderClear(renderer);

        while (SDL_PollEvent(&event))
        {
            switch(event.type){
                case SDL_EVENT_QUIT:
                    quitting = true;
                    break;

                case SDL_EVENT_KEY_DOWN: 
                {
                    if (fx0a_waiting)
                    {
                        SDL_Scancode event_scancode = event.key.scancode;
                        int index_key_down = scancode_to_index(keypad, event_scancode);
                        if (index_key_down != -1)
                        {
                            keypad[index_key_down].is_down = true;
                        }
                    }
                    break;
                }

                case SDL_EVENT_KEY_UP:
                {
                    if (fx0a_waiting)
                    {
                        SDL_Scancode event_scancode = event.key.scancode;
                        int index_key_up = scancode_to_index(keypad, event_scancode);
                        if (index_key_up != -1)
                        {
                            if (keypad[index_key_up].is_down == true)
                            {
                                keypad[index_key_up].is_down = false;
                                fx0a_completed = true;
                                fx0a_waiting = false;
                                fx0a_hexadecimal = chip_8_arr[index_key_up];
                            }
                        }
                    }
                    break; 
                }  
            }
        }

        // Reset pixels to black, which are later rendered based off pixels array
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        for (int i = 0; i < WIDTH; i++)
        {
            for (int j = 0; j < HEIGHT; j++)
            {
                SDL_FRect rect = rects_arr[i][j];
                SDL_RenderFillRect(renderer, &rect);
            }
        }
        
        // Ensure index registe does not overflow
        if (index_register >= 4096)
        {
            SDL_Log("Index register has overflowed. \n");
            return 1;
        }

        // Update sound timer
        if (sound_timer > 0)
        {
            sound_timer -= 1;

            SDL_AudioSpec spec;

            spec.channels = 1;
            spec.format = SDL_AUDIO_F32;
            spec.freq = 8000;

            SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
            if (!stream)
            {
                SDL_Log("Couldn't create audio stream. Reason: %s\n", SDL_GetError());
            }
            SDL_ResumeAudioStreamDevice(stream);  
            
            const int minimum_audio = (spec.freq * sizeof(float)) / 60;
            float samples[512];
            const int freq = 440;
            while (SDL_GetAudioStreamQueued(stream) < minimum_audio)
            {
                for (int i = 0; i < SDL_arraysize(samples); i++)   
                {
                    int phase = (curr_sample * freq) / spec.freq;
                    if (phase % 2 == 0)
                    {
                        samples[i] = 1;
                    }
                    else
                    {
                        samples[i] = -1;
                    }
                    curr_sample++;
                }
                SDL_PutAudioStreamData(stream, samples, sizeof(samples));
            }
        }

        // Update delay timer
        if (delay_timer > 0)
        {
            delay_timer -= 1;
        }

        for (int instructions_read = 0; instructions_read < INSRUCTIONS_PER_FRAME; instructions_read++, INDEX_SS++)
        {
            // Fetch
            uint16_t instruction_1 = memory[pc] << 8;
            uint16_t instruction_2 = memory[pc + 1];
            uint16_t opcode = instruction_1 | instruction_2;
            pc += 2;

            // Decode: opcode is made up of 4 nibbles (each 4 bits)
            // we use masking and shifting to get each nibble

            uint8_t mask = 0xF; // 4 rightmost bits (1111 in binary)
            uint8_t nibble_1 = (opcode >> 12) & mask; // find the kind of instruction
            uint8_t nibble_2 = (opcode >> 8) & mask; // VX register
            uint8_t nibble_3 = (opcode >> 4) & mask; // VY register
            uint8_t nibble_4 = opcode & mask;
            uint8_t N = nibble_4;
            uint8_t NN = opcode & 0x00FF;
            uint16_t NNN = opcode & 0x0FFF;

            uint8_t x = nibble_2;
            uint8_t y = nibble_3;

            // Keyboard input for EX9E and EXA1 - 
            // if key is CURRENTLY being held down, which is different to FX0A
            int curr_key = chip_8_to_keyboard(registers[x]);
            const bool *ex_keyboard = SDL_GetKeyboardState(NULL);
            if (ex_keyboard == NULL)
            {
                SDL_Log("Keyboard is NULL. Reason: %s\n", SDL_GetError());
            }
            SDL_Scancode ex_scancode = keypad[keyboard_to_index(curr_key)].scancode;

            // Execute
            switch (nibble_1){
                case 0x0:
                    switch(nibble_4){
                        // 00E0: clear screen
                        case 0x0:
                            for (int i = 0; i < WIDTH; i++)
                            {
                                for (int j = 0; j < HEIGHT; j++)
                                {
                                    pixels[i][j] = 0;
                                }
                            }
                            break;

                        // 00EE: remove last address from stack and set pc to address
                        case 0xE:
                            pc = pop(&stack);
                            break;
                    }
                    break;

                // 1NNN: jump (set pc = NNN)
                case 0x1:
                    pc = NNN;
                    break;

                // 2NNN: call (push pc to stack and set pc = NNN)
                case 0x2:
                    push(&stack, pc);
                    pc = NNN;
                    break;

                // 3XNN: skip conditionally (if Vx == NN, skip)
                case 0x3:
                    if (registers[x] == NN)
                    {
                        pc += 2; 
                    }
                    break;

                // 4XNN: skip conditionally (if Vx != NN, skip)
                case 0x4:
                    if (registers[x] != NN)
                    {
                        pc += 2;
                    }
                    break;
                
                // 5XY0 skip conditionally (if VX = VY, skip)
                case 0x5:
                    if (registers[x] == registers[y])
                    {
                        pc += 2;
                    }
                    break;

                // 6XNN: set (register VX to NN)
                case 0x6:
                    registers[x] = NN;
                    break;

                // 7XNN: add (add NN to VX)
                case 0x7:
                    registers[x] += NN;
                    break;

                case 0x8: {
                    uint8_t orig_registers_x = registers[x];
                    uint8_t orig_registers_y = registers[y];
                    switch(nibble_4){
                        // 8XY0: set
                        case 0x0:
                            registers[x] = registers[y];
                            break;
                        
                        // 8XY1: binary OR
                        case 0x1:
                            printf("Original registers[0xF]: %d\n", registers[0xF]);
                            printf("x: %d\ny: %d\n", x, y);
                            printf("Registers[x]: %d\n", orig_registers_x);
                            printf("Registers[y]: %d\n", orig_registers_y);
                            registers[x] = orig_registers_x | orig_registers_y;
                            printf("Registers[x] after OR operation: %d\n", registers[x]);
                            // registers[0xF] = 0;
                            printf("Registers[0xF] after being set: %d\n\n", registers[0xF]);
                            break;
                        
                        // 8XY2: binary AND
                        case 0x2:
                            registers[x] = registers[x] & registers[y];
                            break;

                        // 8XY3: logical XOR
                        case 0x3:
                            registers[x] = registers[x] ^ registers[y];
                            break;

                        // 8XY4: add
                        case 0x4: {
                            // If overflows the 8 bits
                            if (orig_registers_x + orig_registers_y < 255)
                            {
                                registers[0xF] = 1;
                            }
                            else
                            {
                                registers[0xF] = 0;
                            }
                            registers[x] = orig_registers_x + orig_registers_y;
                            break;
                        }

                        // 8XY5: subtract
                        case 0x5: {
                            // Affects the carry flag
                            uint8_t orig_registers_x = registers[x];
                            uint8_t orig_registers_y = registers[y];
                            if (orig_registers_x >= orig_registers_y)
                            {
                                registers[0xF] = 1;
                            }
                            else
                            {
                                registers[0xF] = 0;
                            }
                            registers[x] = orig_registers_x - orig_registers_y;
                            break;
                        }
                        
                        // 8XY6 and 8XYE: shift (ambigious instruction)
                        case 0x6:{
                            // registers[x] = registers[y]; // Optional
                            uint8_t shifted_out = registers[x] & 1;
                            registers[x] >>= 1;
                            registers[0xF] = shifted_out;
                            break;
                        }

                        // 8XY7: subtract
                        case 0x7:
                            // Affects the carry flag
                            registers[x] = registers[y] - registers[x];
                            if (registers[y] >= registers[x])
                            {
                                registers[0xF] = 1;
                            }
                            else
                            {
                                registers[0xF] = 0;
                            }
                            break;

                        // 8XYE: shift (ambigious instruction)
                        case 0xE: {
                            // registers[x] = registers[y]; // Optional 
                            uint8_t shifted_out = registers[x] & 1;
                            registers[x] <<= 1;

                            if (shifted_out == 1)
                            {
                                registers[0xF] = 1;
                            }
                            else
                            {
                                registers[0xF] = 0;
                            }
                            break;
                        }
                        break;
                    }
                }
                // 9XY0: skip conditionally
                case 0x9:
                    if (registers[x] != registers[y])
                    {
                        pc += 2;
                    }
                    break;
                
                // ANNN: set index
                case 0xA:
                    index_register = NNN;
                    break;

                // BNNN: jump with offset (ambiguous)
                case 0xB:
                    pc += (NNN + registers[0x0]);
                    // CHIP-8 and SUPER-CHIP change to BXNN (could be accidentally)
                    // Where pc jumps to XNN + registers[x]
                    // pc += (((x << 8) | NN)) + registers[x]);
                    break;

                // CXNN: random
                case 0xC:
                    registers[x] = rand() & NN;
                    break;
                
                // DXYN: display (drawing instruction)
                case 0xD:
                {
                    uint8_t x_start = registers[x] % WIDTH;
                    uint8_t y_coord = registers[y] % HEIGHT;
                    registers[0xF] = 0;

                    for (int row = 0; row < N; row++)
                    {
                        if (y_coord >= HEIGHT || y_coord < 0)
                        {
                            break;
                        }
                        uint8_t x_coord = x_start;
                        uint8_t sprite = memory[index_register + row];
                        uint8_t x_max = x_coord + 8;

                        for (int i = 7; i >= 0; i--)
                        {
                            if (x_coord >= WIDTH || x_coord < 0)
                            {
                                break;
                            }

                            uint8_t sprite_pixel = (sprite >> i) & 1;
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
                    break;
                } 

                case 0xE:
                    switch(nibble_4){
                        // EX9E: skip if key
                        case 0xE:
                            if (ex_keyboard[ex_scancode] && curr_key != -1)
                            {
                                pc += 2;
                            }
                            break;
                            
                        // EXA1: skip if key
                        case 0x1:
                            if (!ex_keyboard[ex_scancode] && curr_key != -1)
                            {
                                pc += 2;
                            }
                            break;
                    }
                    break;

                case 0xF:
                    switch(nibble_4){
                        // FX07: timer
                        case 0x7:
                            registers[x] = delay_timer;
                            break;

                        case 0x5:
                            switch(nibble_3){
                                // FX15: timer
                                case 0x1:
                                    delay_timer = registers[x];
                                    break;
                                
                                // FX55: store memory
                                case 0x5:
                                    for (int i = 0; i <= x; i++)
                                    {
                                        memory[index_register + i] = registers[i];
                                    }
                                    break;

                                // FX65: load memory (opposite of FX55)
                                case 0x6:
                                    for (int i = 0; i <= x; i++)
                                    {
                                        registers[i] = memory[index_register + i];
                                    }
                                    break;
                            }
                            break;
                        
                        // FX18: timer
                        case 0x8:
                            sound_timer = registers[x];
                            break;

                        // FX1E: add to index
                        case 0xE:
                            index_register += registers[x];
                            if (index_register > 0x0FFF)
                            {
                                registers[0xF] = 1;
                            }
                            break;

                        // FX0A: get key (stops execution and waits for key input)
                        case 0xA:
                            // Already been initialised and waiting for input
                            if (fx0a_waiting && !fx0a_completed)
                            {
                                pc -= 2;
                            }
                            // Key has been pressed down after waiting period
                            else if (fx0a_completed && !fx0a_waiting)
                            {
                                registers[x] = fx0a_hexadecimal;
                                fx0a_completed = false;
                            }
                            // Not been initalised
                            else if (!fx0a_waiting && !fx0a_completed)
                            {
                                fx0a_waiting = true;
                            }
                            break;

                        // FX29: font character
                        case 0x9:
                            index_register = 0x50 + registers[x];
                            break;

                        // FX33: binary-coded decimal conversion
                        case 0x3:{
                            memory[index_register] =  (registers[x] / 100) % 10;
                            memory[index_register + 1] = (registers[x] / 10) % 10;
                            memory[index_register + 2] = registers[x] % 10;
                            break;
                        }
                        break;
                    }
                    break;
            }
        }

        // Render pixels that are turned on
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
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

        // Take screenshots for documentation
        if (INDEX_SS > 70 && INDEX_SS % 100 == 0)
        {
            SDL_Surface *ss_surface = SDL_RenderReadPixels(renderer, NULL);
            if (ss_surface == NULL)
            {
                SDL_Log("Screenshot failed! Reason: %s\n", SDL_GetError());
            }
            if (!IMG_SavePNG(ss_surface, "output.png"))
            {
                SDL_Log("Could not save screenshot! Reason: %s\n", SDL_GetError());
            }
            SDL_DestroySurface(ss_surface);
        }

        SDL_RenderPresent(renderer);

        // Measuring time of frame for managing FPS
        Uint64 frame_end = SDL_GetTicksNS();
        float elapsed = (frame_end - frame_start) / 1e6f; // Convert from NS to MS
        if (target_frame_time > elapsed)
        {
            SDL_Delay((Uint32)(target_frame_time - elapsed));
        }
    }

    if (quitting)
    {
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        window = NULL;
        renderer = NULL;
        SDL_Quit();
    }

    return 0;
}
