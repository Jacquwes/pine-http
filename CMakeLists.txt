cmake_minimum_required(VERSION 3.15)

project(pine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(loguru CONFIG REQUIRED)

if (WIN32)
  add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

add_library(pine STATIC)

target_sources(pine
  PUBLIC
    "include/pine/connection.h"
    "include/pine/coroutine.h"
    "include/pine/error.h"
    "include/pine/expected.h"
    "include/pine/http.h"
    "include/pine/http_request.h"
    "include/pine/http_response.h"
    "include/pine/iocp.h"
    "include/pine/pine_socket.h"
    "include/pine/route_node.h" 
    "include/pine/route_tree.h"
    "include/pine/route_path.h"
    "include/pine/server.h"
    "include/pine/server_connection.h"
    "include/pine/thread_pool.h"

  PRIVATE
    "src/error.cpp"
    "src/http.cpp"
    "src/http_request.cpp"
    "src/http_response.cpp"
    "src/iocp.cpp"
    "src/pine_socket.cpp"
    "src/route_node.cpp"
    "src/route_tree.cpp" 
    "src/server.cpp"
    "src/thread_pool.cpp"
)

target_include_directories(pine PUBLIC "include/pine")

target_link_libraries(pine PUBLIC loguru::loguru)
if (WIN32)
  target_link_libraries(pine PUBLIC ws2_32)
endif()

if (ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()

if (ENABLE_TESTS)
    add_subdirectory(tests)
endif()