#include "ModuleNetworkingServer.h"




//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::start(int port)
{
	CreateTCPSocket(listenSocket);
	
	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET; // IPv4
	bindAddr.sin_port = htons(port); // Port
	bindAddr.sin_addr.S_un.S_addr = INADDR_ANY; // Any local IP address

	int res = bind(listenSocket, (const struct sockaddr*) & bindAddr, sizeof(bindAddr));
	
	listen(listenSocket, 1024);

	addSocket(listenSocket);
	// Where backlog is the maximum number of simultaneous incoming connections allowed.

	state = ServerState::Listening;
	serverName = "Server";
	return true;
}

bool ModuleNetworkingServer::isRunning() const
{
	return state != ServerState::Stopped;
}



//////////////////////////////////////////////////////////////////////
// Module virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::update()
{
	return true;
}

bool ModuleNetworkingServer::gui()
{
	if (state != ServerState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Server Window");

		Texture *tex = App->modResources->server;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("List of connected sockets:");

		for (auto &connectedSocket : connectedSockets)
		{
			ImGui::Separator();
			ImGui::Text("Socket ID: %d", connectedSocket.socket);
			ImGui::Text("Address: %d.%d.%d.%d:%d",
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b1,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b2,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b3,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b4,
				ntohs(connectedSocket.address.sin_port));
			ImGui::Text("Player name: %s", connectedSocket.playerName.c_str());
		}

		ImGui::End();
	}

	return true;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::isListenSocket(SOCKET socket) const
{
	return socket == listenSocket;
}

void ModuleNetworkingServer::onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress)
{
	// Add a new connected socket to the list
	ConnectedSocket connectedSocket;
	connectedSocket.socket = socket;
	connectedSocket.address = socketAddress;
	connectedSockets.push_back(connectedSocket);
}

void ModuleNetworkingServer::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	ClientMessage clientMessage;
	packet >> clientMessage;
	switch (clientMessage)
	{
		case ClientMessage::HELLO:
		{
			std::string playerName;
			packet >> playerName;
			ConnectedSocket* playerSocket = nullptr;
			for (auto& connectedSocket : connectedSockets)
			{
				if (connectedSocket.playerName == playerName)
				{
					playerSocket = nullptr;
					break;
				}
				if (connectedSocket.socket == socket)
				{
					playerSocket = &connectedSocket;
				}
			}

			if (playerSocket != nullptr)
			{
				SendWelcomePacket(playerSocket, playerName, socket);
			}
			else
			{
				OutputMemoryStream wpacket;
				wpacket << ServerMessage::NONE_WELCOME;
				wpacket << playerName;	
				sendPacket(wpacket, socket);
			}
		}
		break;

		case ClientMessage::MESSAGE:
		{
			OutputMemoryStream outPacket;
			outPacket << ServerMessage::MESSAGE;
			std::string msg;
			packet >> msg;
			outPacket << msg;
			outPacket << socket;
			EmitPacket(outPacket);
		}
		break;
		case ClientMessage::COMMAND_KICK:
		{
			std::string kickName, commandedUser;
			packet >> kickName;
			packet >> commandedUser;

			for (auto& connectedSocket : connectedSockets)
			{
				OutputMemoryStream outPacket;
				if (connectedSocket.playerName != kickName)
				{
					outPacket << ServerMessage::MESSAGE;
					outPacket << 0;
					outPacket << std::string(commandedUser + "just kicked " + kickName);
					sendPacket(outPacket, connectedSocket.socket);
				}
				else
				{
					outPacket << ServerMessage::COMMAND_KICK;
					sendPacket(outPacket, connectedSocket.socket);
				}
			}
		}
		break;
		case ClientMessage::WHISPER_MESSAGE:
		{
			SOCKET wsocket;
			std::string msg;
			packet >> wsocket;
			packet >> msg;
			OutputMemoryStream outpacket;
			outpacket << ServerMessage::MESSAGE;
			outpacket << msg;
			outpacket << socket;
			sendPacket(outpacket, wsocket);
		}
		break;
		case ClientMessage::CHANGE_NAME:
		{
			std::string name;
			packet >> name;

			OutputMemoryStream outpacket;
			outpacket << ServerMessage::CHANGE_NAME;
			outpacket << name;
			outpacket << socket;
			EmitPacket(outpacket);
		}
		break;
		case ClientMessage::CHANGE_COLOR:
		{
			int color;
			packet >> color;
			OutputMemoryStream outpacket;
			outpacket << ServerMessage::CHANGE_COLOR;
			outpacket << socket;
			outpacket << color;
			for (auto iter = connectedSockets.begin(); iter != connectedSockets.end(); ++iter)
			{
				if ((*iter).socket == socket)
				{
					(*iter).color = (COLORS)color;
				}
			}
			EmitPacket(outpacket);

		}
		break;
		default:
			break;
	}


}

void ModuleNetworkingServer::SendWelcomePacket(ConnectedSocket* playerSocket, std::string& playerName, const SOCKET& socket)
{
	int color = GetColor();
	playerSocket->playerName = playerName;
	playerSocket->color = (COLORS)color;
	
	OutputMemoryStream packet;
	packet << ServerMessage::NEW_USER;
	packet << playerName;
	packet << color;	
	packet << socket;
	BroadcastPacket(socket, packet);

	OutputMemoryStream wpacket;

	wpacket << ServerMessage::WELCOME;
	wpacket << color;
	wpacket << socket;
	SendConnectedUsers(socket, wpacket);
	sendPacket(wpacket, socket);
}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket)
{
	// Remove the connected socket from the list
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		auto &connectedSocket = *it;
		if (connectedSocket.socket == socket)
		{
			OutputMemoryStream packet;
			packet << ServerMessage::DISCONECTED;
			packet << socket;
			BroadcastPacket(socket, packet);
			connectedSockets.erase(it);
			break;
		}
	}
}

void ModuleNetworkingServer::BroadcastPacket(SOCKET socket, OutputMemoryStream& packet)
{
	for (auto& connectedSocket : connectedSockets)
	{

		if (connectedSocket.socket != socket && !isListenSocket(connectedSocket.socket))
		{
			sendPacket(packet, connectedSocket.socket);
		}
	}
}

void ModuleNetworkingServer::EmitPacket(OutputMemoryStream& packet)
{
	for (auto& connectedSocket : connectedSockets)
	{
		if (!isListenSocket(connectedSocket.socket))
			sendPacket(packet, connectedSocket.socket);
	}
}

void ModuleNetworkingServer::EmitPacket(const char* buffer, uint32 size)
{
	for (auto& connectedSocket : connectedSockets)
	{
		if (!isListenSocket(connectedSocket.socket))
			sendPacket(buffer, size, connectedSocket.socket);
	}
}

COLORS ModuleNetworkingServer::GetColor()
{
	++colorCursor;
	if (colorCursor > COLORS::MAX_COLORS)
	{
		colorCursor = 0;
	}
	return (COLORS)colorCursor;
}

void ModuleNetworkingServer::SendConnectedUsers(SOCKET socket, OutputMemoryStream& packet)
{
	int numClients = connectedSockets.size();
	if(numClients==0)
		packet << numClients;
	else
		packet << numClients-1;

	if (numClients > 1)
	{
		for (auto iter = connectedSockets.begin(); iter != connectedSockets.end(); ++iter)
		{
			if ((*iter).socket != socket)
			{
				packet << (*iter).playerName;
				packet << (*iter).color;
				packet << (*iter).socket;
			}
		}
	}
}
