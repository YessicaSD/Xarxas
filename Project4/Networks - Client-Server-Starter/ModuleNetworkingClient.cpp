#include "ModuleNetworkingClient.h"
#include <string>


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
	bool isWhisper = false;
	do
	{
		pos = str.find(delim, prev);
		if (pos == std::string::npos) pos = str.length();
		std::string token;

		if (isWhisper && tokens.size() == 2)
		{
			token = str.substr(prev, str.size() - prev);
			pos = str.size();
		}
		else
		{
			token = str.substr(prev, pos - prev);
			if (tokens.size() == 0 && token == "whisper")
			{
				isWhisper = true;
			}
		}

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
		ImGui::BeginChild("##Chat");
		for (auto iter = msg.begin(); iter != msg.end(); iter++)
		{
			Client client = ClientsConnected[(*iter).user];
			(*iter).PrintMessage(client);
		}
		ImGui::EndChild();

		static char inputText[255];
		ImGui::InputText("## Message", inputText, 50);
		ImGui::SameLine();
		if (ImGui::Button("Send"))
		{
			bool command = false;
			if (inputText[0] == '/')
			{
				command = CallCommand(inputText, socket);
			}
			if(!command)
			{
				OutputMemoryStream packet;
				packet << ClientMessage::MESSAGE;
				packet << std::string(inputText);
				sendPacket(packet, socket);
			}
		}
		ImGui::End();
	}

	return true;
}

bool ModuleNetworkingClient::CallCommand(char  inputText[255], SOCKET serverSocket)
{
	std::string strInputText(inputText);
	auto words = split(strInputText.substr(1, strInputText.size() - 1), " ");
	int numWords = words.size();
	bool ret = true;
	if (numWords <= 0)
		return false;

	if(words[0] == "help")
	{
				std::string strhelp = R"(
	.--.      .-'.      .--.      .--.      .--.      .--.      .`-.      .--.
:::::.\::::::::.\::::::::.\::::::::.\::::::::.\::::::::.\::::::::.\::::::::.\
'      `--'      `.-'      `--'      `--'      `--'      `-.'      `--'      `
The command we have are the following ones:
/help -> Show all commands
/list -> List all users connected
/kick -> to ban some user from the chat.
/whisper -> to send a message only to one user.
/change_name -> to change your username.

This project is made by Yessica Servin Dominguez and Jaume Montagut i Guix
	.--.      .-'.      .--.      .--.      .--.      .--.      .`-.      .--.
:::::.\::::::::.\::::::::.\::::::::.\::::::::.\::::::::.\::::::::.\::::::::.\
'      `--'      `.-'      `--'      `--'      `--'      `-.'      `--'      `
)";
			addMessage(Message(strhelp));
	}
	else if (words[0] == "list")
	{
		int numClients = ClientsConnected.size() - 1;
		
		std::string strUsers("The number of users connected is " + std::to_string(numClients) +"\nThe users connected are: \n");
		for (auto iter = ClientsConnected.begin(); iter != ClientsConnected.end(); iter++)
		{
			if ((*iter).first != 0)
			{
				strUsers += (*iter).second.name;
				strUsers += "\n";
			}
		}
		addMessage(Message(strUsers));
	}
	else if (words[0] == "kick")
	{
		if (numWords < 2 )
			return false;
		if (IsUser(words[1]))
		{
			OutputMemoryStream packet;
			packet << ClientMessage::COMMAND_KICK;
			packet << words[1];
			packet << playerName;
			sendPacket(packet, serverSocket);
		}
		else
		{
			addMessage(Message(words[1] + " is not connected, there is no user name with this name"));
		}
	}
	else if (words[0] == "whisper")
	{
		if (numWords < 3)
			return false;
		Client wclient;
		if (GetClient(words[1], wclient))
		{
			OutputMemoryStream packet;
			packet << ClientMessage::WHISPER_MESSAGE;
			packet << wclient.socket;
			packet << words[2];
			sendPacket(packet, serverSocket);
		}
	}
	else if (words[0] == "change_name")
	{
		if (numWords < 2)
			return false;
		if (IsUserNameFree(words[1]))
		{
			OutputMemoryStream packet;
			packet << ClientMessage::CHANGE_NAME;
			packet << words[1];
			sendPacket(packet, socket);
		}
	}
	else
	{
		ret = false;
	}
	return ret;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	ServerMessage serverMessage;
	packet >> serverMessage;
	switch (serverMessage)
	{
	case ServerMessage::WELCOME:
		{
			COLORS color;
			SOCKET mysocket;
			packet >> color;
			packet >> mysocket;

			Client myClient(playerName, color, mysocket);
			ClientsConnected[mysocket] = myClient;

			Client serverClient(serverName, WHITE);
			ClientsConnected[0] = serverClient;

			std::string welcomemsg = R"(
==========================================================================

                         /$$                                                  
                        | $$                                                  
 /$$  /$$  /$$  /$$$$$$ | $$  /$$$$$$$  /$$$$$$  /$$$$$$/$$$$   /$$$$$$       
| $$ | $$ | $$ /$$__  $$| $$ /$$_____/ /$$__  $$| $$_  $$_  $$ /$$__  $$      
| $$ | $$ | $$| $$$$$$$$| $$| $$      | $$  \ $$| $$ \ $$ \ $$| $$$$$$$$      
| $$ | $$ | $$| $$_____/| $$| $$      | $$  | $$| $$ | $$ | $$| $$_____/      
|  $$$$$/$$$$/|  $$$$$$$| $$|  $$$$$$$|  $$$$$$/| $$ | $$ | $$|  $$$$$$$      
 \_____/\___/  \_______/|__/ \_______/ \______/ |__/ |__/ |__/ \_______/           


===========================================================================
)";
			welcomemsg += "\n **Welcome to the GenerationX 3D Revolution server " + playerName + " **";
			Message newMessage(welcomemsg);
			msg.push_back(newMessage);

			int numClient = 0;
			packet >> numClient;
			for (int i = 0; i < numClient; i++)
			{
				std::string name;
				COLORS color;
				SOCKET newSocket;
				packet >> name;
				packet >> color;
				packet >> newSocket;
				Client newClient(name, color, newSocket); 
				ClientsConnected[newSocket] = newClient;
			}
		}
	break;

	case ServerMessage::MESSAGE:
		{
			std::string strmsg;
			SOCKET clientSocket;
			packet >> strmsg;
			packet >> clientSocket;
			addMessage(Message(clientSocket, strmsg));
		}
	break;

	case ServerMessage::NEW_USER:
	{
		std::string name;
		COLORS color;
		SOCKET newSocket;
		packet >> name;
		packet >> color;
		packet >> newSocket;

		Client newClient(name, color, newSocket);
		ClientsConnected[newSocket] = newClient;
		addMessage(Message(std::string(name + " just joined this server!")));
	}
	break;
	case ServerMessage::COMMAND_KICK:
	{
		state = ClientState::Stopped;
		disconnect();
	}
	break;
	
	case ServerMessage::CHANGE_NAME:
	{
		std::string newName;
		SOCKET newSocket;
		packet >> newName;
		packet >> newSocket;
		if (ClientsConnected[newSocket].name == playerName)
		{
			playerName = newName;
		}
		ClientsConnected[newSocket].name = newName;
	}
	break;

	case ServerMessage::DISCONECTED:
		{
			SOCKET playerDisconnected;
			packet >> playerDisconnected;
			addMessage(Message(ClientsConnected[playerDisconnected].name + " just left, Good bye we will miss you!"));
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

void ModuleNetworkingClient::DeleteClient(SOCKET name)
{
	ClientsConnected.erase(name);
}

bool ModuleNetworkingClient::IsUser(std::string name)
{
	for (auto iter = ClientsConnected.begin(); iter != ClientsConnected.end(); ++iter)
	{
		if ((*iter).second.name == name)
			return true;
	}
	return false;
}
bool ModuleNetworkingClient::GetClient(const std::string& name, Client& client)
{
	for (auto iter = ClientsConnected.begin(); iter != ClientsConnected.end(); ++iter)
	{
		if ((*iter).second.name == name)
		{
			client = (*iter).second;
			return true;
		}
	}
	return false;
}

bool ModuleNetworkingClient::IsUserNameFree(std::string name)
{
	for (auto iter = ClientsConnected.begin(); iter != ClientsConnected.end(); ++iter)
	{
		if ((*iter).second.name == name)
			return false;
	}
	return true;
}

