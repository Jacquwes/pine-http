#pragma once

#include "pch.h"

struct ServerException : std::exception
{
	ServerException(std::string_view const& message)
		: m_message{ message }
	{
	}

	char const* what() const noexcept override
	{
		return m_message.data();
	}
	
	std::string_view m_message;
};