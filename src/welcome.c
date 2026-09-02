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
    check_window(welcome_window, "Could not initialise welcome window.");

    SDL_Renderer* welcome_renderer = SDL_CreateRenderer(welcome_window, NULL);
    check_renderer(welcome_renderer, "Could not initialise welcome renderer.");
    
    // Allow renderer to adjust to window size (adjusted by scale factor)
    SDL_SetRenderLogicalPresentation(welcome_renderer, WIDTH, HEIGHT, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

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

    // Initialising font for title
    const float title_font_size = 120; // Higher font size so text is less pixelated
    TTF_Font *font = TTF_OpenFont("bold_font.ttf", title_font_size);
    check_font(font, "Could not initialise font for title.");

    // Initialising font
    const float text_font_size = 200;
    TTF_Font *text_font = TTF_OpenFont("jetbrains_mono.ttf", text_font_size);
    check_font(text_font, "Could not initialise font.");

    // Initialising surface for title
    char *welcome_text = "CHIP-8";
    SDL_Surface *welcome_surface = TTF_RenderText_Blended(font, welcome_text, strlen(welcome_text), font_color);
    check_surface(welcome_surface, "Could not initialise welcome surface.");

    // Initialsing texture for title
    SDL_Texture *welcome_texture = SDL_CreateTextureFromSurface(welcome_renderer, welcome_surface);
    check_texture(welcome_texture, "Could not initialise welcome texture.");

    // Get rectangles for rendering title
    const SDL_FRect title_rect = get_frect(32, 3, 20, 10, true);

    // Initialise continue button
    char *continue_button_text = "CONTINUE";

    SDL_Surface *continue_surface = TTF_RenderText_Blended(font, continue_button_text, strlen(continue_button_text), font_color);
    check_surface(continue_surface, "Could not create surface for rendering the continue button/text.");

    SDL_Texture *continue_texture = SDL_CreateTextureFromSurface(welcome_renderer, continue_surface);
    check_texture(continue_texture, "Could not create texture for rendering the continue button/text.");

    // Variables for drawing buttons
    const int x_start = 12;
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
        buttons[i].surface = TTF_RenderText_Blended(text_font, quirks[i].quirk_string, strlen(quirks[i].quirk_string), font_color);
        check_surface(buttons[i].surface, "Button title surface is NULL.");

        buttons[i].texture = SDL_CreateTextureFromSurface(welcome_renderer, buttons[i].surface);
        check_texture(buttons[i].texture, "Button texture is NULL.");
    }

    // Initialise popular games menu
    Game games[NUMBER_OF_GAMES];

    games[PONG].name = "PONG";
    games[PONG].file_name = "pong.ch8";

    games[TETRIS].name = "TETRIS";
    games[TETRIS].file_name = "tetris.ch8";

    games[SPACE_INVADERS].name = "SPACE INVADERS";
    games[SPACE_INVADERS].file_name = "space_invaders.ch8";

    // Coordinate settings for rendering game selection menu
    const int menu_x_start = 8;
    const int menu_y_start = 3;
    const int menu_width = 10;
    const int menu_height = 3;
    const int menu_y_buffer = 1;

    const int menu_box_width = menu_width + 4;
    const int menu_box_height = y_buffer + 1;

    // Initialise games
    for (int i = 0; i < NUMBER_OF_GAMES; i++)
    {
        SDL_Surface *curr_surface = TTF_RenderText_Blended(font, games[i].name, strlen(games[i].name), font_color);
        check_surface(curr_surface, "Could not create surface for games menu.");

        SDL_Texture *curr_texture = SDL_CreateTextureFromSurface(welcome_renderer, curr_surface);
        check_texture(curr_texture, "Could not create texture for games menu.");

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

    // Initialise variables for continue button
    int continue_x = 54;
    int continue_y = 4;
    int continue_box_width = 14;
    int continue_box_height = 6;

    // Get rect for rendering continue button
    const SDL_FRect continue_box_rect = get_frect(continue_x, continue_y, continue_box_width, continue_box_height, true);
    const SDL_FRect continue_text_rect = get_frect(continue_x, continue_y, 10, 4, true);

    // Initialise button for opening ROM file
    char *file_button_str = "OPEN ROM FILE";
    SDL_Surface *file_surface = TTF_RenderText_Blended(font, file_button_str, strlen(file_button_str), font_color);
    check_surface(file_surface, "Could not create surface for games menu.");

    SDL_Texture *file_texture = SDL_CreateTextureFromSurface(welcome_renderer, file_surface);
    check_texture(file_texture, "Could not create texture for games menu.");

    // Get rectangles for rendering button to open file
    const int file_box_width = 18;
    const int file_box_height = 5;

    const int file_x = 32;
    const int file_y = 10;
    const SDL_FRect file_box_rect = get_frect(file_x, file_y, file_box_width, file_box_height, true);
    const SDL_FRect file_text_rect = get_frect(file_x, file_y, file_box_width - 4, file_box_height - 2, true);

    // Initialise button for opening ROM file
    char *initial_file_name = "NO FILE SELECTED";
    SDL_Surface *name_surface = TTF_RenderText_Blended(font, initial_file_name, strlen(initial_file_name), font_color);
    check_surface(name_surface, "Could not create surface for displaying file name.");

    SDL_Texture *name_texture = SDL_CreateTextureFromSurface(welcome_renderer, name_surface);
    check_texture(name_texture, "Could not create texture for displaying file name.");

    // Get rectangles for displaying file name (underneath continue button)
    const int name_box_width = 18;
    const int name_box_height = 5;

    const int name_x = 54;
    const int name_y = 8;
    const SDL_FRect name_text_rect = get_frect(name_x, name_y, name_box_width - 4, name_box_height - 2, true);

    // Initialise variables for tracking file
    char *file_name;
    char *user_file_name;
    char *display_name;
    File file;
    file.surface = name_surface;
    file.texture = name_texture;
    file.text_rect = name_text_rect;
    file.file_name = file_name;
    file.user_file_name = user_file_name;
    file.display_name = display_name;
    file.has_changed = false;

    // Variables for loop
    bool quitting = false;
    bool is_first_loop = true;
    bool can_continue = false;
    SDL_Event event;
    SDL_zero(event);

    while (!quitting)
    {
        // Reset colour to black before clearing for black background
        SDL_SetRenderDrawColor(welcome_renderer, 0, 0, 0, 255);

        if (!SDL_RenderClear(welcome_renderer))
        {
            SDL_Log("Could not clear welcome renderer. Reason: %s\n", SDL_GetError());
        }

        // Reset colour back to white
        SDL_SetRenderDrawColor(welcome_renderer, 255, 255, 255, 255);

        while (SDL_PollEvent(&event))
        {
            switch(event.type){
                case SDL_EVENT_QUIT:
                    quitting = true;

                case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                    float x_click;
                    float y_click;

                    if (!SDL_RenderCoordinatesFromWindow(welcome_renderer, event.button.x, event.button.y, &x_click, &y_click))
                    {
                        SDL_Log("Could not get x and y coordinates of click. Reason: %s\n", SDL_GetError());
                    }

                    // Quirk has been selected
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

                    // Game has been selected
                    for (int i = 0; i < NUMBER_OF_GAMES; i++)
                    {
                        if (fabsf(((float)games[i].x - x_click)) < (menu_box_width / 2) && fabsf((float)games[i].y - y_click) < (menu_box_height / 2))
                        {
                            // No need to check this as the built-in games have been tested
                            file.file_name = games[i].file_name;
                            file.display_name = games[i].name;
                            file.has_changed = true;
                            can_continue = true;
                        }
                    }

                    // Continue button clicked
                    if (can_continue && fabsf(((float)continue_x - x_click)) < (continue_box_width / 2) && fabsf((float)continue_y - y_click) < (continue_box_height / 2))
                    {
                        quitting = true;
                    }

                    // Open file button clicked
                    if (fabsf(((float)file_x - x_click)) < (file_box_width / 2) && fabsf((float)file_y - y_click) < (file_box_height / 2))
                    {
                        // Open up file selection window
                        SDL_ShowOpenFileDialog(callback, &file, welcome_window, NULL, 0, NULL, false);
                    }

                    break;
                }
            }
        }
        
        // User quits the welcome screen
        if (quitting)
        {
            // Destroy surfaces and textures for quirks
            for (int i = 0; i < NUMBER_OF_QUIRKS; i++)
            {
                    SDL_DestroySurface(buttons[i].surface);
                    SDL_DestroyTexture(buttons[i].texture);
            }

            // Destroy surfaces and textures for games
            for (int i = 0; i < NUMBER_OF_GAMES; i++)
            {
                SDL_DestroySurface(games[i].surface);
                SDL_DestroyTexture(games[i].texture);
            }

            SDL_DestroySurface(file_surface);
            SDL_DestroyTexture(file.texture);
            SDL_DestroyWindow(welcome_window);
            SDL_DestroyRenderer(welcome_renderer);
            SDL_DestroyTexture(welcome_texture);
            SDL_DestroySurface(welcome_surface);
            TTF_CloseFont(font);
            TTF_CloseFont(text_font);
            TTF_Quit();

            break;
        }

        // Update file name text
        if (file.has_changed == true)
        {
            if (file.user_input_changed == true)
            {
                // Check file user uploaded is not NULL and has valid size
                FILE *ROM_file = fopen(file.user_file_name, "r");
                if (ROM_file != NULL)
                {
                    // Determine size of file (in bytes)
                    fseek(ROM_file, 0, SEEK_END);
                    long size = ftell(ROM_file);

                    // CHIP-8 program starts at 0x200 - ensure file doesn't exceed memory capacity
                    if (size > (4096 - 0x200))
                    {
                        can_continue = false;
                    }
                    else
                    {
                        file.file_name = file.user_file_name;
                        can_continue = true;

                        // Update display_name
                        file.display_name = strrchr(file.file_name, '/');
                        if (file.display_name != NULL)
                        {
                            // Get rid of the slash at the beginning
                            file.display_name += 1;
                        }
                        else
                        {
                            SDL_Log("strrchr failed for updating display name for file.");
                        }
                    }
                }
                else
                {
                    can_continue = false;
                } 
                fclose(ROM_file);
            }
            
            // Initialise variables for updating text underneath continue button (displays file name)
            SDL_DestroyTexture(file.texture);
            SDL_DestroySurface(file.surface);
            SDL_Surface *new_name_surface;
            SDL_Texture *new_name_texture;
            char *error_message = "ERROR";

            if (can_continue == true)
            {
                new_name_surface = TTF_RenderText_Blended(text_font, file.display_name, strlen(file.display_name), font_color);
            }
            else
            {
                new_name_surface = TTF_RenderText_Blended(text_font, error_message, strlen(error_message), font_color);
            }
            
            // Update surfaces and texture for new file name
            new_name_texture = SDL_CreateTextureFromSurface(welcome_renderer, new_name_surface);
            
            check_surface(new_name_surface, "Could not render new surface for displaying file name.");
            check_texture(new_name_texture, "Could not render new texture for displayinf file name.");
            file.surface = new_name_surface;
            file.texture = new_name_texture;
        }

        // Render open file button
        if (!SDL_RenderTexture(welcome_renderer, file_texture, NULL, &file_text_rect))
        {
            SDL_Log("Could not render texture for open file button. Reason: %s\n", SDL_GetError());
        }
        SDL_RenderRect(welcome_renderer, &file_box_rect);

        // Render menu for game selection
        for (int i = 0; i < NUMBER_OF_GAMES; i++)
        {
            // Render text (game name)
            if (!SDL_RenderTexture(welcome_renderer, games[i].texture, NULL, &games[i].text_rect))
            {
                SDL_Log("Could not render texture for game selection menu. Reason: %s\n", SDL_GetError());
            }

            // Render box outline
            SDL_RenderRect(welcome_renderer, &games[i].box_rect);
        }

        // Render texture for title text
        if (!SDL_RenderTexture(welcome_renderer, welcome_texture, NULL, &title_rect))
        {
            SDL_Log("Could not render title text (texture) on welcome screen. Reason: %s\n", SDL_GetError());
        }

        // Render texture for continue button text
        if (!SDL_RenderTexture(welcome_renderer, continue_texture, NULL, &continue_text_rect))
        {
            SDL_Log("Could not render continue button text. Reason: %s\n", SDL_GetError());
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

            // Rendering texture for button labels 
            if (!SDL_RenderTexture(welcome_renderer, buttons[i].texture, NULL, &buttons[i].title_rect))
            {
                SDL_Log("Could not render texture for buttons. Reason: %s\n", SDL_GetError());
            }
        }

        // Display file name
        if (!SDL_RenderTexture(welcome_renderer, file.texture, NULL, &file.text_rect))
        {
            SDL_Log("Could not render texture for displaying file name. Reason: %s\n", SDL_GetError());
        }

        // Render border for continue button
        if (can_continue == true)
        {
            SDL_SetRenderDrawColor(welcome_renderer, 0, 255, 0, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(welcome_renderer, 255, 0, 0, 255);
        }
        SDL_RenderRect(welcome_renderer, &continue_box_rect);
    
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

        // Reset has_changed and user_input_changed for tracking the file name
        file.has_changed = false;
        file.user_input_changed = false;
    }

    char *final_file_name = malloc((strlen(file.file_name) + 1) * sizeof(char));

    if (final_file_name != NULL)
    {
        strcpy(final_file_name, file.file_name);
    }
    else
    {
        SDL_Log("Could not allocate memory for file name.\n");
    }

    return final_file_name;
}
