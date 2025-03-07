# src/config/CMakeLists.txt

# Enable fetching content from internet
include(FetchContent)

# Include json lib If it is available locally then use that instead
FetchContent_Declare(
    json
    GIT_REPOSITORY "https://github.com/nlohmann/json"
    GIT_TAG "v3.11.3"
    FIND_PACKAGE_ARGS NAMES json)

FetchContent_MakeAvailable(json)

add_library(config STATIC
    ConfigManager.cpp
    ConfigManager.hpp
    configs.hpp)

target_include_directories(config PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(config PRIVATE nlohmann_json::nlohmann_json)