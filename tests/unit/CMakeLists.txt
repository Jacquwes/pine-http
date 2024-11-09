add_executable(unit_tests)

target_sources(unit_tests
  PRIVATE
    "http_request_tests.cpp"
    "http_response_tests.cpp"
    "http_tests.cpp"
    "unit_tests.cpp"
    "route_tests.cpp"
)

target_link_libraries(unit_tests PRIVATE shared)
target_link_libraries(unit_tests PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

gtest_discover_tests(unit_tests)