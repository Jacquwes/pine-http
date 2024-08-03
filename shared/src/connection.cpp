#include <cstdint>
#include <exception>
#include <iostream>
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
    std::vector<uint8_t> buffer(bufferSize, 0);

    if (!bufferSize)
      co_return buffer;

    asio::error_code ec;
    auto flags = asio::socket_base::message_peek;
    size_t n = socket.receive(asio::buffer(buffer), flags, ec);

    if (ec && ec != asio::error::connection_reset && ec != asio::error::connection_aborted)
    {
      std::cout << "[Connection] Failed to receive message: " << std::dec << ec.value() << " -> " << ec.message() << std::endl;
      co_return buffer;
    }

    buffer.resize(n);

    co_return buffer;
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