project(examples)

add_executable(
	server_example
	server_example.cpp
)

target_link_libraries(server_example PRIVATE pine)
target_link_libraries(server_example PRIVATE loguru::loguru)
