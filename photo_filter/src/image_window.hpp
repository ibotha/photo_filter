#pragma once

#include <SDL.h>

struct ImageWindowModel {
    SDL_Texture* texture;
    float width, height;
    float zoom = 1.f;
    bool fit = true;
};

void render_image_window(const char* name, ImageWindowModel* model);
