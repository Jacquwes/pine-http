#ifdef _WIN32
#include <WinSock2.h>
#endif // _WIN32
#include <array>
#include <connection.h>
#include <coroutine.h>
#include <error.h>
#include <iostream>
#include <loguru.hpp>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace pine
{
  connection::connection(SOCKET socket, iocp_context& context)
    : socket_(socket),
    context_(context)
  {
    LOG_F(1, "Connection created: %zu", socket_);
  }

  connection::~connection()
  {
    LOG_F(1, "Connection destroyed: %zu", socket_);
  }

  void connection::on_read_raw(const iocp_operation_data* data)
  {
    message_size_ += data->wsa_buffer.len;

    if (message_size_ == 1024)
    {
      LOG_F(INFO, "Connection %zu received partial message", get_socket());
      post_read();
    }
    else
    {
      std::string_view message{ message_buffer_.data(),
                                message_buffer_.size() };

      on_read(message);

      LOG_F(INFO, "Connection %zu received message of size %zu", get_socket(), message.size());

      message_buffer_.clear();
      message_size_ = 0;
    }
  }

  void connection::on_write_raw(const iocp_operation_data*)
  {
    LOG_F(INFO, "Connection %d wrote message", get_socket());

    on_write();
  }

  void connection::post_read()
  {
    message_buffer_.resize(message_size_ + 1024);
    WSABUF wsa_buffer{};
    wsa_buffer.buf = message_buffer_.data();
    wsa_buffer.len = static_cast<ULONG>(message_buffer_.size());
    if (!context_.post(iocp_operation::read, socket_, wsa_buffer, 0))
    {
      LOG_F(WARNING, "Failed to post read operation: %d", WSAGetLastError());
    }
  }

  void connection::post_write(std::string_view raw_message) const
  {
    if (raw_message.empty())
      return;

    WSABUF wsa_buffer{};
    wsa_buffer.buf = const_cast<char*>(raw_message.data());
    wsa_buffer.len = static_cast<ULONG>(raw_message.size());

    context_.post(iocp_operation::write, socket_, wsa_buffer, 0);
  }

  void connection::close()
  {
    shutdown(this->socket_, SD_BOTH);
    closesocket(this->socket_);

    this->socket_ = INVALID_SOCKET;

    LOG_F(INFO, "Connection %d closed", get_socket());
  }
}