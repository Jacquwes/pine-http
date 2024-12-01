#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <pine/WinSock2.h>
#include <pine/WS2tcpip.h>
#include <pine/MSWSock.h>
#include <pine/Windows.h>
#include <array>
#include <bit>
#include <coroutine>
#include <cstring>
#include <functional>
#include <iostream>
#include <source_location>
#include <thread>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

namespace pine
{
  /// @brief Enumeration of operations that can be posted to the IOCP.
  enum class iocp_operation
  {
    accept,
    read,
    write
  };

  /// @brief Structure that holds the data for an operation.
  struct iocp_operation_data
  {
    /// @brief The overlapped structure.
    OVERLAPPED overlapped;
    /// @brief The operation to perform.
    iocp_operation operation;
    /// @brief The socket.
    SOCKET socket;
    /// @brief The WSABUF structure.
    WSABUF wsa_buffer;
    /// @brief The buffer to store the data.
    std::array<char, 1024> buffer;
    /// @brief The buffer to store the address.
    std::array<char, (sizeof(sockaddr_in) + 16) * 2> accept_buffer;
    /// @brief The number of bytes transferred.
    DWORD bytes_transferred;
    /// @brief The flags.
    DWORD flags;
  };

  /// @brief This class represents an IOCP.
  class iocp_context
  {
  public:
    struct thread_data
    {
      HANDLE iocp;
      SOCKET socket;
      iocp_context* iocp_context;
    };

    /// @brief Default constructor.
    iocp_context();

    /// @brief Destructor.
    ~iocp_context();

    /// @brief Associates a socket with the IOCP.
    /// @param socket The socket to associate.
    /// @return True if the socket was associated successfully, false otherwise.
    bool associate(SOCKET socket);

    /// @brief Posts an operation to the IOCP. The operation will be performed
    /// asynchronously by Windows.
    /// @param operation The operation to post.
    /// @param socket The socket to post the operation to.
    /// @param wsa_buffer The WSABUF structure.
    /// @param flags The flags.
    /// @return True if the operation was posted successfully, false otherwise.
    bool post(iocp_operation operation, SOCKET socket, WSABUF wsa_buffer, DWORD flags = 0);

    /// @brief Closes the IOCP.
    /// @return True if the IOCP was closed successfully, false otherwise.
    bool close();

    inline void set_on_accept(const std::function<void(const iocp_operation_data*)>& on_accept)
    {
      on_accept_ = on_accept;
    }

    inline void set_on_read(const std::function<void(const iocp_operation_data*)>& on_read)
    {
      on_read_ = on_read;
    }

    inline void set_on_write(const std::function<void(const iocp_operation_data*)>& on_write)
    {
      on_write_ = on_write;
    }

    inline void init(SOCKET socket)
    {
      setup_thread_pool(socket);
      init_accept_ex(socket);
    }


  private:
    /// @brief The IOCP handle.
    HANDLE iocp_;

    thread_data data;

    std::function<void(const iocp_operation_data*)> on_accept_;
    std::function<void(const iocp_operation_data*)> on_read_;
    std::function<void(const iocp_operation_data*)> on_write_;

    std::vector<std::thread> threads_;

    using LPFN_ACCEPTEX = BOOL(PASCAL*)(SOCKET, SOCKET, PVOID, DWORD, DWORD, DWORD, LPDWORD, LPOVERLAPPED);
    LPFN_ACCEPTEX accept_ex = nullptr;

    static DWORD WINAPI worker_thread(LPVOID lpParam);

    void setup_thread_pool(SOCKET socket);
    bool init_accept_ex(SOCKET socket);

    bool post_accept(SOCKET socket, WSABUF wsa_buffer, DWORD flags);
    bool post_read(SOCKET socket, WSABUF wsa_buffer, DWORD flags);
    bool post_write(SOCKET socket, WSABUF wsa_buffer, DWORD flags);
  };
}

#endif
