#pragma once

#include <cstdint>
#include <loguru.hpp>
#include <memory>
#include <mutex>
#include <pine/context.h>
#include <pine/error.h>
#include <pine/io_processor.h>
#include <pine/socket.h>
#include <string_view>
#include <vector>


namespace pine
{
  /// @brief A connection base class that is used by the server and the client.
  class connection
  {
    static constexpr size_t buffer_size = 64 * 1024;

  public:
    explicit connection(socket&& socket)
      : socket_{ std::move(socket) }
    {}

    /// @brief Destroy the connection.
    virtual ~connection()
    {
      close();
    }

    /// @brief Close the connection.
    virtual void close()
    {
      if (bool expected = false;
          is_closed.compare_exchange_strong(expected, true))
        return;

      socket_.close();
    }

    /// @brief Get the socket of the connection. 
    /// @return The socket of the connection.
    constexpr socket_t get_socket() const
    {
      return socket_.get();
    }

    /// @brief This function is called when a message is received.
    /// @param message The message that was received.
    virtual void on_read(std::string_view message) = 0;

    /// @brief This function is called when a message is sent.
    virtual void on_write() = 0;

    /// @brief Handle a read operation.
    /// @param data The data of the operation.
    void on_read_raw(const read_context& data)
    {
      read_pending = false;

      if (is_closed)
        return;

      size_t message_size = data.size;
      // Client closed the connection.
      if (message_size == 0)
      {
        close();
        return;
      }

      {
        std::lock_guard lock{ buffer_mutex };

        message_size_ += message_size;
        if (message_size_ >= buffer_size)
        {
          LOG_F(WARNING,
                "Connection %zu tried to send a message that was too large",
                get_socket());
          close();
          return;
        }

        post_read();

        on_read(std::string_view{ read_buffer_.data(), message_size_ });
      }

      message_size_ = 0;
    }

    /// @brief Handle a write operation.
    /// @param data The data of the operation.
    void on_write_raw(const write_context& ctx)
    {
      write_pending = false;

      if (is_closed)
        return;

      if (ctx.size == 0)
      {
        LOG_F(WARNING, "Connection %zu failed to write message", get_socket());
        close();
        return;
      }

      on_write();
    }

    /// @brief Post a read operation to the thread pool.
    void post_read()
    {
      std::lock_guard lock{ read_mutex };

      if (is_closed || read_pending)
        return;

      auto ctx = std::make_unique<read_context>();
      ctx->connection = this;
      ctx->buffer = read_buffer_.data();
      ctx->size = buffer_size;

      if (auto result = io_processor::instance().post_read(std::move(ctx));
          !result)
      {
        LOG_F(WARNING, result.error().message().data());
        close();
      }

      read_pending = true;
    }

    /// @brief Post a write operation to the thread pool.
    /// @param raw_message The message to write.
    void post_write(std::string_view raw_message)
    {
      std::lock_guard lock{ write_mutex };

      if (is_closed || write_pending || raw_message.size() == 0)
        return;

      write_buffer_.fill(0);
      std::ranges::copy(raw_message, write_buffer_.begin());

      auto ctx = std::make_unique<write_context>();
      ctx->connection = this;
      ctx->buffer = write_buffer_.data();
      ctx->size = raw_message.size();

      if (auto result = io_processor::instance().post_write(std::move(ctx));
          !result)
      {
        LOG_F(WARNING, result.error().message().data());
        close();
      }

      write_pending = true;
    }

  protected:
    std::atomic_bool write_pending = false;
    std::atomic_bool read_pending = false;
    std::atomic_bool is_closed = false;

    std::mutex buffer_mutex;
    std::mutex read_mutex;
    std::mutex write_mutex;
  private:
    /// @brief The socket of the connection.
    socket socket_;

    std::array<char, buffer_size> read_buffer_;
    std::array<char, buffer_size> write_buffer_;
    size_t message_size_ = 0;
  };
}