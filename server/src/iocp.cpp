#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <Windows.h>
#include <array>
#include <bit>
#include <cstring>
#include <server.h>
#include <iocp.h>

namespace pine
{
  DWORD WINAPI iocp_context::worker_thread(LPVOID arg)
  {
    auto args = static_cast<thread_data*>(arg);
    HANDLE iocp = args->iocp;
    SOCKET socket = args->socket;
    iocp_context* context = args->iocp_context;

    while (true)
    {
      DWORD bytes_transferred;
      ULONG_PTR completion_key;
      LPOVERLAPPED overlapped;

      bool result = GetQueuedCompletionStatus(iocp,
                                              &bytes_transferred,
                                              &completion_key,
                                              &overlapped,
                                              INFINITE);

      if (!result || !overlapped)
      {
        break;
      }

      auto data = CONTAINING_RECORD(overlapped, iocp_operation_data, overlapped);

      switch (data->operation)
      {
        using enum iocp_operation;
      case accept:
        context->server_->on_accept(data);
        context->post_accept(socket, data->wsa_buffer, data->flags);
        break;
      case read:
        context->server_->on_read(data);
        break;
      default:
        break;
      }
    }
  }

  iocp_context::iocp_context()
  {
    iocp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);

    setup_thread_pool();
  }

  iocp_context::~iocp_context()
  {
    CloseHandle(iocp_);
  }

  bool iocp_context::associate(SOCKET socket)
  {
    return CreateIoCompletionPort(std::bit_cast<HANDLE>(socket), iocp_, socket, 0) != nullptr;
  }

  bool iocp_context::post(iocp_operation operation, SOCKET socket, WSABUF wsa_buffer, DWORD flags)
  {
    switch (operation)
    {
      using enum iocp_operation;
    case accept:
      return post_accept(socket, wsa_buffer, flags);
    case read:
      return post_read(socket, wsa_buffer, flags);
    case write:
      return post_write(socket, wsa_buffer, flags);
    default:
      return false;
    }
  }

  bool iocp_context::close()
  {
    return CloseHandle(iocp_);
  }

  void iocp_context::set_server(server* server)
  {
    server_ = server;
  }

  void iocp_context::init_accept_ex(SOCKET socket)
  {

  }

  void iocp_context::setup_thread_pool()
  {
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);

    thread_data data = { iocp_, INVALID_SOCKET, this };
    for (DWORD i = 0; i < system_info.dwNumberOfProcessors; i++)
    {
      CreateThread(nullptr, 0, worker_thread, &data, 0, nullptr);
    }
  }

  bool iocp_context::post_accept(SOCKET socket, WSABUF wsa_buffer, DWORD flags)
  {
    SOCKET accept_socket = WSASocketA(AF_INET,
                                      SOCK_STREAM,
                                      IPPROTO_TCP,
                                      nullptr,
                                      0,
                                      WSA_FLAG_OVERLAPPED);
    if (accept_socket == INVALID_SOCKET)
    {
      return false;
    }

    auto data = new iocp_operation_data;
    data->socket = accept_socket;
    data->operation = iocp_operation::accept;
    data->wsa_buffer = wsa_buffer;
    data->flags = flags;
    memset(&data->overlapped, 0, sizeof(data->overlapped));

    DWORD bytes_received;
    accept_ex(socket,
              accept_socket,
              wsa_buffer.buf,
              wsa_buffer.len - (sizeof(sockaddr_in) + 16),
              sizeof(sockaddr_in) + 16,
              sizeof(sockaddr_in) + 16,
              &bytes_received,
              &data->overlapped);
  }

  bool iocp_context::post_read(SOCKET socket, WSABUF wsa_buffer, DWORD flags)
  {
    auto data = new iocp_operation_data;
    data->socket = socket;
    data->operation = iocp_operation::read;
    data->wsa_buffer = wsa_buffer;
    data->flags = flags;
    memset(&data->overlapped, 0, sizeof(data->overlapped));
    DWORD bytes_received;
    WSARecv(socket,
            &wsa_buffer,
            1,
            &bytes_received,
            &flags,
            &data->overlapped,
            nullptr);

    return true;
  }

  bool iocp_context::post_write(SOCKET socket, WSABUF wsa_buffer, DWORD flags)
  {
    auto data = new iocp_operation_data;
    data->socket = socket;
    data->operation = iocp_operation::write;
    data->wsa_buffer = wsa_buffer;
    data->flags = flags;
    memset(&data->overlapped, 0, sizeof(data->overlapped));
    DWORD bytes_sent;
    WSASend(socket,
            &wsa_buffer,
            1,
            &bytes_sent,
            flags,
            &data->overlapped,
            nullptr);
    return true;
  }
}