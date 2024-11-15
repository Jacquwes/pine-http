#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <Windows.h>
#include <array>
#include <bit>
#include <cstring>
#include <iocp.h>
#include <loguru.hpp>
#include <thread>

namespace pine
{
  DWORD WINAPI iocp_context::worker_thread(LPVOID arg)
  {
    auto args = (thread_data*)arg;
    HANDLE iocp = args->iocp;
    SOCKET socket = args->socket;
    iocp_context* context = args->iocp_context;

    while (true)
    {
      DWORD bytes_transferred;
      ULONG_PTR completion_key;
      LPOVERLAPPED overlapped;

      LOG_F(1, "Worker thread waiting for notification");

      bool result = GetQueuedCompletionStatus(iocp,
                                              &bytes_transferred,
                                              &completion_key,
                                              &overlapped,
                                              INFINITE);

      LOG_F(1, "Worker thread received a notification! Key: %d", completion_key);

      if (!result || !overlapped)
      {
        LOG_F(ERROR, "Worker thread failed to get completion status:\n"
              "\tiocp                             = %d\n"
              "\tsocket                           = %d\n"
              "\tWSAGetLastError                  = %d\n"
              "\tGetQueuedCompletionStatusresult  = %x\n"
              "\toverlapped                       = %x\n"
              "\tbytes_transferred                = %d\n"
              "\tcompletion_key                   = %d\n",
              iocp, socket, GetLastError(), result, overlapped, bytes_transferred, completion_key);
        break;
      }

      auto data = CONTAINING_RECORD(overlapped, iocp_operation_data, overlapped);
      data->bytes_transferred = bytes_transferred;
      data->overlapped = *overlapped;

      switch (data->operation)
      {
        using enum iocp_operation;
      case accept:
        LOG_F(1, "Worker thread accepted a connection");
        context->associate(data->socket);
        context->on_accept_(data);
        context->post_accept(socket, data->wsa_buffer, data->flags);
        break;
      case read:
        LOG_F(1, "Worker thread read data");
        context->on_read_(data);
        break;
      case write:
        LOG_F(1, "Worker thread wrote data");
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

    LOG_F(1, "IOCP created: %d", iocp_);
  }

  iocp_context::~iocp_context()
  {
    CloseHandle(iocp_);

    LOG_F(1, "IOCP destroyed: %d", iocp_);
  }

  bool iocp_context::associate(SOCKET socket)
  {
    if (HANDLE cp = CreateIoCompletionPort(std::bit_cast<HANDLE>(socket), iocp_, socket, 0);
        cp == nullptr)
    {
      LOG_F(ERROR, "Failed to associate socket %d with IOCP", socket);
      return false;
    }

    return true;
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
      LOG_F(WARNING, "Invalid IOCP operation");
      return false;
    }
  }

  bool iocp_context::close()
  {
    LOG_F(1, "Closing IOCP");

    return CloseHandle(iocp_);
  }

  bool iocp_context::init_accept_ex(SOCKET socket)
  {
    GUID guid_accept_ex = WSAID_ACCEPTEX;
    DWORD bytes_received;
    if (int result = WSAIoctl(socket,
                              SIO_GET_EXTENSION_FUNCTION_POINTER,
                              &guid_accept_ex,
                              sizeof(guid_accept_ex),
                              &accept_ex,
                              sizeof(accept_ex),
                              &bytes_received,
                              nullptr,
                              nullptr);
        result == 0)
    {
      LOG_F(1, "AcceptEx initialized");
      return true;
    }
    else
    {
      LOG_F(WARNING, "Failed to initialize AcceptEx");
      return false;
    }
  }

  void iocp_context::setup_thread_pool(SOCKET socket)
  {
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);

    data = { iocp_, socket, this };
    for (DWORD i = 0; i < system_info.dwNumberOfProcessors; i++)
    {
      std::thread(worker_thread, &data).detach();
      //CreateThread(nullptr, 0, worker_thread, &data, 0, nullptr);
    }

    LOG_F(1, "Thread pool created with %d threads", system_info.dwNumberOfProcessors);
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
      LOG_F(WARNING, "Failed to create accept socket");
      return false;
    }

    LOG_F(1, "Accept socket created: %d", accept_socket);

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
      LOG_F(WARNING, "Failed to post AcceptEx");
      delete data;
      return false;
    }

    LOG_F(1, "Accept posted");
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
    if (int result = WSARecv(socket,
                             &wsa_buffer,
                             1,
                             &bytes_received,
                             &flags,
                             &data->overlapped,
                             nullptr);
        result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
      LOG_F(ERROR, "Failed to post WSARecv: %d", WSAGetLastError());
      delete data;
      return false;
    }

    LOG_F(1, "Read posted");
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
    if (int result = WSASend(socket,
                             &wsa_buffer,
                             1,
                             &bytes_sent,
                             flags,
                             &data->overlapped,
                             nullptr);
        result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
      LOG_F(WARNING, "Failed to post WSASend: %d", WSAGetLastError());
      delete data;
      return false;
    }

    LOG_F(1, "Write posted");
    return true;
  }
}