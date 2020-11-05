#include "ModuleNetworkingClient.h"


bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	playerName = pplayerName;
	//playerName += '\0';
	// x TODO(jesus): TCP connection stuff
	// x - Create the socket
	// x - Create the remote address object
	// x - Connect to the remote address
	// x- Add the created socket to the managed list of sockets using addSocket()
	CreateTCPSocket(socket);
	serverAddress.sin_family = AF_INET; // IPv4
	serverAddress.sin_port = htons(serverPort); // Port

	inet_pton(AF_INET, serverAddressStr, &serverAddress.sin_addr);

	int ret = connect(socket, (const  sockaddr*)&serverAddress, sizeof(serverAddress));

	if (ret == SOCKET_ERROR)
	{
		reportError("Error connecting socket!");
		return false;
	}

	addSocket(socket);

	// If everything was ok... change the state
	state = ClientState::Start;

	return true;
}

bool ModuleNetworkingClient::isRunning() const
{
	return state != ClientState::Stopped;
}

bool ModuleNetworkingClient::update()
{
	if (state == ClientState::Start)
	{
		OutputMemoryStream packet;
		packet << ClientMessage::Hello;
		packet << playerName;
		if (sendPacket(packet, socket))
		{
			state = ClientState::Logging;
		}
		else
		{
			disconnect();
			state = ClientState::Stopped;
		}
		//sendto(socket, playerName.c_str(), playerName.size(), 0, (const sockaddr*)&serverAddress, sizeof(serverAddress));
	}

	return true;
}

bool ModuleNetworkingClient::gui()
{
	if (state != ClientState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Client Window");

		Texture *tex = App->modResources->client;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("%s connected to the server...", playerName.c_str());

		for (auto iter = msg.begin(); iter != msg.end(); iter++)
		{
			ImGui::TextColored(yellow,(*iter).c_str());
		}

		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	ClientMessage clientMessage;
	packet >> clientMessage;

	if (clientMessage == ClientMessage::Welcome)
	{
		std::string strmsg;
		packet >> strmsg;
		msg.push_back(strmsg);
	}

	//state = ClientState::Stopped;
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

