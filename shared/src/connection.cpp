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
    message_buffer_.reserve(1024);
  }

  connection::~connection()
  {
    this->close();
  }

  void connection::on_read_raw(const iocp_operation_data* data)
  {
    read_pending = false;

    if (is_closed)
      return;

    try
    {
      DWORD bytes_transferred = data->bytes_transferred;
      if (bytes_transferred == 0)
      {
        LOG_F(1, "Connection %zu ended by peer", get_socket());
        close();
        return;
      }

      message_size_ += bytes_transferred;

      std::string_view message{ message_buffer_.data(),
                                message_buffer_.size() };

      on_read(message);

      LOG_F(INFO, "Connection %zu received message of size %zu", get_socket(), message.size());

      message_buffer_.clear();
      message_size_ = 0;

      if (!is_closed)
        post_read();
    } catch (const std::system_error& e)
    {
      LOG_F(ERROR, "Exception thrown while trying to read message on socket %zu: %s", get_socket(), e.what());
      close();
    }
  }

  void connection::on_write_raw(const iocp_operation_data* data)
  {
    write_pending = false;

    if (data->bytes_transferred == 0)
    {
      LOG_F(WARNING, "Connection %zu failed to write message", get_socket());
      close();
      return;
    }

    LOG_F(INFO, "Connection %zu wrote message", get_socket());

    on_write();
  }

  void connection::post_read()
  {
    try
    {
      std::lock_guard lock{ operation_mutex };

      if (is_closed || read_pending)
        return;

      if (message_buffer_.size() < message_size_ + 1024)
        message_buffer_.resize(message_size_ + 1024);

      WSABUF wsa_buffer{};
      wsa_buffer.buf = message_buffer_.data() + message_size_;
      wsa_buffer.len = 1024;

      read_pending = true;
      if (!context_.post(iocp_operation::read, socket_, wsa_buffer, 0))
      {
        LOG_F(WARNING, "Failed to post read operation: %d", WSAGetLastError());
        read_pending = false;
        close();
      }
    } catch (const std::system_error& e)
    {
      LOG_F(ERROR, "Exception thrown while trying to post read operation on socket %zu: %s", get_socket(), e.what());
      close();
    }
  }

  void connection::post_write(std::string_view raw_message)
  {
    try
    {
      std::lock_guard lock{ operation_mutex };

      if (is_closed || write_pending || raw_message.empty())
        return;

      WSABUF wsa_buffer{};
      wsa_buffer.buf = const_cast<char*>(raw_message.data());
      wsa_buffer.len = static_cast<ULONG>(raw_message.size());

      write_pending = true;
      if (!context_.post(iocp_operation::write, socket_, wsa_buffer, 0))
      {
        LOG_F(WARNING, "Failed to post write operation: %d", WSAGetLastError());
        write_pending = false;
        close();
      }
    } catch (const std::system_error& e)
    {
      LOG_F(ERROR, "Exception thrown while trying to post write operation on socket %zu: %s", get_socket(), e.what());
      close();
    }
  }

  void connection::close()
  {
    std::lock_guard lock{ operation_mutex };

    if (is_closed)
      return;

    if (socket_ != INVALID_SOCKET)
    {
      CancelIoEx((HANDLE)socket_, nullptr);

      shutdown(socket_, SD_BOTH);
      closesocket(socket_);
      LOG_F(INFO, "Connection %zu closed", get_socket());

      socket_ = INVALID_SOCKET;
    }
  }
}