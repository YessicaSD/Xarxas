#pragma once

#include "ModuleNetworking.h"

class ModuleNetworkingClient : public ModuleNetworking
{
public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingClient public methods
	//////////////////////////////////////////////////////////////////////

	void setServerAddress(const char *serverAddress, uint16 serverPort);

	void setPlayerInfo(const char *playerName, uint8 spaceshipType);

	uint32 getNetworkId();

	float secondsSinceLastReplication = 0.1f;

	void ProcessInput(uint32 index, GameObject* obj);

	uint32 inputIndex = 0u;

private:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworking virtual methods
	//////////////////////////////////////////////////////////////////////

	bool isClient() const override { return true; }

	void onStart() override;

	void onGui() override;

	void onPacketReceived(const InputMemoryStream &packet, const sockaddr_in &fromAddress) override;

	void onUpdate() override;

	void onConnectionReset(const sockaddr_in &fromAddress) override;

	void onDisconnect() override;

	

	//////////////////////////////////////////////////////////////////////
	// Client state
	//////////////////////////////////////////////////////////////////////

	enum class ClientState
	{
		Stopped,
		Connecting,
		Connected
	};

	ClientState state = ClientState::Stopped;

	std::string serverAddressStr;
	uint16 serverPort = 0;

	sockaddr_in serverAddress = {};
	std::string playerName = "player";
	uint8 spaceshipType = 0;

	uint32 playerId = 0;
	uint32 networkId = 0;


	// Connecting stage

	float secondsSinceLastHello = 0.0f;


	// Input ///////////

	static const int MAX_INPUT_DATA_SIMULTANEOUS_PACKETS = 64;

	InputPacketData inputData[MAX_INPUT_DATA_SIMULTANEOUS_PACKETS];
	uint32 inputDataFront = 0;
	uint32 inputDataBack = 0;

	float inputDeliveryIntervalSeconds = 0.05f;
	float secondsSinceLastInputDelivery = 0.0f;
	float timeSinceLastReplication = 0;
	


	//////////////////////////////////////////////////////////////////////
	// Virtual connection
	//////////////////////////////////////////////////////////////////////

	// TODO(you): UDP virtual connection lab session

	ReplicationManagerClient replicationManager;
	DeliveryManagerClient deliveryManager;



	//////////////////////////////////////////////////////////////////////
	// Latency management
	//////////////////////////////////////////////////////////////////////

	// TODO(you): Latency management lab session

	friend class ReplicationManagerClient;
};

