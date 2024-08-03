#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <asio.hpp>

#include "coroutine.h"

namespace pine
{
	/// @brief A connection base class that is used by the server and the client.
	class connection
	{
	public:
		/// @brief Construct a connection with the given socket.
		explicit connection(asio::ip::tcp::socket socket);

		/// @brief Destroy the connection.
		virtual ~connection() = default;

		/// @brief Receive a raw message to the connection.
		/// @param buffer_size Number of bytes to receive.
		/// @return An asynchronous operation that returns the received bytes.
		async_operation<std::vector<uint8_t>> receive_raw_message(uint64_t const& buffer_size);

		/// @brief Send a raw message to the connection.
		/// @param buffer Buffer to send.
		/// @return An asynchronous task completed when the message has been sent.
		async_task send_raw_message(std::vector<uint8_t> const& buffer);

		/// @brief Close the connection.
		void close();

		/// @brief The id of the connection.
		snowflake id{};

		/// @brief The socket of the connection.
		asio::ip::tcp::socket socket;
	};
}