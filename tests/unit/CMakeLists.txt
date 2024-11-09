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
target_link_libraries(unit_tests PRIVATE server)
target_link_libraries(unit_tests PRIVATE doctest::doctest)

include(doctest)
DOCTEST_DISCOVER_TESTS(unit_tests)