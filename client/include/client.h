#pragma once

#include <cstdint>
#include <functional>
#include <thread>
#include <memory>
#include <string>
#include <vector>
#include "coroutine.h"
#include "client_connection.h"

namespace pine
{
	/// @brief A client that can connect to a single server.
	class client
	{
		friend class client_connection;
	public:
		/// @brief Initialize a client.
		client() = default;

		/// @brief Connect to a server.
		/// @param host: Host name or IP address of the server.
		/// @param port: TCP port of the server.
		/// @return True if the connection was successful, false otherwise.
		bool connect(std::string const& host = "localhost", uint16_t const& port = 80);

		/// @brief Disconnect from the server.
		void disconnect();

	private:
		std::unique_ptr<client_connection> connection;
	};
}
