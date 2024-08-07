#include <algorithm>
#include <bit>
#include <cstdint>
#include <ios>
#include <iostream>
#include <random>

#include <asio/ip/tcp.hpp>
#include <connection.h>
#include <coroutine.h>
#include <message.h>
#include <snowflake.h>
#include <socket_messages.h>

#include "server.h"
#include "server_connection.h"


namespace pine
{
	server_connection::server_connection(
		asio::ip::tcp::socket& client_socket,
		server& server
	)
		: server_ref{ server },
		connection{ std::move(client_socket) }
	{

  async_task server_connection::start()
	}

	{
		co_await switch_thread(listen_thread);

		std::scoped_lock lock(connection_mutex);

		for (auto& callback : server_ref.on_connection_attemps_callbacks)
			callback(server_ref, server_connection::shared_from_this());

		if (!(co_await establish_connection()))
		{
			close();
			co_return;
		}

		std::cout << "[Server]   Client successfully connected: " << std::dec << id << std::endl;
		is_connected = true;

		for (auto& callback : server_ref.on_connection_callbacks)
			callback(server_ref, shared_from_this());

		while (is_connected)
		{
			auto&& message = co_await receive_message();

			if (message->header.type == socket_messages::message_type::INVALID_MESSAGE)
			{
				close();
				co_return;
			}

			server_ref.handle_message(shared_from_this(), message);
		}
	}

		if (!(co_await identify()))
		{
			std::cout << "[Server]   Client failed identify: " << std::dec << id << std::endl;
			co_return false;
		}

		std::cout << "[Server]   Client successfully identified in as \"" << user_data.username
			<< "\": " << std::dec << id << std::endl;

		co_return true;
	}
	}
