#pragma once

#include <system_error>

struct addrinfo;

namespace pine
{
  /// @brief Initialize the Windows Socket API.
  /// @param ec An error code to be set if an error occurs.
  void initialize_wsa(std::error_code& ec);

  /// @brief Cleanup the Windows Socket API.
  void cleanup_wsa();

  /// @brief Get address information for the specified node and service.
  /// @param node The node name or IP address. nullptr for the local host.
  /// @param service The service name or port number.
  /// @param hints Additional hints for the address resolution.
  /// @param ec An error code to be set if an error occurs.
  /// @return A pointer to the addrinfo structure containing the address information.
  addrinfo* get_address_info(const char* node,
                             const char* service,
                             std::error_code& ec);

  /// @brief Create a socket with the specified address information.
  /// @param address_info The address information for the socket.
  /// @param ec An error code to be set if an error occurs.
  /// @return The created socket.
  SOCKET create_socket(const addrinfo* address_info, std::error_code& ec);

  /// @brief Bind the socket to the specified address.
  /// @param socket The socket to bind.
  /// @param address_info The address information for the socket.
  /// @param ec An error code to be set if an error occurs.
  void bind_socket(SOCKET socket, const addrinfo* address_info, std::error_code& ec);

  /// @brief Listen for incoming connections on the socket.
  /// @param socket The socket to listen on.
  /// @param backlog The maximum length of the queue of pending connections.
  void listen_socket(SOCKET socket, int backlog, std::error_code& ec);

  /// @brief Accept an incoming connection on the socket.
  /// @param socket The socket to accept the connection on.
  /// @param address_info The address information for the socket.
  /// @param ec An error code to be set if an error occurs.
  /// @return The accepted socket.
  SOCKET accept_socket(SOCKET socket, const addrinfo* address_info, std::error_code& ec);

  /// @brief Connect the socket to the specified address.
  /// @param socket The socket to connect.
  /// @param address_info The address information for the socket.
  /// @param ec An error code to be set if an error occurs.
  void connect_socket(SOCKET socket, const addrinfo* address_info, std::system_error& ec);

  /// @brief Close the socket.
  /// @param socket The socket to close.
  void close_socket(SOCKET socket);
}
