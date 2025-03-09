# test/config/CMakeLists.txt

add_executable(
    config_test
    ConfigTest.cpp
)
target_link_libraries(
    config_test
    config
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(config_test)