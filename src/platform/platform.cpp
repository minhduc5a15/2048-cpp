#include "platform.h"

#include <cstdlib>

namespace tfe::platform {

    std::filesystem::path get_user_data_directory() {
#ifdef _WIN32
        const char* appdata = std::getenv("APPDATA");
        if (appdata != nullptr) {
            return std::filesystem::path(appdata);
        }
#else
        // Works for Linux and should for macOS as well
        const char* home = std::getenv("HOME");
        if (home != nullptr) {
            return std::filesystem::path(home) / ".local" / "share";
        }
#endif
        // Fallback to current directory if no standard path is found
        return {};
    }

} // namespace tfe::platform
