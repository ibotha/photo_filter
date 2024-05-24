#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include <iostream>
#include <cmath>

struct AppContext {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Surface* input_surface;
    SDL_Texture* input_texture;
    SDL_Texture* output_texture;
    SDL_FRect window_dimentions;
    SDL_bool app_quit = SDL_FALSE;
};

int SDL_Fail(const char* message = "An error occured") {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "%s: %s", message, SDL_GetError());
    return -1;
}

int load_new_image(const char* path, AppContext* context) {
    auto in_texture = context->input_texture;
    auto out_texture = context->output_texture;
    auto surface = context->input_surface;
    context->input_surface = IMG_Load(path);
    if (!context->input_surface) {
        context->input_surface = surface;
        return SDL_Fail("Error loading image");
    }
    SDL_Log("image loaded: %i %i", context->input_surface->w, context->input_surface->h);
    // === Make a test texture for now to show it on one part of the screen
    context->input_texture = SDL_CreateTextureFromSurface(context->renderer, context->input_surface);
    context->output_texture = SDL_CreateTextureFromSurface(context->renderer, context->input_surface);

    SDL_DestroySurface(surface);
    SDL_DestroyTexture(in_texture);
    SDL_DestroyTexture(out_texture);

    return 0;
}

int SDL_AppInit(void** appstate, int argc, char* argv[]) {
    // init the library, here we make a window so we only need the Video capabilities.
    if (SDL_Init(SDL_INIT_VIDEO)) {
        return SDL_Fail("Error on initialisation");
    }

    // create a window
    SDL_Window* window = SDL_CreateWindow("Window", 1024, 640, SDL_WINDOW_RESIZABLE);
    if (!window) {
        return SDL_Fail("Error creating window");
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        return SDL_Fail("Error creating renderer");
    }
    
    SDL_FRect window_dimentions{};
    // print some information about the window
    SDL_ShowWindow(window);
    {
        int width, height, bbwidth, bbheight;
        SDL_GetWindowSize(window, &width, &height);
        window_dimentions.w = width;
        window_dimentions.h = height;
        SDL_GetWindowSizeInPixels(window, &bbwidth, &bbheight);
        SDL_Log("Window size: %ix%i", width, height);
        SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);
        if (width != bbwidth) {
            SDL_Log("This is a highdpi environment.");
        }
    }

    // set up the application data
    *appstate = new AppContext{
       window,
       renderer,
       NULL,
       NULL,
       NULL,
       window_dimentions,
    };

    // Load the image from disk
    if (argc < 2) {
        SDL_Log("Please provide a path to an image.");
        return -1;
    }
    SDL_Log("Loading image: %s", argv[1]);
    if (load_new_image(argv[1], (AppContext*)*appstate) != 0) {
        return -1;
    }

    SDL_Log("Application started successfully!");

    return 0;
}

int SDL_AppEvent(void* appstate, const SDL_Event* event) {
    auto* app = (AppContext*)appstate;

    if (event->type == SDL_EVENT_WINDOW_RESIZED) {
        int width, height;
        SDL_GetWindowSize(app->window, &width, &height);
        app->window_dimentions.w = width;
        app->window_dimentions.h = height;
    }

    if (event->type == SDL_EVENT_QUIT) {
        app->app_quit = SDL_TRUE;
    }

    return 0;
}

/*
Calculates the size of the image out rect such that it fits on the left half of the screen,
taking as much space as it can.
*/ 
SDL_FRect image_rect(AppContext* app) {
    if (!app->input_surface)
        return SDL_FRect{};
    float margin = 20;
    float max_width = app->window_dimentions.w / 2.f - margin * 2.f;
    float max_height = app->window_dimentions.h - margin * 2.f;
    float width_scale = max_width / app->input_surface->w;
    float height_scale = max_height / app->input_surface->h;
    float scale = width_scale > height_scale ? height_scale : width_scale;
    float image_width = app->input_surface->w * scale;
    float image_height = app->input_surface->h * scale;
    const SDL_FRect input_block{
        (max_width - image_width) / 2.f + margin,(max_height - image_height) / 2.f + margin,
        image_width,image_height
    };
    return input_block;
}

int SDL_AppIterate(void* appstate) {
    auto* app = (AppContext*)appstate;

    SDL_SetRenderDrawColor(app->renderer, 0x33, 0x30, 0x2c, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(app->renderer);

    SDL_FRect out_block = image_rect(app);
    SDL_FRect border = out_block;
    border.h += 10;
    border.w += 10;
    border.x -= 5;
    border.y -= 5;

    SDL_SetRenderDrawColor(app->renderer, 0x11, 0x11, 0x11, SDL_ALPHA_OPAQUE);
    if (app->input_texture != NULL) {
        SDL_RenderFillRect(app->renderer, &border);
        SDL_RenderTexture(app->renderer, app->input_texture, NULL, &out_block);
    }
    out_block.x += app->window_dimentions.w / 2.f;
    border.x += app->window_dimentions.w / 2.f;
    if (app->output_texture != NULL) {
        SDL_RenderFillRect(app->renderer, &border);
        SDL_RenderTexture(app->renderer, app->output_texture, NULL, &out_block);
    }
    SDL_RenderPresent(app->renderer);


    return app->app_quit;
}

void SDL_AppQuit(void* appstate) {
    auto* app = (AppContext*)appstate;
    if (app) {
        SDL_DestroyTexture(app->input_texture);
        SDL_DestroyTexture(app->output_texture);
        SDL_DestroySurface(app->input_surface);
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        delete app;
    }

    SDL_Quit();
    SDL_Log("Application quit successfully!");
}