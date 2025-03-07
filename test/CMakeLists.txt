
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest/
  GIT_TAG "v1.16.0"
  FIND_PACKAGE_ARGS NAMES googletest GTest GoogleTest
)

FetchContent_MakeAvailable(googletest)

# Add test subdirectories
add_subdirectory(config)
add_subdirectory(packet)