#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <initializer_list>
#include <memory>
#include <pine/context.h>
#include <pine/error.h>
#include <pine/expected.h>
#include <pine/http.h>
#include <pine/http_request.h>
#include <pine/http_response.h>
#include <pine/io_processor.h>
#include <pine/socket.h>
#include <pine/route_node.h>
#include <pine/route_path.h>
#include <pine/route_tree.h>
#include <shared_mutex>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

namespace pine
{
  class server_connection;

  /// @brief A server that accepts connections from clients.
  class server
  {
    friend class server_connection;

  public:
    using callback_function = std::function<void(const http_request&, http_response&)>;

    /// @brief Construct a server with the given asio context and port.
    explicit server(uint16_t port = 80);

    /// @brief Start listening for connections.
    std::expected<void, pine::error> start();

    /// @brief Stop listening for connections.
    void stop();

    /// @brief Disconnect a client.
    /// @param client_id Id of the client to disconnect.
    /// @return An asynchronous task completed when the client has been
    /// disconnected.
    void remove_client(uint64_t const& client_id);

    /// @brief Add a route to the server.
    /// @param path The HTTP path to match in order to call the handler.
    /// @param methods The HTTP methods to match in order to call the handler.
    /// @param handler The function to call when the route is requested.
    /// The second parameter of the handler represents the response to send
    /// to the client.
    /// @return A reference to the created route.
    route_node&
      add_route(route_path path,
                const callback_function& handler,
                const std::initializer_list<pine::http_method>& methods
                = { http_method::get });

    /// @brief Add a static route to the server. The route will serve files from
    /// the specified directory, or the specified file.
    /// @param path The path to match in order to serve files from the location.
    /// @param location The location to serve files from.
    /// @return 
    route_node& add_static_route(route_path path,
                                 std::filesystem::path&& location);

    /// @brief Add an error handler to the server. The handler will be called
    /// when the server encounters a certain error.
    /// @param status The status to match.
    /// @param handler The function to call.
    void add_error_handler(http_status status, callback_function&& handler);

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

    std::shared_mutex clients_mutex_;

    std::unordered_map<uint64_t, std::shared_ptr<server_connection>> clients;
    std::unordered_map<http_status, callback_function> error_handlers;

    route_tree routes;

    uint16_t port{};
    socket server_socket{};

#ifdef _WIN32
    wsa_guard wsa{};
#endif

    bool is_listening = false;
    std::jthread acceptor_thread;
    std::jthread delete_clients_thread;

    friend class io_processor;
    /// @brief Handle an accept operation.
    void on_accept(const accept_context& ctx);
  };
}
