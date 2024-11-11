#define WIN32_LEAN_AND_MEAN

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#include <expected.h>
#include <string>
#include <ws2def.h>
#include "error.h"
#include "wsa.h"

namespace pine
{
  std::expected<void, pine::error> initialize_wsa()
  {
    WSADATA wsa_data = { 0 };
    if (int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        result != 0)
    {
      return std::make_unexpected(error(error_code::winsock_error,
                                        std::to_string(result)));
    }

    return {};
  }

  void cleanup_wsa()
  {
    WSACleanup();
  }

  std::expected<addrinfo*, pine::error>
    get_address_info(const char* node, const char* service)
  {
    addrinfo* result = nullptr;
    addrinfo hints = { 0 };
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (int error = getaddrinfo(node, service, &hints, &result);
        error != 0)
    {
      return std::make_unexpected(pine::error(error_code::getaddrinfo_error,
                                              std::to_string(error)));
    }

    return result;
  }

  std::expected<SOCKET, pine::error>
    create_socket(const addrinfo* address_info)
  {
    SOCKET socket = WSASocket(address_info->ai_family,
                               address_info->ai_socktype,
                               address_info->ai_protocol,
                               nullptr,
                               0,
                               WSA_FLAG_OVERLAPPED);

    if (socket == INVALID_SOCKET)
    {
      return std::make_unexpected(error(error_code::winsock_error,
                                        std::to_string(WSAGetLastError())));

    }

    return socket;
  }

  std::expected<void, pine::error>
    bind_socket(SOCKET socket, const addrinfo* address_info)
  {
    if (bind(socket,
             address_info->ai_addr,
             static_cast<int>(address_info->ai_addrlen))
        == SOCKET_ERROR)
    {
      return std::make_unexpected(error(error_code::winsock_error,
                                        std::to_string(WSAGetLastError())));
    }

    return {};
  }

  std::expected<void, pine::error>
    listen_socket(SOCKET socket, int backlog)
  {
    if (listen(socket, backlog) == SOCKET_ERROR)
    {
      return std::make_unexpected(error(error_code::winsock_error,
                                        std::to_string(WSAGetLastError())));
    }

    return {};
  }

  std::expected<SOCKET, pine::error>
    accept_socket(SOCKET socket, const addrinfo* address_info)
  {
    SOCKET accepted_socket = accept(socket, address_info->ai_addr, nullptr);

    if (accepted_socket == INVALID_SOCKET)
    {
      return std::make_unexpected(error(error_code::winsock_error,
                                        std::to_string(WSAGetLastError())));
    }

    return accepted_socket;
  }

  std::expected<void, pine::error>
    connect_socket(SOCKET socket, const addrinfo* address_info)
  {
    if (connect(socket,
                address_info->ai_addr,
                static_cast<int>(address_info->ai_addrlen))
        == SOCKET_ERROR)
    {
      return std::make_unexpected(error(error_code::winsock_error,
                                        std::to_string(WSAGetLastError())));
    }

    return {};
  }

  void close_socket(SOCKET socket)
  {
    closesocket(socket);
  }
}