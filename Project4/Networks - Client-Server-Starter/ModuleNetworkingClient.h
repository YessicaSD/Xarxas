#pragma once

#include "ModuleNetworking.h"
#include <map>



class Message
{
public:
	std::string user;
	std::string msg;
	Message(std::string user, std::string msg): user(user), msg(msg) {}
};

class ModuleNetworkingClient : public ModuleNetworking
{
public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingClient public methods
	//////////////////////////////////////////////////////////////////////
	bool start(const char *serverAddress, int serverPort, const char *playerName);
	bool isRunning() const;

private:

	//////////////////////////////////////////////////////////////////////
	// Module virtual methods
	//////////////////////////////////////////////////////////////////////

	bool update() override;

	bool gui() override;



	//////////////////////////////////////////////////////////////////////
	// ModuleNetworking virtual methods
	//////////////////////////////////////////////////////////////////////

	void onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet) override;

	void onSocketDisconnected(SOCKET socket) override;



	//////////////////////////////////////////////////////////////////////
	// Client state
	//////////////////////////////////////////////////////////////////////

	enum class ClientState
	{
		Stopped,
		Start,
		Logging
	};

	ClientState state = ClientState::Stopped;
	sockaddr_in serverAddress = {};
	SOCKET socket = INVALID_SOCKET;
	std::map <std::string, Client > ClientsConnected;
	std::string playerName;
	std::list<Message> msg;
};

