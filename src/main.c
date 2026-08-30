#include "header.h"

// Memory is 4 kB (4096 bytes). 
// Initial space can be left empty before 0x200 (except for fonts at 0x50)
uint8_t memory[4096];

uint8_t pixels[WIDTH][HEIGHT];
Keypad keypad[16];
uint8_t registers[16];
Stack stack;
SDL_FRect rects_arr[WIDTH][HEIGHT];

Quirk quirks[NUMBER_OF_QUIRKS];
Button buttons[NUMBER_OF_QUIRKS];

int main(void)
{
    // <----- QUIRKS ----->
    // CHIP-8 has ambigious instructions so there are settings to adjust quirks
    // The defaults are the most standard ones that will allow most games to run
    
    // 8XY1, 8XY2, 8XY3 reset registers[0xF] to 0
    quirks[RESET_VF].is_on = false;
    quirks[RESET_VF].quirk_string = "VF RESET";
    
    // FX55 and FX65 incrementing index register
    quirks[MEMORY].is_on = false;
    quirks[MEMORY].quirk_string = "MEMORY";

    // DXYN only called once per frame 
    quirks[DRAW_WAIT].is_on = false;
    quirks[DRAW_WAIT].quirk_string = "DRAW WAIT";

    // Sprites get clipped instead of wrapping in DXYN
    quirks[CLIPPING].is_on = false;
    quirks[CLIPPING].quirk_string = "CLIPPING";

    // 8XY6 and 8XYE only operate on VX instead of storing shifted VY in VX
    quirks[SHIFTING].is_on = true;
    quirks[SHIFTING].quirk_string = "SHIFTING";

    // BNNN doesn't use V0, but VX instead (X is first nibble in NNN)
    quirks[JUMPING].is_on = false;
    quirks[JUMPING].quirk_string = "JUMPING";

    // FX1E sets VF to 0 if the index registers overflow
    quirks[FX1E_OVERFLOW].is_on = true;
    quirks[FX1E_OVERFLOW].quirk_string = "OVERFLOW";

    // FX0A resumes execution if the key is both pressed and released or simply pressed
    quirks[FX0A_PRESSED_AND_RELEASED].is_on = true;
    quirks[FX0A_PRESSED_AND_RELEASED].quirk_string = "PRESSED";

    char *file_name = open_welcome_window();

    // <----- CHIP-8 PROGRAM STARTS ----->
    
    // Open file (for loading ROM data into memory)
    FILE *ROM_file = fopen(file_name, "r");
    if (ROM_file == NULL)
    {
        // Open up a game we know works instead
        ROM_file = fopen("space_invaders.ch8", "r");
    }

    // Determine size of file
    fseek(ROM_file, 0, SEEK_END);
    long size = ftell(ROM_file); // size of file in bytes

    // CHIP-8 program starts at 0x200 - ensure file doesn't exceed memory capacity
    if (size > (4096 - 0x200))
    {
        SDL_Log("File is too large (memory exceeded).\n");
    }

    // Read file into memory starting at index 0x200
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
            rects_arr[i][j] = get_frect(i, j, 1, 1, false);
        }
    }
    
    SDL_Window* window = SDL_CreateWindow("CHIP-8", WIDTH * SCALE, HEIGHT * SCALE, 0);
    check_window(window, "Could not initialise main window.");

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    check_renderer(renderer, "Could not initialise main renderer.");

    // Allow renderer to adjust to window size (adjusted by scale factor)
    SDL_SetRenderLogicalPresentation(renderer, WIDTH, HEIGHT, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    // Managing keyboard input
    bool fx0a_waiting = false;
    bool fx0a_completed = false;
    uint8_t fx0a_hexadecimal;

    // Initialising program counter and index register
    uint16_t pc = 0x200;
    uint16_t index_register = 0;

    // Timers decremented once per frame (60 Hz)
    uint8_t delay_timer = 0;
    uint8_t sound_timer = 0;
    int curr_sample = 0;

    // Event loop
    const float target_frame_time = 1000 / 60;  // 60 FPS, so 0.017 seconds per frame.
    bool quitting = false;
    bool DXYN_PAUSED = false;
    SDL_Event event;
    SDL_zero(event);

    // Add a small delay before opening emulator
    SDL_Delay(300);

    while (!quitting)
    {
        uint64_t frame_start = SDL_GetTicksNS();
        if (!SDL_RenderClear(renderer))
        {
            SDL_Log("Could not clear renderer during main loop. Reason: %s\n", SDL_GetError());
        }

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

                        // Key only has to be pressed for execution to resume
                        if (!quirks[FX0A_PRESSED_AND_RELEASED].is_on)
                        {
                            fx0a_completed = true;
                            fx0a_waiting = false;
                            fx0a_hexadecimal = chip_8_arr[index_key_down];
                            break;
                        }

                        if (index_key_down != -1)
                        {
                            keypad[index_key_down].is_down = true;
                        }
                    }
                    break;
                }

                case SDL_EVENT_KEY_UP:
                {
                    if (fx0a_waiting && quirks[FX0A_PRESSED_AND_RELEASED].is_on)
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
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        for (int i = 0; i < WIDTH; i++)
        {
            for (int j = 0; j < HEIGHT; j++)
            {
                SDL_FRect rect = rects_arr[i][j];
                SDL_RenderFillRect(renderer, &rect);
            }
        }
        
        // Ensure index register does not overflow
        if (index_register >= 4096)
        {
            SDL_Log("Index register has overflowed. \n");
        }

        // Update sound timer and if value > 0 play beeping sound
        if (sound_timer > 0)
        {
            // Increment sound timer
            sound_timer -= 1;

            // Adapted from SDL documentation of examples
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

        for (int instructions_read = 0; instructions_read < INSRUCTIONS_PER_FRAME; instructions_read++)
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
                
                // 5XY0 skip conditionally (if Vx = Vy, skip)
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
                            registers[x] = orig_registers_x | orig_registers_y;
                            if (quirks[RESET_VF].is_on)
                            {
                                registers[0xF] = 0;
                            }
                            break;
                        
                        // 8XY2: binary AND
                        case 0x2:
                            registers[x] = registers[x] & registers[y];
                            if (quirks[RESET_VF].is_on)
                            {
                                registers[0xF] = 0;
                            }
                            break;

                        // 8XY3: bitwise XOR
                        case 0x3:
                            registers[x] = registers[x] ^ registers[y];
                            if (quirks[RESET_VF].is_on)
                            {
                                registers[0xF] = 0;
                            }
                            break;

                        // 8XY4: add
                        case 0x4: {
                            // If overflows the 8 bits
                            registers[x] = orig_registers_x + orig_registers_y;
                            if (orig_registers_x + orig_registers_y > 255)
                            {
                                registers[0xF] = 1;
                            }
                            else
                            {
                                registers[0xF] = 0;
                            }
                            break;
                        }

                        // 8XY5: subtract
                        case 0x5: {
                            // Affects the carry flag
                            uint8_t orig_registers_x = registers[x];
                            uint8_t orig_registers_y = registers[y];
                            registers[x] = orig_registers_x - orig_registers_y;
                            if (orig_registers_x >= orig_registers_y)
                            {
                                registers[0xF] = 1;
                            }
                            else
                            {
                                registers[0xF] = 0;
                            }
                            break;
                        }
                        
                        // 8XY6 and 8XYE: shift 
                        case 0x6: {
                            if (!quirks[SHIFTING].is_on)
                            {
                                registers[x] = registers[y]; 
                            }
                            uint8_t shifted_out = registers[x] & 1;
                            registers[x] >>= 1;
                            registers[0xF] = shifted_out;
                            break;
                        }

                        // 8XY7: subtract
                        case 0x7: 
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
                            if (!quirks[SHIFTING].is_on)
                            {
                                registers[x] = registers[y]; 
                            }
                            uint8_t shifted_out = (registers[x] >> 7) & 1;
                            registers[x] <<= 1;
                            registers[0xF] = shifted_out;
                            break;
                        }
                        break;
                    }
                    break;
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
                    pc = NNN + registers[0x0];

                    // CHIP-8 and SUPER-CHIP change BXNN (could be accidentally)
                    // Where pc jumps to XNN + registers[x]
                    if (quirks[JUMPING].is_on)
                    {
                        pc = (((x << 8) | NN)) + registers[x];
                    }
                    break;

                // CXNN: random
                case 0xC:
                    registers[x] = rand() & NN;
                    break;
                
                // DXYN: display (drawing instruction)
                case 0xD: {
                    if (DXYN_PAUSED)
                    {
                        pc -= 2;
                        break;
                    }

                    uint8_t x_start = registers[x] % WIDTH;
                    uint8_t y_coord = registers[y] % HEIGHT;
                    registers[0xF] = 0;

                    for (int row = 0; row < N; row++)
                    {
                        if (y_coord >= HEIGHT || y_coord < 0)
                        {
                            if (quirks[CLIPPING].is_on)
                            {
                                break;
                            }
                            else
                            {
                                y_coord %= HEIGHT;
                            }
                        }

                        uint8_t x_coord = x_start;
                        uint8_t sprite = memory[index_register + row];
                        uint8_t x_max = x_coord + 8;

                        for (int i = 7; i >= 0; i--)
                        {
                            if (x_coord >= WIDTH || x_coord < 0)
                            {
                                if (quirks[CLIPPING].is_on)
                                {
                                    break;
                                }
                                else
                                {
                                    x_coord *= WIDTH;
                                }
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
                    
                    if (quirks[DRAW_WAIT].is_on)
                    {
                        DXYN_PAUSED = true;
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
                        // FX07: timer (set VX to value of delay timer)
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
                                    if (quirks[MEMORY].is_on)
                                    {
                                        index_register+= (x + 1);
                                    }
                                    break;

                                // FX65: load memory (opposite of FX55)
                                case 0x6:
                                    for (int i = 0; i <= x; i++)
                                    {
                                        registers[i] = memory[index_register + i];
                                    }
                                    if (quirks[MEMORY].is_on)
                                    {
                                        index_register+= (x + 1);
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

                            // On original COSMAC VIP, VF was not affected on overflow
                            // However, CHIP-8 interpreter for Amiga would set VF to 1 on overflow
                            if (quirks[FX1E_OVERFLOW].is_on)
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
                                pc -= 2;
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

        // Reset DXYN_PAUSED at end of loop
        if (quirks[DRAW_WAIT].is_on)
        {
            DXYN_PAUSED = false;
        }

        if (!SDL_RenderPresent(renderer))
        {
            SDL_Log("Could not render current CHIP-8 display. Reason: %s\n", SDL_GetError());
        }

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
        SDL_Quit();

        // Free memory allocated in welcome.c for the ROM file name
        free(file_name);
    }

    return 0;
}
