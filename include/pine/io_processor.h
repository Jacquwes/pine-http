#pragma once

#include <memory>
#include <pine/context.h>
#include <pine/error.h>
#include <pine/expected.h>
#include <pine/socket.h>

namespace pine
{
  class io_processor
  {
  public:
    static io_processor& instance();

    std::expected<void, error> associate(socket_t socket) const;

    std::expected<void, error> post_accept(std::unique_ptr<accept_context> ctx) const;
    std::expected<void, error> post_read(std::unique_ptr<read_context> ctx) const;
    std::expected<void, error> post_write(std::unique_ptr<write_context> ctx) const;

  private:
    io_processor();

    void on_accept(const accept_context& ctx);
    void on_read(const read_context& ctx);
    void on_write(const write_context& ctx);
  };
}
