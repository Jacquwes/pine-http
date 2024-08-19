include_directories()
include(GoogleTest)

add_executable(unit_tests)

target_sources(unit_tests
  PRIVATE
    "http_request_test.cpp"
    "http_response_test.cpp"
    "http_utils_test.cpp"
    "thread_pool_test.cpp"
    "unit_tests.cpp"
)

target_link_libraries(unit_tests PRIVATE shared)
target_link_libraries(unit_tests PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

gtest_discover_tests(unit_tests)