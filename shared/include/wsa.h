#pragma once

#include <expected.h>
#include <system_error>

struct addrinfo;
using SOCKET = unsigned long long;

namespace pine
{
  /// @brief Initialize the Windows Socket API.
  std::expected<void, pine::error> initialize_wsa();

  /// @brief Cleanup the Windows Socket API.
  void cleanup_wsa();

  /// @brief Get address information for the specified node and service.
  /// @param node The node name or IP address. nullptr for the local host.
  /// @param service The service name or port number.
  /// @return A pointer to the addrinfo structure containing the address
  /// information.
  std::expected<addrinfo*, pine::error>
    get_address_info(const char* node,
                     const char* service);

  /// @brief Create a socket with the specified address information.
  /// @param address_info The address information for the socket.
  /// @return The created socket.
  std::expected<SOCKET, pine::error>
    create_socket(const addrinfo* address_info);

  /// @brief Bind the socket to the specified address.
  /// @param socket The socket to bind.
  /// @param address_info The address information for the socket.
  std::expected<void, pine::error>
    bind_socket(SOCKET socket, const addrinfo* address_info);

  /// @brief Listen for incoming connections on the socket.
  /// @param socket The socket to listen on.
  /// @param backlog The maximum length of the queue of pending connections.
  std::expected<void, pine::error>
    listen_socket(SOCKET socket, int backlog);

  /// @brief Accept an incoming connection on the socket.
  /// @param socket The socket to accept the connection on.
  /// @param address_info The address information for the socket.
  /// @param ec An error code to be set if an error occurs.
  /// @return The accepted socket.
  std::expected<SOCKET, pine::error>
    accept_socket(SOCKET socket, const addrinfo* address_info);

  /// @brief Connect the socket to the specified address.
  /// @param socket The socket to connect.
  /// @param address_info The address information for the socket.
  /// @param ec An error code to be set if an error occurs.
  std::expected<void, pine::error>
    connect_socket(SOCKET socket, const addrinfo* address_info);

  /// @brief Close the socket.
  /// @param socket The socket to close.
  void close_socket(SOCKET socket);
}
