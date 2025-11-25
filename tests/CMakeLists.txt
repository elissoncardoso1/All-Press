cmake_minimum_required(VERSION 3.20)

# Find GTest
find_package(GTest REQUIRED)

# Test executable
add_executable(all_press_tests
    test_printer_manager.cpp
    test_job_queue.cpp
    test_file_processor.cpp
    test_rest_api.cpp
)

target_include_directories(all_press_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(all_press_tests
    GTest::gtest
    GTest::gtest_main
    Boost::boost
    nlohmann_json::nlohmann_json
    ${SQLITE3_LIBRARY}
    Threads::Threads
)

# Add test
add_test(NAME AllPressTests COMMAND all_press_tests)
