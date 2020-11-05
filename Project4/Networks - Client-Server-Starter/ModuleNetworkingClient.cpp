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
			ImGui::TextColored(yellow,(*iter).c_str());
		}
		ImGui::SetCursorPosY(ImGui::GetWindowPos().y + ImGui::GetWindowHeight() - 40);
		static char inputText[50];
		if (ImGui::InputText("## Message", inputText, 50))
		ImGui::SameLine();
		if (ImGui::Button("Send"))
		{
			OutputMemoryStream packet;
			packet << ClientMessage::MESSAGE;
			packet << std::string(inputText);
			sendPacket(packet, socket);
		}
		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	ClientMessage clientMessage;
	packet >> clientMessage;
	switch (clientMessage)
	{
	case ClientMessage::HELLO:
		break;
	case ClientMessage::WELCOME:
	case ClientMessage::MESSAGE:
		{
			std::string strmsg;
			packet >> strmsg;
			msg.push_back(strmsg);
		}
		break;
	case ClientMessage::NONE_WELCOME:
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

