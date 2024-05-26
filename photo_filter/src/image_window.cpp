#include "image_window.hpp"
#include <imgui.h>

/*
Calculates the size of bounds of an image such that it fits perfectly within a viewport.
*/

constexpr ImVec2 operator*(ImVec2 lhs, const float& rhs)
{
    lhs.x *= rhs;
    lhs.y *= rhs;
    return lhs;
}


constexpr ImVec2& operator*=(ImVec2& lhs, const float& rhs)
{
    lhs.x *= rhs;
    lhs.y *= rhs;
    return lhs;
}

constexpr ImVec2 operator*(ImVec2 lhs, const ImVec2& rhs)
{
    lhs.x *= rhs.x;
    lhs.y *= rhs.y;
    return lhs;
}

constexpr ImVec2 operator+(ImVec2 lhs, const ImVec2& rhs)
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
}

constexpr ImVec2 operator-(ImVec2 lhs, const ImVec2& rhs)
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    return lhs;
}


float fit_zoom(ImVec2 bounds, ImVec2 image_size) {
    float bounds_ar = bounds.x / bounds.y;
    float image_ar = image_size.x / image_size.y;
    if (image_ar > bounds_ar) {
        return bounds.x / image_size.x;
    }
    return bounds.y / image_size.y;
}

void render_image_window(const char* name, ImageWindowModel* model)
{
    ImGui::Begin(name, NULL, ImGuiWindowFlags_MenuBar |
        (model->fit ? 0 : ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar)
    );
    if (ImGui::BeginMenuBar()) {
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_KeypadMultiply)) {
            model->fit = true;
        }
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_KeypadAdd)) {
            model->zoom = model->zoom * 1.1f;
            model->fit = false;
        }
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_KeypadSubtract)) {
            model->zoom = model->zoom / 1.1f;
            model->fit = false;
        }
        if (ImGui::BeginMenu("View")) {
            
            if (ImGui::MenuItem("Toggle Fit", "Ctrl *")) {
                model->fit = true;
            }

            if (ImGui::MenuItem("Zoom In", "Ctrl +")) {
                model->zoom = model->zoom * 1.1f;
                model->fit = false;
            }

            if (ImGui::MenuItem("Zoom Out", "Ctrl -")) {
                model->zoom = model->zoom / 1.1f;
                model->fit = false;
            }

            if (ImGui::MenuItem("100% Zoom")) {
                model->zoom = 1;
                model->fit = false;
            }

            ImGui::EndMenu();
        }
        if (model->texture) {
            ImGui::Text("Zoom %f%%", model->zoom * 100.f);
        }
        ImGui::EndMenuBar();
    }
    if (model->texture) {
        auto available = ImGui::GetContentRegionAvail();
        
        auto image_size = ImVec2{ model->width, model->height };
        if (model->fit) {
            model->zoom = fit_zoom(available, image_size);
        }
        image_size *= model->zoom;
        if (model->fit) {
            ImGui::SetCursorPos(ImGui::GetWindowContentRegionMin() + (available - image_size) * 0.5f);
        }
        ImGui::Image(model->texture, image_size);
    }
        
    ImGui::End();
}
