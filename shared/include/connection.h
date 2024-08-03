#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "coroutine.h"
#include "snowflake.h"

#ifdef _WIN32
#include <WinSock2.h>
#endif // _WIN32


namespace pine
{
	/// @brief A connection base class that is used by the server and the client.
	class connection
	{
	public:
    explicit connection(SOCKET socket, snowflake id = snowflake());

		/// @brief Destroy the connection.
		virtual ~connection() = default;

		/// @brief Receive a raw message to the connection.
		/// @param buffer_size Number of bytes to receive.
		/// @return An asynchronous operation that returns the received bytes.
    async_operation<std::vector<uint8_t>> receive_raw_message(const uint64_t& buffer_size);

		/// @brief Send a raw message to the connection.
		/// @param buffer Buffer to send.
		/// @return An asynchronous task completed when the message has been sent.
    async_task send_raw_message(const std::vector<uint8_t>& buffer);

		/// @brief Close the connection.
		void close();

    /// @brief Get the id of the connection.
    /// @return The id of the connection.
    constexpr const snowflake& get_id() const noexcept { return id; }

  private:
		/// @brief The id of the connection.
    snowflake id;

		/// @brief The socket of the connection.
  #ifdef _WIN32
    SOCKET socket;
  #endif // _WIN32
	};
}