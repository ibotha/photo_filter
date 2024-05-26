// === Dear ImGUI ===
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"

// === SDL ===
#include <SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include "nfd.h"

// === stdlib ===
#include <future>
#include <thread>
#include <iostream>
#include <chrono>
#include <cmath>

#include "app_modal.hpp"
#include "file_dialog.hpp"

const char* APP_NAME = "Photo Filter";

int SDL_Fail(const char* message = "An error occured") {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "%s: %s", message, SDL_GetError());
    return -1;
}

void load_new_image(const char* path, AppModel* app) {
    SDL_Texture* in_texture = NULL, *out_texture = NULL;
    SDL_Surface* in_surface = NULL, *out_surface = NULL;

    try {
        auto raw_surface = IMG_Load(path);
        if (!raw_surface) {
            SDL_Fail("Error loading image");
            throw std::exception();
        }
        in_surface = SDL_ConvertSurfaceFormat(raw_surface, (SDL_PixelFormatEnum)SDL_GetWindowPixelFormat(app->window));
        if (!in_surface) {
            SDL_Fail("Error converting image");
            throw std::exception();
        }
        SDL_Log("image loaded: %i %i", in_surface->w, in_surface->h);

        out_surface = SDL_DuplicateSurface(in_surface);
        in_texture = SDL_CreateTextureFromSurface(app->renderer, in_surface);
        out_texture = SDL_CreateTextureFromSurface(app->renderer, out_surface);

        if (!out_surface || !in_texture || !out_texture) {
            SDL_Fail("Error loading image");
            throw std::exception();
        }
        SDL_DestroySurface(app->in_surface);
        SDL_DestroySurface(app->out_surface);
        SDL_DestroyTexture(app->in_image_window.texture);
        SDL_DestroyTexture(app->out_image_window.texture);

        app->in_surface = in_surface;
        app->out_surface = out_surface;
        app->in_image_window.texture = in_texture;
        app->in_image_window.width = in_surface->w;
        app->in_image_window.height = in_surface->h;
        app->out_image_window.texture = out_texture;
        app->out_image_window.width = in_surface->w;
        app->out_image_window.height = in_surface->h;
    }
    catch (std::exception &e) {
        SDL_DestroySurface(in_surface);
        SDL_DestroySurface(out_surface);
        SDL_DestroyTexture(in_texture);
        SDL_DestroyTexture(out_texture);
    }

}

int SDL_AppInit(void** appstate, int argc, char* argv[]) {
    // init the library, here we make a window so we only need the Video capabilities.
    if (SDL_Init(SDL_INIT_VIDEO)) {
        return SDL_Fail("Error on initialisation");
    }

    // create a window
    SDL_Window* window = SDL_CreateWindow(APP_NAME, 1024, 640, SDL_WINDOW_RESIZABLE);
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

    SDL_SetRenderVSync(renderer, 1);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // set up the application data
    *appstate = new AppModel{
       window,
       renderer,
       NULL,
       NULL,
    };

    SDL_Log("Application started successfully!");

    return 0;
}

int SDL_AppEvent(void* appstate, const SDL_Event* event) {
    auto* app = (AppModel*)appstate;

    if (event->type == SDL_EVENT_QUIT) {
        app->app_quit = SDL_TRUE;
    }

    ImGui_ImplSDL3_ProcessEvent(event);

    return 0;
}



struct Pixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

void invert_filter(SDL_Surface* source, SDL_Surface* dest) {
    for (int y = 0; y < source->h; y++) {
        for (int x = 0; x < source->w; x++) {
            Uint32* const source_pixel = (Uint32*)((Uint8*)source->pixels
                + y * source->pitch
                + x * source->format->bytes_per_pixel);
            Uint32* const dest_pixel = (Uint32*)((Uint8*)dest->pixels
                + y * source->pitch
                + x * source->format->bytes_per_pixel);
            Uint8 r, g, b, a;
            SDL_GetRGBA(*source_pixel, source->format, &r, &g, &b, &a);
            r = 255 - r;
            g = 255 - g;
            b = 255 - b;
            a = a;
            *dest_pixel = SDL_MapRGBA(dest->format, r, g, b, a);
        }
    }
}



int SDL_AppIterate(void* appstate) {
    auto* app = (AppModel*)appstate;
    auto path = check_dialog_future(app->opened_file_path_future);
    if (path != std::nullopt) {
        load_new_image(path.value().c_str(), app);
    }

    SDL_SetRenderDrawColor(app->renderer, 0x33, 0x30, 0x2c, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(app->renderer);

    // === ImGUI logic ===
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();


    ImGui::DockSpaceOverViewport();
    // === Setup a basic window to cover the screen ==
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_RouteGlobal)) {
            app->opened_file_path_future = std::async(open_image_file_dialog);
        }
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal)) {
            SDL_SaveBMP(app->out_surface, "out.bmp");
        }

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open..", "Ctrl O")) {
                app->opened_file_path_future = std::async(open_image_file_dialog);
            }
            if (ImGui::MenuItem("Save", "Ctrl S")) {
                SDL_SaveBMP(app->out_surface, "out.bmp");
            }
            ImGui::EndMenu();
        }
        if (app->in_surface && ImGui::BeginMenu("Filter"))
        {
            if (ImGui::MenuItem("invert")) {
                invert_filter(app->in_surface, app->out_surface);
                SDL_UpdateTexture(app->out_image_window.texture, NULL, app->out_surface->pixels, app->out_surface->pitch);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // Source Image
    render_image_window("Source", &app->in_image_window);
    render_image_window("Dest", &app->out_image_window);

    // === ImGUI finalisation ===
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), app->renderer);

    SDL_RenderPresent(app->renderer);

    return app->app_quit;
}

void SDL_AppQuit(void* appstate) {
    auto* app = (AppModel*)appstate;

    // Cleanup
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    if (app) {
        SDL_DestroyTexture(app->in_image_window.texture);
        SDL_DestroyTexture(app->out_image_window.texture);
        SDL_DestroySurface(app->in_surface);
        SDL_DestroySurface(app->out_surface);
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        delete app;
    }

    SDL_Quit();
    SDL_Log("Application quit successfully!");
}