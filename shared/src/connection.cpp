#ifdef _WIN32
#include <WinSock2.h>
#endif // _WIN32
#include <array>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error>
#include "connection.h"
#include "coroutine.h"
#include "error.h"
#include "snowflake.h"

namespace pine
{
  connection::connection(SOCKET socket, snowflake id)
    : id(id), socket(socket)
  {
    std::cout << "[Connection] New connection: " << id << std::endl;
  }

  async_operation<std::string>
    connection::receive_raw_message() const
  {
    static constexpr size_t chunk_size = 1024;
    std::array<char, chunk_size> buffer{};
    std::string message;

    while (true)
    {
      size_t bytes_received =
        recv(this->socket, buffer.data(), buffer.size(), 0);

      if (bytes_received == 0)
      {
        co_return error(error_code::connection_closed,
                        "The connection was closed by the remote host.");
      }

      if (bytes_received == SOCKET_ERROR)
      {
        co_return error(error_code::winsock_error,
                        std::to_string(WSAGetLastError()));
      }

      message.append(buffer.data(), bytes_received);

      if (bytes_received < buffer.size())
        break;
    }

    co_return message;
  }

  async_operation<void>
    connection::send_raw_message(std::string_view raw_message) const
  {
    if (raw_message.empty())
      co_return error(error_code::success);

    size_t bytes_sent = send(this->socket,
                             raw_message.data(),
                             static_cast<int>(raw_message.size()),
                             0);

    if (bytes_sent == SOCKET_ERROR)
    {
      co_return error(error_code::winsock_error,
                      std::to_string(WSAGetLastError()));
    }

    if (bytes_sent != raw_message.size())
    {
      co_return error(error_code::winsock_error,
                      "Not all bytes were sent.");
    }

    co_return error(error_code::success);
  }

  void connection::close()
  {
    shutdown(this->socket, SD_BOTH);
    closesocket(this->socket);

    this->socket = INVALID_SOCKET;
  }
}