project(tests)

enable_testing()
find_package(doctest REQUIRED)

add_subdirectory(unit)
