#pragma once

#include <filesystem>

namespace tfe::platform {

    /**
     * @brief Gets the standard directory for storing user-specific application data.
     *
     * This function is cross-platform:
     * - On Windows, it returns the path to the %APPDATA% directory.
     * - On Linux, it returns the path to the ~/.local/share directory.
     *
     * @return A filesystem::path object representing the user data directory.
     *         Returns an empty path if the directory cannot be determined.
     */
    std::filesystem::path get_user_data_directory();

} // namespace tfe::platform
