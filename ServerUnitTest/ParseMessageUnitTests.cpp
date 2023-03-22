#include "CppUnitTest.h"

#include <bit>
#include <memory>

#include "Snowflake.h"
#include "SocketMessages.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ServerUnitTest
{
	TEST_CLASS(ParseMessages)
	{
	public:

		TEST_METHOD(ParseACKMessage)
		{
			Snowflake messageId{};

			SocketMessages::AcknowledgeMessage ackMessage{ messageId };

			std::vector<uint8_t> messageBuffer = ackMessage.Serialize();
			std::vector<uint8_t> buffer(messageBuffer.begin() + SocketMessages::MessageHeader::size, messageBuffer.end());

			Assert::IsFalse(ackMessage.ParseBody({ 0, 3, 4 }), L"Invalid data can be parsed");
			Assert::IsTrue(ackMessage.ParseBody(buffer), L"Valid data can't be parsed");

			Assert::AreEqual(
				static_cast<uint64_t>(messageId),
				static_cast<uint64_t>(ackMessage.GetAcknowledgedMessageId()),
				L"Acknowledged message Id id wrong"
			);
		}

		TEST_METHOD(ParseErrorMessage)
		{
			SocketMessages::ErrorCode errorCode{ SocketMessages::ErrorCode::InvalidMessage };

			SocketMessages::ErrorMessage errorMessage{ errorCode };

			std::vector<uint8_t> messageBuffer = errorMessage.Serialize();
			std::vector<uint8_t> buffer(messageBuffer.begin() + SocketMessages::MessageHeader::size, messageBuffer.end());

			Assert::IsFalse(errorMessage.ParseBody({ 0, 3, 4 }), L"Invalid data can be parsed");
			Assert::IsTrue(errorMessage.ParseBody(buffer), L"Valid data can't be parsed");

			Assert::AreEqual(
				static_cast<uint8_t>(errorCode),
				static_cast<uint8_t>(errorMessage.GetErrorCode()),
				L"Error code is wrong"
			);
		}

		TEST_METHOD(ParseHelloMessage)
		{
			SocketMessages::HelloMessage helloMessage{};

			std::vector<uint8_t> messageBuffer = helloMessage.Serialize();
			std::vector<uint8_t> buffer(messageBuffer.begin() + SocketMessages::MessageHeader::size, messageBuffer.end());

			Assert::IsFalse(helloMessage.ParseBody({ 0, 3, 4 }), L"Invalid data can be parsed");
			Assert::IsTrue(helloMessage.ParseBody(buffer), L"Valid data can't be parsed");

			Assert::AreEqual(
				static_cast<uint8_t>(CurrentVersion),
				static_cast<uint8_t>(helloMessage.GetVersion()),
				L"API version is wrong"
			);
		}

		TEST_METHOD(ParseIdentifyMessage)
		{
			std::string username = "Username";
			
			SocketMessages::IdentifyMessage identifyMessage{};

			identifyMessage.SetUsername(username);

			std::vector<uint8_t> messageBuffer = identifyMessage.Serialize();
			std::vector<uint8_t> buffer(messageBuffer.begin() + SocketMessages::MessageHeader::size, messageBuffer.end());

			Assert::IsFalse(identifyMessage.ParseBody({ 0, 3, 4 }), L"Invalid data can be parsed");
			Assert::IsTrue(identifyMessage.ParseBody(buffer), L"Valid data can't be parsed");

			Assert::AreEqual(username, identifyMessage.GetUsername(), L"Username is wrong");
		}

		TEST_METHOD(ParseKeepAliveMessage)
		{
			SocketMessages::KeepAliveMessage keepAliveMessage{};

			std::vector<uint8_t> messageBuffer = keepAliveMessage.Serialize();
			std::vector<uint8_t> buffer(messageBuffer.begin() + SocketMessages::MessageHeader::size, messageBuffer.end());

			Assert::IsFalse(keepAliveMessage.ParseBody({ 0, 3, 4 }), L"Invalid data can be parsed");
			Assert::IsTrue(keepAliveMessage.ParseBody(buffer), L"Valid data can't be parsed");
		}

		TEST_METHOD(ParseReceiveChatMessage)
		{
			std::string authorUsername = "Username";
			std::string chatMessage = "Message";

			SocketMessages::ReceiveChatMessage receiveChatMessage{};
			receiveChatMessage.SetAuthorUsername(authorUsername);
			receiveChatMessage.SetChatMessage(chatMessage);

			std::vector<uint8_t> messageBuffer = receiveChatMessage.Serialize();
			std::vector<uint8_t> buffer(messageBuffer.begin() + SocketMessages::MessageHeader::size, messageBuffer.end());

			Assert::IsFalse(receiveChatMessage.ParseBody({ 0, 3, 4 }), L"Invalid data can be parsed");
			Assert::IsTrue(receiveChatMessage.ParseBody(buffer), L"Valid data can't be parsed");

			Assert::AreEqual(authorUsername, receiveChatMessage.GetAuthorUsername(), L"Author username is wrong");
			Assert::AreEqual(chatMessage, receiveChatMessage.GetChatMessage(), L"Chat message is wrong");
		}

		TEST_METHOD(ParseSendChatMessage)
		{
			std::string chatMessage = "Message";

			SocketMessages::SendChatMessage sendChatMessage{};
			sendChatMessage.SetChatMessage(chatMessage);

			std::vector<uint8_t> messageBuffer = sendChatMessage.Serialize();
			std::vector<uint8_t> buffer(messageBuffer.begin() + SocketMessages::MessageHeader::size, messageBuffer.end());

			Assert::IsFalse(sendChatMessage.ParseBody({ 0, 3, 4 }), L"Invalid data can be parsed");
			Assert::IsTrue(sendChatMessage.ParseBody(buffer), L"Valid data can't be parsed");

			Assert::AreEqual(chatMessage, sendChatMessage.GetChatMessage(), L"Chat message is wrong");
		}
	};
}
