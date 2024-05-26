#include "./file_dialog.hpp"

nfdchar_t* open_image_file_dialog() {
    nfdchar_t* outPath = NULL;
    nfdresult_t result = NFD_OpenDialog("png,jpeg,jpg", NULL, &outPath);

    if (result == NFD_OKAY) {
        return outPath;
    }
    else if (result == NFD_CANCEL) {
    }
    else {
        SDL_Log("Error: %s", NFD_GetError());
    }

    return outPath;
}


// Query the file dialog future to see if it has a result for us;
std::optional<std::string> check_dialog_future(std::future<nfdchar_t*>& future) {
    using namespace std::chrono_literals;

    if (!future.valid())
        return std::nullopt;

    std::string out;

    std::future_status status;
    status = future.wait_for(0s);
    switch (status)
    {
    case std::future_status::deferred:
        throw std::exception("File open future was in deffered state! This should never happen.");
    case std::future_status::timeout:
        return std::nullopt;
        break;
    case std::future_status::ready:
        nfdchar_t* res = future.get();
        out = std::string(res);
        free(res);
        break;
    }
    return out;
}