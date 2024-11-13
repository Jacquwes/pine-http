project(server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(server STATIC)

target_sources(server
  PRIVATE
    "src/route_node.cpp"
    "src/route_tree.cpp" 
    "src/server.cpp"
    "src/server_connection.cpp"

  PUBLIC
    "include/route_node.h" 
    "include/route_tree.h"
    "include/route_path.h"
    "include/server.h"
    "include/server_connection.h"
)

target_include_directories(server PUBLIC include)

target_link_libraries(server PRIVATE shared)
target_link_libraries(server PRIVATE loguru::loguru)
