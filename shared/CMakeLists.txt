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
    "include/snowflake.h"
    "include/thread_pool.h"
    "include/wsa.h"

    "src/connection.cpp"
    "src/http.cpp"
    "src/http_request.cpp"
    "src/http_response.cpp"
    "src/snowflake.cpp"
    "src/thread_pool.cpp"
    "src/wsa.cpp"
)

target_include_directories(shared PUBLIC include)

if (WIN32)
  target_link_libraries(shared PUBLIC ws2_32)
endif()