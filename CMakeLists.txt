cmake_minimum_required(VERSION 3.15)

project(Pine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(loguru CONFIG REQUIRED)

if (WIN32)
	add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

add_subdirectory(shared)
add_subdirectory(server)

if (ENABLE_EXAMPLES)
	add_subdirectory(examples)
endif()

if (ENABLE_TESTS)
		add_subdirectory(tests)
endif()