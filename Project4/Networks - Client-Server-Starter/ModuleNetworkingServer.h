#pragma once

#include "ModuleNetworking.h"

class ModuleNetworkingServer : public ModuleNetworking
{

public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingServer public methods
	//////////////////////////////////////////////////////////////////////

	bool start(int port);

	bool isRunning() const;



private:

	struct ConnectedSocket
	{
		sockaddr_in address;
		SOCKET socket;
		std::string playerName;
		COLORS color;
	};

	//////////////////////////////////////////////////////////////////////
	// Module virtual methods
	//////////////////////////////////////////////////////////////////////

	bool update() override;

	bool gui() override;



	//////////////////////////////////////////////////////////////////////
	// ModuleNetworking virtual methods
	//////////////////////////////////////////////////////////////////////

	bool isListenSocket(SOCKET socket) const override;

	void onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress) override;

	void onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet) override;

	void SendWelcomePacket(ConnectedSocket* playerSocket, std::string& playerName, const SOCKET& socket);

	void onSocketDisconnected(SOCKET socket) override;

	void BroadcastPacket(SOCKET socket, OutputMemoryStream& packet);

	void EmitPacket(OutputMemoryStream& packet);

	void EmitPacket(const char* buffer, uint32 size);

	COLORS GetColor();

	void SendConnectedUsers(SOCKET socket, OutputMemoryStream& packet);

	//////////////////////////////////////////////////////////////////////
	// State
	//////////////////////////////////////////////////////////////////////

	enum class ServerState
	{
		Stopped,
		Listening
	};

	ServerState state = ServerState::Stopped;

	SOCKET listenSocket;


	int colorCursor = 0;
	std::vector<ConnectedSocket> connectedSockets;
};

