#pragma once

#include <any>
#include <cstdint>
#include <pine/error.h>
#include <pine/expected.h>

namespace pine
{
#ifdef _WIN32
  /// @brief File descriptor representing a socket.
  using socket_t = unsigned long long;

  /// @brief Guard for the Windows Socket API.
  struct wsa_guard
  {
    /// @brief Initialize the Windows Socket API.
    wsa_guard();

    /// @brief Cleanup the Windows Socket API.
    ~wsa_guard();
  };

  /// @brief Initialize the Windows Socket API.
  std::expected<void, pine::error> initialize_wsa();

  /// @brief Cleanup the Windows Socket API.
  void cleanup_wsa();
#else
  /// @brief File descriptor representing a socket.
  using socket_t = int;
#endif

  constexpr int invalid_socket = -1;

  /// @brief Socket wrapper functions.
  class socket
  {
  public:
    socket()
      : fd_{ invalid_socket }
    {}
    socket(const socket&) = delete;
    socket(socket&& other) noexcept
      : fd_{ other.fd_ }
    {
      other.fd_ = invalid_socket;
    }
    socket& operator=(const socket&) = delete;
    socket& operator=(socket&& other) noexcept
    {
      if (this != &other)
      {
        fd_ = other.fd_;
        other.fd_ = invalid_socket;
      }
      return *this;
    }

    /// @brief Accept an incoming connection on the socket.
    /// @return The accepted socket.
    std::expected<socket, pine::error> accept() const;

    /// @brief Close the socket if it is open.
    void close();

    /// @brief Create a new socket.
    /// @param port The port to bind the socket to.
    static std::expected<socket, pine::error> create(uint16_t port);

    /// @brief Get the socket file descriptor.
    /// @return The socket file descriptor.
    constexpr socket_t get() const noexcept
    {
      return fd_;
    }

    /// @brief Listen for incoming connections on the socket.
    std::expected<void, pine::error> listen() const;

  private:
    /// @brief The socket file descriptor.
    socket_t fd_;

    /// @brief Bind the socket to the specified port.
    /// @param port The port to bind the socket to.
    /// @return The result of the operation.
    std::expected<void, pine::error> bind(uint16_t port) const;

    /// @brief Set a socket option.
    /// @tparam T The type of the value to set.
    /// @param level The level at which the option is defined.
    /// @param option The socket option for which the value is to be set.
    /// @param value The value to set.
    /// @param size The size of the value.
    /// @return The result of the operation.
    template <typename T>
    std::expected<void, pine::error> set_option(int level,
                                                int option,
                                                const T& value)
    {
#ifdef _WIN32
      int setsockopt(socket_t s,
                     int level,
                     int optname,
                     const char* optval,
                     int optlen);
      if (setsockopt(fd_,
                     level,
                     option,
                     reinterpret_cast<const char*>(&value),
                     sizeof(T)) == SOCKET_ERROR)
      {
        return std::make_unexpected(
          error(error_code::socket_error, std::to_string(WSAGetLastError())));
      }
#else
      int setsockopt(int sockfd,
                     int level,
                     int optname,
                     const void* optval,
                     socklen_t optlen);
      if (setsockopt(fd_,
                     level,
                     option,
                     reinterpret_cast<const void*>(&value),
                     sizeof(T)) < 0)
      {
        return std::make_unexpected(
          error(error_code::socket_error, std::to_string(errno)));
      }
#endif
      return {};
    }
  };
}
