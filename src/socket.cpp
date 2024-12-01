#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <ws2def.h>
#include <WS2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include <loguru.hpp>
#include <pine/error.h>
#include <pine/expected.h>
#include <pine/socket.h>
#include <string>

namespace pine
{
  static constexpr int max_connections = SOMAXCONN;

  std::expected<socket, pine::error> socket::accept() const
  {
    socket client;
    client.fd_ = ::accept(fd_, nullptr, nullptr);
    if (client.fd_ < 0)
    {
      LOG_F(ERROR, "Failed to accept socket: %d", errno);
      return std::make_unexpected(error(error_code::socket_error,
                                        std::to_string(errno)));
    }
    LOG_F(1, "Socket accepted: %d", client.fd_);

    // set linger option
    struct linger so_linger;
    so_linger.l_onoff = 1;
    so_linger.l_linger = 0;
    if (const auto& opt_result = client.set_option(SOL_SOCKET,
                                                   SO_LINGER,
                                                   so_linger);
        !opt_result)
      return std::make_unexpected(opt_result.error());

    LOG_F(1, "Socket option set: %d", client.fd_);

    return client;
  }

  std::expected<void, pine::error> socket::bind(uint16_t port) const
  {
    sockaddr_in address{};
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (::bind(fd_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0)
    {
      LOG_F(ERROR, "Failed to bind socket: %d", errno);
      return std::make_unexpected(error(error_code::socket_error,
                                        std::to_string(errno)));
    }
    LOG_F(1, "Socket bound: %d", fd_);
    return {};
  }

  void socket::close()
  {
    if (fd_ == invalid_socket)
      return;

    auto socket = fd_;
    fd_ = invalid_socket;

#ifdef _WIN32
    closesocket(socket);
#else
    ::close(socket);
#endif

    LOG_F(1, "Socket closed: %d", socket);
  }

  std::expected<socket, pine::error> socket::create(uint16_t port)
  {
    socket s;

    s.fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s.fd_ < 0)
    {
      LOG_F(ERROR, "Failed to create socket: %d", errno);
      return std::make_unexpected(error(error_code::socket_error,
                                        std::to_string(errno)));
    }

    LOG_F(1, "Socket created: %d", s.fd_);

    if (const auto& opt_result = s.set_option(SOL_SOCKET,
                                              SO_REUSEADDR | SO_REUSEPORT,
                                              1);
        !opt_result)
      return std::make_unexpected(opt_result.error());

    LOG_F(1, "Socket option set: %d", s.fd_);

    if (const auto& bind_result = s.bind(port);
        !bind_result)
      return std::make_unexpected(bind_result.error());

    LOG_F(1, "Socket bound: %d", s.fd_);

    return s;
  }

  std::expected<void, pine::error> socket::listen(int backlog) const
  {
    if (::listen(fd_, backlog) < 0)
    {
      LOG_F(ERROR, "Failed to listen on socket: %d", errno);
      return std::make_unexpected(error(error_code::socket_error,
                                        std::to_string(errno)));
    }

    LOG_F(1, "Listen socket listening");

    return {};
  }

#ifdef _WIN32
  wsa_guard::wsa_guard()
  {
    if (const auto& init_result = initialize_wsa();
        !init_result)
      throw std::runtime_error(init_result.error().message());
  }

  wsa_guard::~wsa_guard()
  {
    cleanup_wsa();
  }

  void cleanup_wsa()
  {
    WSACleanup();
    LOG_F(1, "WSA cleaned up");
  }

  std::expected<void, pine::error> initialize_wsa()
  {
    WSADATA wsa_data = { 0 };
    if (int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        result != 0)
    {
      LOG_F(ERROR, "Failed to init WSA: %d", result);
      return std::make_unexpected(error(error_code::winsock_error,
                                        std::to_string(result)));
    }

    LOG_F(1, "WSA initialized: %s", wsa_data.szDescription);

    return {};
  }
#endif
}
