project(examples)

add_executable(
	server_example
	server_example.cpp
)

target_link_libraries(server_example PRIVATE shared)
target_link_libraries(server_example PRIVATE server)

add_executable(
	client_example
	client_example.cpp
)

target_link_libraries(client_example PRIVATE shared)
target_link_libraries(client_example PRIVATE client)
