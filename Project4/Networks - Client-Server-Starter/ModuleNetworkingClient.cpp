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
	serverName = "Server";
	return true;
}

bool ModuleNetworkingClient::isRunning() const
{
	return state != ClientState::Stopped;
}

std::vector<std::string> ModuleNetworkingClient::split(std::string str, std::string delim)
{
	std::vector<std::string> tokens;
	auto start = 0U;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == std::string::npos) pos = str.length();
		std::string token = str.substr(prev, pos - prev);
		if (!token.empty()) tokens.push_back(token);
		prev = pos + delim.length();
	}
	while (pos < str.length() && prev < str.length());
	return tokens;
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
			Client client = ClientsConnected[(*iter).user];
			ImGui::TextColored(colors[client.color], (*iter).GetMessage().c_str());
		}

		ImGui::SetCursorPosY(ImGui::GetWindowPos().y + ImGui::GetWindowHeight() - 40);
		static char inputText[255];
		if (ImGui::InputText("## Message", inputText, 50))
		ImGui::SameLine();
		if (ImGui::Button("Send"))
		{
			
			if (inputText[0] == '/')
			{
				CallCommand(inputText, socket);
			}
			else
			{
				OutputMemoryStream packet;
				packet << ClientMessage::MESSAGE;
				packet << this->playerName;
				packet << std::string(inputText);
				sendPacket(packet, socket);
			}
		}
		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::CallCommand(char  inputText[255], SOCKET serverSocket)
{
	std::string strInputText(inputText);
	auto words = split(strInputText.substr(1, strInputText.size() - 1), " ");
	int numWords = words.size();
	if (numWords <= 0)
		return;

	if(words[0] == "help")
	{
				std::string strhelp = R"(
	.--.      .-'.      .--.      .--.      .--.      .--.      .`-.      .--.
:::::.\::::::::.\::::::::.\::::::::.\::::::::.\::::::::.\::::::::.\::::::::.\
'      `--'      `.-'      `--'      `--'      `--'      `-.'      `--'      `
The command we have are the following ones:
/users -> List all users connected
	.--.      .-'.      .--.      .--.      .--.      .--.      .`-.      .--.
:::::.\::::::::.\::::::::.\::::::::.\::::::::.\::::::::.\::::::::.\::::::::.\
'      `--'      `.-'      `--'      `--'      `--'      `-.'      `--'      `
)";
			addMessage(Message("Server", strhelp));
	}
	else if (words[0] == "list")
	{
		std::string strUsers("The users connected are: \n");
		for (auto iter = ClientsConnected.begin(); iter != ClientsConnected.end(); iter++)
		{
			if ((*iter).first != serverName)
			{
				strUsers += (*iter).first;
				strUsers += "\n";
			}
		}
		addMessage(Message("Server", strUsers));
	}
	else if (words[0] == "kick")
	{
		if (numWords < 2 )
			return;
		if (ClientsConnected.find(words[1]) != ClientsConnected.end())
		{
			OutputMemoryStream packet;
			packet << ClientMessage::COMMAND_KICK;
			packet << words[1];
			packet << playerName;
			sendPacket(packet, serverSocket);
		}
		else
		{
			addMessage(Message(serverName, words[1] + " is not connected, there is no user name with this name"));
		}
	}
	else if (words[0] == "whisper")
	{

	}
	else if (words[0] == "change_name")
	{

	}
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
			packet >> strmsg;
			packet >> color;
			Client myClient(playerName, color);
			ClientsConnected[playerName] = myClient;

			std::string serverName = "Server";
			Client serverClient(serverName, WHITE);
			ClientsConnected[serverName] = serverClient;

			Message newMessage(serverName, strmsg);
			msg.push_back(newMessage);

			int numClient = 0;
			packet >> numClient;
			for (int i = 0; i < numClient; i++)
			{
				std::string name;
				COLORS color;
				packet >> name;
				packet >> color;

				Client newClient(name, color); 
				ClientsConnected[name] = newClient;
			}
		}
	break;

	case ServerMessage::MESSAGE:
		{
			std::string strmsg;
			std::string clientName;
			packet >> clientName;
			packet >> strmsg;
			addMessage(Message(clientName, strmsg));
		}
	break;

	case ServerMessage::NEW_USER:
	{
		std::string name;
		COLORS color;
		packet >> name;
		packet >> color;
		Client newClient(name, color);
		ClientsConnected[name] = newClient;
		std::string nmsg, serverName;
		packet >> nmsg;
		packet >> serverName;
		addMessage(Message(serverName, nmsg));
	}
	break;
	case ServerMessage::COMMAND_KICK:
	{
		state = ClientState::Stopped;
		disconnect();
	}
		break;
	case ServerMessage::DISCONECTED:
		{
			
			std::string playerDisconnected;
			packet >> playerDisconnected;
			addMessage(Message(std::string("Server"), playerDisconnected + " just left, Good bye we will miss you!"));
			DeleteClient(playerDisconnected);
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

void ModuleNetworkingClient::addMessage(Message newMessage)
{
	msg.push_back(newMessage);
}

void ModuleNetworkingClient::DeleteClient(std::string name)
{
	ClientsConnected.erase(name);
}


