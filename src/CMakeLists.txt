# src/CMakeLists.txt

include(FetchContent)

# Enable fetching content from internet
include(FetchContent)
# Include json lib If it is available locally then use that instead
FetchContent_Declare(
  json
  GIT_REPOSITORY "https://github.com/nlohmann/json"
  GIT_TAG "v3.11.3"
  FIND_PACKAGE_ARGS NAMES json nlohmann_json)
FetchContent_MakeAvailable(json)

add_executable(lunar-network-daemon ConfigManager.cpp)

target_link_libraries(lunar-network-daemon PRIVATE nlohmann_json::nlohmann_json)
