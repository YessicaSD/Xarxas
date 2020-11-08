#pragma once

#include "ModuleNetworking.h"
#include <map>
class Message
{
public:
	SOCKET user = 0;
	std::string msg;
	void PrintMessage(Client client) {
		std::string outmsg(client.name + ": " + msg);
		ImGui::TextColored(colors[client.color], outmsg.c_str());
	}
	Message(std::string msg) :msg(msg) {}
	Message(SOCKET user, std::string msg) : user(user), msg(msg) {}
};

class ModuleNetworkingClient : public ModuleNetworking
{
public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingClient public methods
	//////////////////////////////////////////////////////////////////////
	bool start(const char *serverAddress, int serverPort, const char *playerName);
	bool isRunning() const;

	static std::vector<std::string> split(std::string s, std::string delim);

private:

	

	//////////////////////////////////////////////////////////////////////
	// Module virtual methods
	//////////////////////////////////////////////////////////////////////

	bool update() override;

	bool gui() override;

	bool CallCommand(char  inputText[255], SOCKET serverSocket);

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworking virtual methods
	//////////////////////////////////////////////////////////////////////

	void onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet) override;

	void onSocketDisconnected(SOCKET socket) override;

	void addMessage(Message newMessage);

	void DeleteClient(SOCKET name);

	bool IsUser(std::string name);

	bool GetClient(const std::string& name, Client& client);

	bool IsUserNameFree(std::string name);

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
	std::map <SOCKET, Client > ClientsConnected;
	std::string playerName;
	std::list<Message> msg;
	std::string serverName;
	friend class Message;
};

