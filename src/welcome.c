#include "header.h"

char *open_welcome_window(void)
{
    // Initialise SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        SDL_Log("SDL video/audio initialisation failed! Reason: %s\n", SDL_GetError());
    }

    // Open window for game and settings selection
    SDL_Window* welcome_window = SDL_CreateWindow("CHIP-8", WIDTH * SCALE, HEIGHT * SCALE, 0);
    if (welcome_window == NULL)
    {
        SDL_Log("Window creation failed. Reason: %s\n", SDL_GetError());
    }

    SDL_Renderer* welcome_renderer = SDL_CreateRenderer(welcome_window, NULL);
    if (welcome_renderer == NULL){
        SDL_Log("Renderer creation failed. Reason: %s\n", SDL_GetError());
    }

    // Allow renderer to adjust to window size (adjusted by scale factor)
    SDL_SetRenderLogicalPresentation(welcome_renderer, WIDTH, HEIGHT, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    if (!SDL_StartTextInput(welcome_window))
    {
        SDL_Log("Could not get text input. Reason: %s\n", SDL_GetError());
    }

    // Initialise TTF
    if (!TTF_Init())
    {
        SDL_Log("Could not initialise TTF. Reason: %s\n", SDL_GetError());
    }

    // Font settings for title (white color, completely solid)
    SDL_Color font_color = {255, 255, 255, 255};

    // Font settings for buttons (green and red)
    SDL_Color on_color = {0, 255, 0, 255};
    SDL_Color off_color = {255, 0, 0, 255};

    // Variables for getting user input
    bool has_changed = false;
    bool typing = true;
    bool quitting = false;
    bool is_first_loop = true;
    Input_String input_string;
    input_string.input[0] = '\0';
    input_string.length = 0;
    SDL_Event event;
    SDL_zero(event);

    // Initialising font for title
    const float title_font_size = 120; // Higher font size so text is less pixelated
    TTF_Font *font = TTF_OpenFont("bold_font.ttf", title_font_size);
    if (!font)
    {
        SDL_Log("Could not initialise font. Reason: %s\n", SDL_GetError());
    }

    // White text box so users can see their keyboard input (only a border of a rect)
    const float text_font_size = 200;
    TTF_Font *text_font = TTF_OpenFont("jetbrains_mono.ttf", text_font_size);
    if (!font)
    {
        SDL_Log("Could not initialise text font. Reason: %s\n", SDL_GetError());
    }

    // Initialising surface for title
    char *welcome_text = "CHIP-8";
    SDL_Surface *welcome_surface = TTF_RenderText_Blended(font, welcome_text, strlen(welcome_text), font_color);
    if (!welcome_surface)
    {
        SDL_Log("Could not initialise surface. Reason: %s\n", SDL_GetError());
    }

    // Initialsing texture for title
    SDL_Texture *welcome_texture = SDL_CreateTextureFromSurface(welcome_renderer, welcome_surface);
    if (!welcome_texture)
    {
        SDL_Log("Could not initialise texture. Reason: %s\n", SDL_GetError());
    }

    // Get rectangles for rendering title
    const SDL_FRect title_rect = get_frect(32, 3, 20, 10, true);

    // Get rectangles for rendering text input and text box
    const int input_x = 42;
    const int input_y = 10;
    const SDL_FRect box_rect = get_frect(input_x, input_y, 30, 6, true);
    const SDL_FRect input_rect = get_frect(input_x, input_y, 25, 4, true);

    // Initialise input surfaces and textures
    SDL_Surface *welcome_surface_input = NULL;
    SDL_Texture *welcome_texture_input = NULL;
    const char *initial_text_input = "ROM file name";

    // Initialise continue button
    char *continue_button_text = "CONTINUE";

    SDL_Surface *continue_surface = TTF_RenderText_Blended(font, continue_button_text, strlen(continue_button_text), font_color);
    if (!continue_surface)
    {
        SDL_Log("Could not create surface for rendering the continue button/text. Reason: %s\n", SDL_GetError());
    }

    SDL_Texture *continue_texture = SDL_CreateTextureFromSurface(welcome_renderer, continue_surface);
    if (!continue_texture)
    {
        SDL_Log("Could not create texture for rendering the continue button/text. Reason: %s\n", SDL_GetError());
    }
    
    // Get rect for rendering continue button
    int continue_x = 55;
    int continue_y = 3;
    int continue_box_width = 12;
    int continue_box_height = 6;

    const SDL_FRect continue_box_rect = get_frect(continue_x, continue_y, continue_box_width, continue_box_height, true);
    const SDL_FRect continue_text_rect = get_frect(continue_x, continue_y, 10, 4, true);

    // Variables for drawing buttons
    const int x_start = 15;
    const int y_start = 20;
    const int x_buffer = 2;
    const int y_buffer = 4;

    int row_number = 0;
    int column_number = 0;

    // Initialise buttons
    for (int i = 0; i < NUMBER_OF_QUIRKS; i++, column_number++)
    {
        // We want 2 buttons in each horizontal row
        if (i % 4 == 0 && i != 0)
        {
            row_number++;
            column_number = 0;
        }

        int x = x_start + column_number * BUTTON_WIDTH + x_buffer * column_number;
        int y = y_start + row_number * BUTTON_HEIGHT + y_buffer * row_number;

        const SDL_FRect button_rect = get_frect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, true);
        const SDL_FRect button_title_rect = get_frect(x, y - ((y_buffer + 1) / 1.35), BUTTON_WIDTH, (y_buffer + 1), true);

        buttons[i] = get_button(x, y, quirks[i].quirk_string, quirks[i].is_on, font, font_color, welcome_renderer, button_rect, button_title_rect);
    }

    // Initialise text for buttons
    for (int i = 0; i < NUMBER_OF_QUIRKS; i++)
    {
        SDL_SetRenderDrawColor(welcome_renderer, 255, 255, 255, 255);
        buttons[i].surface = TTF_RenderText_Blended(text_font, quirks[i].quirk_string, strlen(quirks[i].quirk_string), font_color);

        if (buttons[i].surface == NULL)
        {
            SDL_Log("Button title surface is NULL. Reason: %s\n", SDL_GetError());
        }

        buttons[i].texture = SDL_CreateTextureFromSurface(welcome_renderer, buttons[i].surface);
        
        if (buttons[i].texture == NULL)
        {
            SDL_Log("Button texture is NULL. Reason: %s\n", SDL_GetError());
        }
    }

    // Create popular games menu
    Game games[NUMBER_OF_GAMES];

    games[PONG].name = "PONG";
    games[PONG].file_name = "pong.ch8";

    games[TETRIS].name = "TETRIS";
    games[TETRIS].file_name = "tetris.ch8";

    games[SPACE_INVADERS].name = "SPACE INVADERS";
    games[SPACE_INVADERS].file_name = "space_invaders.ch8";

    const int menu_x_start = 8;
    const int menu_y_start = 3;
    const int menu_width = 10;
    const int menu_height = 3;
    const int menu_y_buffer = 1;

    int menu_box_width = menu_width + 4;
    int menu_box_height = y_buffer + 1;

    for (int i = 0; i < NUMBER_OF_GAMES; i++)
    {
        SDL_Surface *curr_surface = TTF_RenderText_Blended(font, games[i].name, strlen(games[i].name), font_color);
        if (!curr_surface)
        {
            SDL_Log("Could not create surface for games menu. Reason: %s\n", SDL_GetError());
        }

        SDL_Texture *curr_texture = SDL_CreateTextureFromSurface(welcome_renderer, curr_surface);
        if (!curr_texture)
        {
            SDL_Log("Could not create surface for games menu. Reason: %s\n", SDL_GetError());;
        }

        // Store surface and texture
        games[i].texture = curr_texture;
        games[i].surface = curr_surface;

        // Get rect for rendering later in the while loop
        int curr_x = menu_x_start;
        int curr_y = menu_y_start + (menu_height + menu_y_buffer) * i;

        games[i].x = curr_x;
        games[i].y = curr_y;

        games[i].text_rect = get_frect(curr_x, curr_y, menu_width, menu_height, true);
        games[i].box_rect = get_frect(curr_x, curr_y, menu_box_width, menu_box_height, true);
    }

    while (!quitting)
    {
        // Reset colour to black before clearing for black background
        SDL_SetRenderDrawColor(welcome_renderer, 0, 0, 0, 255);

        if (!SDL_RenderClear(welcome_renderer))
        {
            SDL_Log("Could not clear welcome renderer. Reason: %s\n", SDL_GetError());
        }

        while (SDL_PollEvent(&event))
        {
            switch(event.type){
                case SDL_EVENT_QUIT:
                    quitting = true;
                    typing = false;

                    // Stop further text input
                    if (!SDL_StopTextInput(welcome_window))
                    {
                        SDL_Log("Could not end text input. Reason: %s\n", SDL_GetError());
                    }
                    break;

                case SDL_EVENT_TEXT_INPUT:
                    // User is inputting text (for the file name for ROM)
                    if (typing)
                    {
                        strcat(input_string.input, event.text.text);
                        input_string.length++;
                        has_changed = true;
                    }
                    break;
                
                case SDL_EVENT_KEY_DOWN:
                    if (typing)
                    {   
                        // User is deleting text input  
                        if ((event.key.key == SDLK_BACKSPACE) && strcmp(input_string.input, "") != 0)  
                        {
                            input_string.input[input_string.length - 1] = '\0';
                            input_string.length--;
                            has_changed = true;
                        }
                    }
                    break;

                case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                    float x_click;
                    float y_click;

                    if (!SDL_RenderCoordinatesFromWindow(welcome_renderer, event.button.x, event.button.y, &x_click, &y_click))
                    {
                        SDL_Log("Could not get x and y coordinates of click. Reason: %s\n", SDL_GetError());
                    }

                    // Handle buttons for quirks 
                    for (int i = 0; i < NUMBER_OF_QUIRKS; i++)
                    {
                        // Check which button is being pressed
                        if (fabsf(((float)buttons[i].x - x_click)) < (BUTTON_WIDTH / 2) && fabsf((float)buttons[i].y - y_click) < (BUTTON_HEIGHT / 2))
                        {
                            // Invert buttons[i] boolean value
                            buttons[i].is_on = !buttons[i].is_on;
                            buttons[i].just_changed = true;
                        }
                    }

                    // Handle buttons for game selection
                    for (int i = 0; i < NUMBER_OF_GAMES; i++)
                    {
                        if (fabsf(((float)games[i].x - x_click)) < (menu_box_width / 2) && fabsf((float)games[i].y - y_click) < (menu_box_height / 2))
                        {
                            // Set rendered text to game title
                            strcpy(input_string.input, games[i].file_name);
                            input_string.length = strlen(input_string.input);
                            has_changed = true;
                        }
                    }

                    // Handle continue button
                    if (fabsf(((float)continue_x - x_click)) < (continue_box_width / 2) && fabsf((float)continue_y - y_click) < (continue_box_height / 2))
                    {
                        quitting = true;
                    }

                    break;
                }
            }
        }
        
        // User quits the welcome screen
        if (quitting)
        {

            SDL_DestroyWindow(welcome_window);
            SDL_DestroyRenderer(welcome_renderer);
            SDL_DestroyTexture(welcome_texture);
            SDL_DestroySurface(welcome_surface);
            TTF_CloseFont(font);
            TTF_Quit();

            break;
        }

        SDL_SetRenderDrawColor(welcome_renderer, 255, 255, 255, 255);

        // Render menu for game selection
        for (int i = 0; i < NUMBER_OF_GAMES; i++)
        {
            // Render text
            if (!SDL_RenderTexture(welcome_renderer, games[i].texture, NULL, &games[i].text_rect))
            {
                SDL_Log("Could not render texture for game selection menu. Reason: %s\n", SDL_GetError());
            }

            // Render box outline
            SDL_RenderRect(welcome_renderer, &games[i].box_rect);
        }

        // Render buttons for quirks
        for (int i = 0; i < NUMBER_OF_QUIRKS; i++, column_number++)
        {
            SDL_Color button_color;

            // Choose colour for button (green = on, red = off)
            if (buttons[i].is_on == true)
            {
                button_color = on_color;
            }
            else
            {
                button_color = off_color;
            }
            
            // Render rectangle
            SDL_SetRenderDrawColor(welcome_renderer, button_color.r, button_color.g, button_color.b, button_color.a);
            SDL_RenderFillRect(welcome_renderer, &buttons[i].button_rect);

            
            if (!SDL_RenderTexture(welcome_renderer, buttons[i].texture, NULL, &buttons[i].title_rect))
            {
                SDL_Log("Could not render texture for buttons. Reason: %s\n", SDL_GetError());
            }
        }

        if (has_changed == true || is_first_loop == true)
        {
            SDL_DestroySurface(welcome_surface_input);
            SDL_DestroyTexture(welcome_texture_input);
            
            // Set renderer to white
            SDL_SetRenderDrawColor(welcome_renderer, 255, 255, 255, 255); 

            // Initialising surface for input
            if (input_string.length == 0)
            {
                welcome_surface_input = TTF_RenderText_Blended(text_font, initial_text_input, strlen(initial_text_input), font_color);   
            }
            else
            {
                welcome_surface_input = TTF_RenderText_Blended(text_font, input_string.input, input_string.length, font_color);   
            }

            if (!welcome_surface_input)
            {
                SDL_Log("Could not initialise surface. Reason: %s\n", SDL_GetError());
            }

            // Initialising texture for input
            welcome_texture_input = SDL_CreateTextureFromSurface(welcome_renderer, welcome_surface_input);
            if (!welcome_texture_input)
            {
                SDL_Log("Could not initialise texture. Reason: %s\n", SDL_GetError());
            }
        }

        // Render border for text box
        SDL_SetRenderDrawColor(welcome_renderer, 255, 255, 255, 255);
        SDL_RenderRect(welcome_renderer, &box_rect);

        // Render border for continue button
        SDL_RenderRect(welcome_renderer, &continue_box_rect);

        // Render texture for input text
        if (!SDL_RenderTexture(welcome_renderer, welcome_texture_input, NULL, &input_rect))
        {
            SDL_Log("Could not render input text. Reason: %s\n", SDL_GetError());
        }

        // Render texture for title text
        if (!SDL_RenderTexture(welcome_renderer, welcome_texture, NULL, &title_rect))
        {
            SDL_Log("Could not render title text (texture) on welcome screen. Reason: %s\n", SDL_GetError());
        }

        // Render texture for continue button texr
        if (!SDL_RenderTexture(welcome_renderer, continue_texture, NULL, &continue_text_rect))
        {
            SDL_Log("Could not render continue button text. Reason: %s\n", SDL_GetError());
        }
        
        // Render present 
        if (!SDL_RenderPresent(welcome_renderer))
        {
            SDL_Log("Could not render present on welcome screen. Reason: %s\n", SDL_GetError());
        }
        
        // Reset quirks array
        for (int i = 0; i < NUMBER_OF_QUIRKS; i++)
        {
            buttons[i].just_changed = false;
        }

        // Reset has_changed (refers to the user's filename input)
        has_changed = false;
    }

    char *file_name = malloc((input_string.length + 1) * sizeof(char));

    if (file_name != NULL)
    {
        file_name = strcpy(file_name, input_string.input);
    }
    else
    {
        SDL_Log("Could not allocate memory for file name.");
    }

    return file_name;
}
