include_directories()
include(GoogleTest)

add_executable(unit_tests)

target_sources(unit_tests
  PRIVATE
    "unit_tests.cpp"
    "http_request_test.cpp"
    "http_utils_test.cpp"
)

target_link_libraries(unit_tests PRIVATE shared)
target_link_libraries(unit_tests PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

gtest_discover_tests(unit_tests)