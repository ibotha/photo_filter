#pragma once

#include <SDL3/SDL.h>
#include <nfd.h>

#include <optional>
#include <string>
#include <future>

// Blocking call that opens a file dialog with an image filter, intended to be used in a future.
nfdchar_t* open_image_file_dialog();

// See if there is a result waiting from the dialog future.
std::optional<std::string> check_dialog_future(std::future<nfdchar_t*>& future);
