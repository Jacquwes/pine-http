#define WIN32_LEAN_AND_MEAN

#include <expected.h>
#include <system_error>
#include <Windows.h>
#include <WinSock2.h>
#include <ws2def.h>
#include <WS2tcpip.h>
#include "wsa.h"

namespace pine
{
  std::expected<void, std::error_code> initialize_wsa()
  {
    WSADATA wsa_data = { 0 };
    if (int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        result != 0)
    {
      return std::make_unexpected(
        std::make_error_code(static_cast<std::errc>(result)));
    }

    return {};
  }

  void cleanup_wsa()
  {
    WSACleanup();
  }

  std::expected<addrinfo*, std::error_code>
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
      return std::make_unexpected(
        std::make_error_code(static_cast<std::errc>(error)));
    }

    return result;
  }

  std::expected<SOCKET, std::error_code>
    create_socket(const addrinfo* address_info)
  {
    SOCKET socket = ::socket(address_info->ai_family,
                             address_info->ai_socktype,
                             address_info->ai_protocol);

    if (socket == INVALID_SOCKET)
    {
      return std::make_unexpected(
        std::make_error_code(static_cast<std::errc>(WSAGetLastError())));
    }

    return socket;
  }

  std::expected<void, std::error_code>
    bind_socket(SOCKET socket, const addrinfo* address_info)
  {
    if (bind(socket,
        address_info->ai_addr,
        static_cast<int>(address_info->ai_addrlen))
        == SOCKET_ERROR)
    {
      return std::make_unexpected(
        std::make_error_code(static_cast<std::errc>(WSAGetLastError())));
    }

    return {};
  }

  std::expected<void, std::error_code>
    listen_socket(SOCKET socket, int backlog)
  {
    if (listen(socket, backlog) == SOCKET_ERROR)
    {
      return std::make_unexpected(
        std::make_error_code(static_cast<std::errc>(WSAGetLastError())));
    }

    return {};
  }

  std::expected<SOCKET, std::error_code>
    accept_socket(SOCKET socket, const addrinfo* address_info)
  {
    SOCKET accepted_socket = accept(socket, address_info->ai_addr, nullptr);

    if (accepted_socket == INVALID_SOCKET)
    {
      return std::make_unexpected(
        std::make_error_code(static_cast<std::errc>(WSAGetLastError())));
    }

    return accepted_socket;
  }

  std::expected<void, std::system_error>
    connect_socket(SOCKET socket, const addrinfo* address_info)
  {
    if (connect(socket,
        address_info->ai_addr,
        static_cast<int>(address_info->ai_addrlen))
        == SOCKET_ERROR)
    {
      return std::make_unexpected(
        std::system_error(WSAGetLastError(), std::system_category()));
    }

    return {};
  }

  void close_socket(SOCKET socket)
  {
    closesocket(socket);
  }
}