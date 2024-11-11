#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <iocp.h>
#include <string_view>
#include <coroutine.h>
#include <error.h>
#include <snowflake.h>

#ifdef _WIN32
#include <WinSock2.h>
#endif // _WIN32


namespace pine
{
  /// @brief A connection base class that is used by the server and the client.
  class connection
  {
  public:
    explicit connection(SOCKET socket, iocp_context& context);

    /// @brief Destroy the connection.
    virtual ~connection() = default;

    /// @brief This function is called when a message is received.
    /// @param message The message that was received.
    virtual void on_read(std::string_view message) = 0;

    /// @brief This function is called when a message is sent.
    virtual void on_write() = 0;

    /// @brief Post a read operation to the thread pool.
    void post_read();

    /// @brief Post a write operation to the thread pool.
    /// @param raw_message The message to write.
    void post_write(std::string_view raw_message) const;

    void on_read_raw(const iocp_operation_data* data);
    void on_write_raw(const iocp_operation_data* data);
    
    /// @brief Close the connection.
    void close();

    constexpr SOCKET get_socket() const
    {
      return socket_;
    }

  private:
    /// @brief The socket of the connection.
  #ifdef _WIN32
    SOCKET socket_;
  #endif // _WIN32

    iocp_context& context_;

    /// @brief An array of characters to store the message.
    std::vector<char> message_buffer_;
    size_t message_size_ = 0;
  };
}