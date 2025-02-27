# src/CMakeLists.txt

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

add_executable(lunar-network-daemon
    ConfigManager.cpp
)

target_link_libraries(lunar-network-daemon PRIVATE nlohmann_json::nlohmann_json)
