#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <Windows.h>
#include <array>
#include <bit>
#include <cstring>
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
        context->on_accept_(data);
        context->post_accept(socket, data->wsa_buffer, data->flags);
        break;
      case read:
        context->on_read_(data);
        break;
      case write:
        context->on_write_(data);
        break;
      }

      delete data;
    }

    return 0;
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

  bool iocp_context::init_accept_ex(SOCKET socket)
  {
    GUID guid_accept_ex = WSAID_ACCEPTEX;
    DWORD bytes_received;
    return WSAIoctl(socket,
                    SIO_GET_EXTENSION_FUNCTION_POINTER,
                    &guid_accept_ex,
                    sizeof(guid_accept_ex),
                    &accept_ex,
                    sizeof(accept_ex),
                    &bytes_received,
                    nullptr,
                    nullptr) == 0;
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
    SOCKET accept_socket = WSASocket(AF_INET,
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
    memset(&data->overlapped, 0, sizeof(data->overlapped));
    data->socket = accept_socket;
    data->operation = iocp_operation::accept;

    DWORD bytes_received;
    if (int result = accept_ex(socket,
                               accept_socket,
                               data->accept_buffer.data(),
                               0,
                               sizeof(sockaddr_in) + 16,
                               sizeof(sockaddr_in) + 16,
                               &bytes_received,
                               &data->overlapped);
        result == FALSE && WSAGetLastError() != ERROR_IO_PENDING)
    {
      delete data;
      return false;
    }

    return true;
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