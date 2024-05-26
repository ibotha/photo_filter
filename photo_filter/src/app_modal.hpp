#pragma once

#include <SDL.h>
#include <future>

#include "image_window.hpp"

struct AppModel {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Surface* in_surface;
    SDL_Surface* out_surface;
    std::future<char*> opened_file_path_future = {};
    ImageWindowModel in_image_window = {};
    ImageWindowModel out_image_window = {};
    SDL_bool app_quit = SDL_FALSE;
};