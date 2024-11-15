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
    DWORD bytes_transferred = data->bytes_transferred;

    if (bytes_transferred == 0)
    {
      LOG_F(1, "Connection %zu ended by peer", get_socket());
      close();
      return;
    }

    message_size_ += bytes_transferred;

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
    LOG_F(INFO, "Connection %zu wrote message", get_socket());

    on_write();
  }

  void connection::post_read()
  {
    message_buffer_.resize(message_size_ + 1024);
    WSABUF wsa_buffer{};
    wsa_buffer.buf = message_buffer_.data() + message_size_;
    wsa_buffer.len = 1024;
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
    if (socket_ != INVALID_SOCKET)
    {
      shutdown(socket_, SD_BOTH);
      closesocket(socket_);
      LOG_F(INFO, "Connection %zu closed", get_socket());
      socket_ = INVALID_SOCKET;
    }
  }
}