project(shared)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(shared STATIC)

target_sources(shared
  PRIVATE
    "include/connection.h"
    "include/coroutine.h"
    "include/error.h"
    "include/expected.h"
    "include/http.h"
    "include/http_request.h"
    "include/http_response.h"
    "include/iocp.h"
    
    
    "include/wsa.h"

    "src/connection.cpp"
    "src/error.cpp"
    "src/http.cpp"
    "src/http_request.cpp"
    "src/http_response.cpp"
    "src/iocp.cpp"
    
    
    "src/wsa.cpp"
)

target_include_directories(shared PUBLIC include)

target_link_libraries(shared PUBLIC loguru::loguru)
if (WIN32)
  target_link_libraries(shared PUBLIC ws2_32)
endif()