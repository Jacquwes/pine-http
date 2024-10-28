project(server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(server STATIC)

target_sources(server
  PRIVATE
    "src/server.cpp"
    "src/server_connection.cpp"
    "src/route.cpp" 

  PUBLIC
    "include/server.h"
    "include/server_connection.h"
    "include/route.h")

target_include_directories(server PUBLIC include)

target_link_libraries(server PRIVATE shared)
