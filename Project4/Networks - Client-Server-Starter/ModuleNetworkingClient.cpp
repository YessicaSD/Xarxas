#include "ModuleNetworkingClient.h"


bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	playerName = pplayerName;
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
		packet << ClientMessage::HELLO;
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
			COLORS msgColor = ClientsConnected[(*iter).user].color;
			ImGui::TextColored(colors[msgColor],(*iter).msg.c_str());
		}

		ImGui::SetCursorPosY(ImGui::GetWindowPos().y + ImGui::GetWindowHeight() - 40);
		static char inputText[50];
		if (ImGui::InputText("## Message", inputText, 50))
		ImGui::SameLine();
		if (ImGui::Button("Send"))
		{
			OutputMemoryStream packet;
			packet << ClientMessage::MESSAGE;
			packet << this->playerName;
			packet << std::string(inputText);
			sendPacket(packet, socket);
		}
		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	ServerMessage serverMessage;
	packet >> serverMessage;
	switch (serverMessage)
	{
	case ServerMessage::WELCOME:
		{
			std::string strmsg;
			COLORS color;
			unsigned int mySocket;
			packet >> strmsg;
			packet >> color;
			this->color = color;
			Client myClient(playerName, color);
			ClientsConnected[playerName] = myClient;
			std::string serverName = "Server";
			Client serverClient(serverName, WHITE);
			ClientsConnected[serverName] = serverClient;
			Message newMessage(serverName, strmsg);
			msg.push_back(newMessage);
		}
	break;

	case ServerMessage::MESSAGE:
		{
			std::string strmsg;
			std::string clientName;
			packet >> clientName;
			packet >> strmsg;
			Message newMessage(clientName, strmsg);
			msg.push_back(newMessage);
		}
	break;

	case ServerMessage::NEW_USER:
	{
		std::string name;
		COLORS color;
		packet >> name;
		packet >> color;
		Client newClient(name, color);
	}
	break;

	case ServerMessage::NONE_WELCOME:
	{
		std::string name;
		packet >> name;
		ELOG("%s is already taken :(", name.c_str());
		state = ClientState::Stopped;
	}
		break;
	default:
		break;
	}
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

