#pragma once

#include <WinSock2.h>
#include <coroutine.h>
#include <cstdint>
#include <error.h>
#include <expected.h>
#include <filesystem>
#include <functional>
#include <http.h>
#include <http_request.h>
#include <http_response.h>
#include <initializer_list>
#include <iocp.h>
#include <memory>
#include <mutex>
#include <route_node.h>
#include <route_path.h>
#include <route_tree.h>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>
#include <ws2def.h>

namespace pine
{
  class iocp_operation_data;

  /// @brief A server that accepts connections from clients.
  class server
  {
    friend class server_connection;
    friend class iocp_context;

  public:
    /// @brief Construct a server with the given asio context and port.
    explicit server(const char* port = "80");

    /// @brief Start listening for connections.
    std::expected<void, pine::error> start();

    /// @brief Stop listening for connections.
    void stop();

    /// @brief Disconnect a client.
    /// @param client_id Id of the client to disconnect.
    /// @return An asynchronous task completed when the client has been
    /// disconnected.
    async_operation<void> remove_client(uint64_t const& client_id);

    /// @brief Add a route to the server.
    /// @param path The HTTP path to match in order to call the handler.
    /// @param methods The HTTP methods to match in order to call the handler.
    /// @param handler The function to call when the route is requested.
    /// The second parameter of the handler represents the response to send
    /// to the client.
    /// @return A reference to the created route.
    route_node&
      add_route(route_path path,
                const std::function<void(const http_request&,
                                         http_response&)>& handler,
                const std::initializer_list<pine::http_method>& methods
                = { http_method::get });

    /// @brief Add a static route to the server. The route will serve files from
    /// the specified directory, or the specified file.
    /// @param path The path to match in order to serve files from the location.
    /// @param location The location to serve files from.
    /// @return 
    route_node& add_static_route(route_path path,
                                 std::filesystem::path&& location);

    /// @brief Get a route by path and method.
    /// @return If the route was found, a shared pointer to the route.
    /// If the route was not found, an error code.
    const route_node&
      get_route(std::string_view path) const;

  private:
    /// @brief Accept clients.
    /// This function waits for clients to connect and creates a server
    /// connection for each client.
    /// @return An asynchronous task completed when the server has stopped
    /// listening.
    std::expected<void, pine::error> accept_clients();

    iocp_context iocp_;

    std::mutex delete_clients_mutex;
    std::mutex mutate_clients_mutex;

    std::unordered_map<uint64_t, std::shared_ptr<server_connection>> clients;
    route_tree routes;

    const char* port;
  #ifdef _WIN32
    SOCKET server_socket = INVALID_SOCKET;
    addrinfo* address_info = nullptr;
  #else
    int server_socket = -1;
  #endif

    bool is_listening = false;
    std::jthread acceptor_thread;
    std::jthread delete_clients_thread;

    /// @brief Handle an accept operation.
    void on_accept(const iocp_operation_data*);

    /// @brief Handle a read operation.
    void on_read(const iocp_operation_data*);

    /// @brief Handle a write operation.
    void on_write(const iocp_operation_data*);
  };
}