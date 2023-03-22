#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <vector>

#include "Message.h"

namespace SocketMessages
{
	struct IdentifyMessage : Message
	{
		IdentifyMessage()
		{
			header.messageType = MessageType:: IdentifyMessage;
			header.bodySize = GetBodySize();
		}

		bool ParseBody(std::vector<uint8_t> const& buffer) override
		{
			if (buffer.size() != GetBodySize())
				return false;

			std::memcpy(std::bit_cast<void*>(&m_usernameLength), std::bit_cast<void*>(buffer.data()), sizeof(m_usernameLength));
			m_username = std::string(buffer.begin() + sizeof(m_usernameLength), buffer.end());

			return true;
		}

		std::vector<uint8_t> Serialize() const override
		{
			std::vector<uint8_t> buffer(MessageHeader::size + GetBodySize(), 0);
			std::vector<uint8_t> headerBuffer = header.Serialize();

			std::memcpy(&buffer[0], &headerBuffer[0], MessageHeader::size);
			std::memcpy(&buffer[MessageHeader::size], &m_usernameLength, sizeof(m_usernameLength));
			std::memcpy(&buffer[MessageHeader::size + sizeof(m_usernameLength)], &m_username[0], m_usernameLength);

			return buffer;
		}

		uint64_t GetBodySize() const final
		{
			return sizeof(m_usernameLength) + m_usernameLength;
		}

		[[nodiscard]] constexpr std::string const& GetUsername() const { return m_username; }
		constexpr bool SetUsername(std::string_view const& username)
		{
			if (username.length() < UsernameMinLength || username.length() > UsernameMaxLength)
				return false;

			m_username = username;
			m_usernameLength = static_cast<uint8_t>(username.length());

			header.bodySize = GetBodySize();

			return true;
		}

	private:
		uint8_t m_usernameLength{};
		std::string m_username{};
	};
}