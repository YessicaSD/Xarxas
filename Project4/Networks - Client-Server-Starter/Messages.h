#pragma once

// Add as many messages as you need depending on the
// functionalities that you decide to implement.

enum class ClientMessage
{
	HELLO,
	MESSAGE,
	WELCOME_FROM_OTHERS,
};

enum class ServerMessage
{
	WELCOME,
	MESSAGE,
	NONE_WELCOME,
	NEW_USER,
	DISCONECTED,
};

