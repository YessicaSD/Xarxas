#pragma once

// Add as many messages as you need depending on the
// functionalities that you decide to implement.

enum class ClientMessage
{
	HELLO,
	MESSAGE,
	COMMAND_KICK,
	WHISPER_MESSAGE,
	CHANGE_NAME,

};

enum class ServerMessage
{
	WELCOME,
	MESSAGE,
	WHISPER_MESSAGE,
	NONE_WELCOME,
	NEW_USER,
	DISCONECTED,
	COMMAND_KICK,
	CHANGE_NAME
};

