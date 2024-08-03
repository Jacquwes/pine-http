#include <array>
#include <cstdint>
#include <exception>
#include <iostream>
#include <system_error>
#include <vector>
#include "connection.h"
#include "coroutine.h"
#include "snowflake.h"

#ifdef _WIN32
#include <WinSock2.h>
#endif // _WIN32

namespace pine
{
  connection::connection(SOCKET socket, snowflake id)
    : id(id), socket(socket)
  {
    std::cout << "[Connection] New connection: " << id << std::endl;
  }

  async_operation<std::string> connection::receive_raw_message(
    std::error_code& ec)
  {
    static constexpr size_t chunk_size = 1024;
    std::array<char, chunk_size> buffer{};
    std::string message;

    while (true)
    {
      size_t bytes_received = recv(this->socket, buffer.data(), buffer.size(), 0);

      if (bytes_received == 0)
      {
        ec = std::make_error_code(std::errc::connection_reset);
        co_return "";
      }

      if (bytes_received == SOCKET_ERROR)
      {
        ec = std::make_error_code(static_cast<std::errc>(WSAGetLastError()));
        co_return "";
      }

      message.append(buffer.data(), bytes_received);

      if (bytes_received < buffer.size())
        break;
    }

    co_return message;
  }


  async_task connection::send_raw_message(std::string_view buffer,
                                          std::error_code& ec)
  {
    if (buffer.empty())
      co_return;

    asio::error_code ec;
    socket.send(asio::buffer(buffer), {}, ec);

    if (ec && ec != asio::error::connection_reset && ec != asio::error::connection_aborted)
    {
      std::cout << "[Connection] Failed to send message: " << std::dec << ec.value() << " -> " << ec.message() << std::endl;
      co_return;
    }

    co_return;
  }

  void connection::close()
  {
    socket.close();
  }
}