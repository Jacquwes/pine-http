#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <system_error>
#include <WS2tcpip.h>
#include "wsa.h"

namespace pine
{
  void initialize_wsa(std::error_code& ec)
  {
    WSADATA wsa_data = { 0 };
    if (int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        result != 0)
    {
      ec = std::make_error_code(static_cast<std::errc>(result));
    }

    ec = std::error_code{};
  }

  void cleanup_wsa()
  {
    WSACleanup();
  }

  addrinfo* get_address_info(const char* node,
                             const char* service,
                             std::error_code& ec)
  {
    addrinfo* result = nullptr;
    addrinfo hints = { 0 };
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (int error = getaddrinfo(node, service, &hints, &result);
        error != 0)
    {
      ec = std::make_error_code(static_cast<std::errc>(error));
    }

    return result;
  }

  SOCKET create_socket(const addrinfo* address_info, std::error_code& ec)
  {
    SOCKET socket = ::socket(address_info->ai_family,
                             address_info->ai_socktype,
                             address_info->ai_protocol);

    if (socket == INVALID_SOCKET)
    {
      ec = std::make_error_code(static_cast<std::errc>(WSAGetLastError()));
    }

    return socket;
  }

  void bind_socket(SOCKET socket, const addrinfo* address_info, std::error_code& ec)
  {
    if (bind(socket, address_info->ai_addr, static_cast<int>(address_info->ai_addrlen)) == SOCKET_ERROR)
    {
      ec = std::make_error_code(static_cast<std::errc>(WSAGetLastError()));
    }
  }

  void listen_socket(SOCKET socket, int backlog, std::error_code& ec)
  {
    if (listen(socket, backlog) == SOCKET_ERROR)
    {
      ec = std::make_error_code(static_cast<std::errc>(WSAGetLastError()));
    }
  }

  SOCKET accept_socket(SOCKET socket, const addrinfo* address_info, std::error_code& ec)
  {
    SOCKET accepted_socket = accept(socket, address_info->ai_addr, nullptr);

    if (accepted_socket == INVALID_SOCKET)
    {
      ec = std::make_error_code(static_cast<std::errc>(WSAGetLastError()));
    }

    return accepted_socket;
  }

  void connect_socket(SOCKET socket, const addrinfo* address_info, std::system_error& ec)
  {
    if (connect(socket, address_info->ai_addr, static_cast<int>(address_info->ai_addrlen)) == SOCKET_ERROR)
    {
      ec = std::system_error(WSAGetLastError(), std::system_category());
    }
  }

  void close_socket(SOCKET socket)
  {
    closesocket(socket);
  }
}