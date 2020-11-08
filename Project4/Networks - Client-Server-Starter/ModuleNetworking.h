#pragma once
#include <list>

class Client
{
public:
	std::string name = "";
	COLORS color = WHITE;
	SOCKET socket = 0;
	bool connected = true;
	Client() {}
	Client(std::string name, COLORS color) :name(name), color(color) {}
	Client(std::string name, COLORS color, SOCKET socket) :name(name), color(color), socket(socket) {}
};

class ModuleNetworking : public Module
{
public:
	static bool CreateTCPSocket(SOCKET& socket);
	static bool sendPacket(const OutputMemoryStream & packet, SOCKET socket);
	bool sendPacket(const char* buffer, uint32 size, SOCKET socket);
private:

	//////////////////////////////////////////////////////////////////////
	// Module virtual methods
	//////////////////////////////////////////////////////////////////////

	bool init() override;

	bool preUpdate() override;

	bool cleanUp() override;

	
	

	//////////////////////////////////////////////////////////////////////
	// Socket event callbacks
	//////////////////////////////////////////////////////////////////////

	virtual bool isListenSocket(SOCKET socket) const { return false; }

	virtual void onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress) { }

	virtual void onSocketReceivedData(SOCKET s, const InputMemoryStream& packet) = 0;

	virtual void onSocketDisconnected(SOCKET s) = 0;



protected:

	std::vector<SOCKET> sockets;

	void addSocket(SOCKET socket);

	void disconnect();

	static void reportError(const char *message);
};

