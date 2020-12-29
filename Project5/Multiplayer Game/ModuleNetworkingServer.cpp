#include "ModuleNetworkingServer.h"



//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

void ModuleNetworkingServer::setListenPort(int port)
{
	listenPort = port;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

void ModuleNetworkingServer::onStart()
{
	if (!createSocket()) return;

	// Reuse address
	int enable = 1;
	int res = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));
	if (res == SOCKET_ERROR) {
		reportError("ModuleNetworkingServer::start() - setsockopt");
		disconnect();
		return;
	}

	// Create and bind to local address
	if (!bindSocketToPort(listenPort)) {
		return;
	}

	state = ServerState::Listening;
}

void ModuleNetworkingServer::onGui()
{
	if (ImGui::CollapsingHeader("ModuleNetworkingServer", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Connection checking info:");
		ImGui::Text(" - Ping interval (s): %f", PING_INTERVAL_SECONDS);
		ImGui::Text(" - Disconnection timeout (s): %f", DISCONNECT_TIMEOUT_SECONDS);

		ImGui::Separator();

		if (state == ServerState::Listening)
		{
			int count = 0;

			for (ClientProxy &proxy : clientProxies)
			{
				if (proxy.connected)
				{
					ImGui::Text("CLIENT %d", count++);
					ImGui::Text(" - address: %d.%d.%d.%d",
						proxy.address.sin_addr.S_un.S_un_b.s_b1,
						proxy.address.sin_addr.S_un.S_un_b.s_b2,
						proxy.address.sin_addr.S_un.S_un_b.s_b3,
						proxy.address.sin_addr.S_un.S_un_b.s_b4);
					ImGui::Text(" - port: %d", ntohs(proxy.address.sin_port));
					ImGui::Text(" - name: %s", proxy.name.c_str());
					ImGui::Text(" - id: %d", proxy.clientId);
					if (proxy.gameObject != nullptr)
					{
						ImGui::Text(" - gameObject net id: %d", proxy.gameObject->networkId);
					}
					else
					{
						ImGui::Text(" - gameObject net id: (null)");
					}
					
					ImGui::Separator();
				}
			}

			ImGui::Checkbox("Render colliders", &App->modRender->mustRenderColliders);
		}
	}
}

void ModuleNetworkingServer::onPacketReceived(const InputMemoryStream &packet, const sockaddr_in &fromAddress)
{
	if (state == ServerState::Listening)
	{
		uint32 protoId;
		packet >> protoId;
		if (protoId != PROTOCOL_ID) return;

		ClientMessage message;
		packet >> message;

		ClientProxy *proxy = getClientProxy(fromAddress);

		if (message == ClientMessage::Hello)
		{
			if (proxy == nullptr)
			{
				proxy = createClientProxy();

				if (proxy != nullptr)
				{
					std::string playerName;
					uint8 spaceshipType;
					packet >> playerName;
					packet >> spaceshipType;

					proxy->address.sin_family = fromAddress.sin_family;
					proxy->address.sin_addr.S_un.S_addr = fromAddress.sin_addr.S_un.S_addr;
					proxy->address.sin_port = fromAddress.sin_port;
					proxy->connected = true;
					proxy->name = playerName;
					proxy->clientId = nextClientId++;

					// Create new network object
					vec2 initialPosition = 500.0f * vec2{ Random.next() - 0.5f, Random.next() - 0.5f};
					float initialAngle = 360.0f * Random.next();
					proxy->gameObject = spawnPlayer(spaceshipType, initialPosition, initialAngle);
				}
				else
				{
					// NOTE(jesus): Server is full...
				}
			}

			if (proxy != nullptr)
			{
				// Send welcome to the new player
				OutputMemoryStream welcomePacket;
				welcomePacket << PROTOCOL_ID;
				welcomePacket << ServerMessage::Welcome;
				welcomePacket << proxy->clientId;
				welcomePacket << proxy->gameObject->networkId;
				sendPacket(welcomePacket, fromAddress);

				// Send all network objects to the new player
				uint16 networkGameObjectsCount;
				GameObject *networkGameObjects[MAX_NETWORK_OBJECTS];
				App->modLinkingContext->getNetworkGameObjects(networkGameObjects, &networkGameObjectsCount);
				for (uint16 i = 0; i < networkGameObjectsCount; ++i)
				{
					if (proxy->gameObject->networkId != networkGameObjects[i]->networkId)
					{
						proxy->replicationManager.Create(networkGameObjects[i]->networkId);
					}
				}

				OutputMemoryStream packet;
				proxy->replicationManager.Write(packet, &proxy->deliveryManager, proxy->replicationManager.replicationCommands);
				sendPacket(packet, proxy->address);


				LOG("Message received: hello - from player %s", proxy->name.c_str());
			}
			else
			{
				OutputMemoryStream unwelcomePacket;
				unwelcomePacket << PROTOCOL_ID;
				unwelcomePacket << ServerMessage::Unwelcome;
				sendPacket(unwelcomePacket, fromAddress);

				WLOG("Message received: UNWELCOMED hello - server is full");
			}
		}
		else if (message == ClientMessage::Input)
		{
			// Process the input packet and update the corresponding game object
			if (proxy != nullptr && IsValid(proxy->gameObject))
			{
				proxy->lastPacketReceivedTime = Time.time;

				proxy->deliveryManager.processAckdSequenceNumbers(packet);

				// Read input data
				while (packet.RemainingByteCount() > 0)
				{
					InputPacketData inputData;
					packet >> inputData.sequenceNumber;
					packet >> inputData.horizontalAxis;
					packet >> inputData.verticalAxis;
					packet >> inputData.mouseX;
					packet >> inputData.mouseY;
					packet >> inputData.buttonBits;
					packet >> inputData.mouseBits;

					if (inputData.sequenceNumber >= proxy->nextExpectedInputSequenceNumber)
					{
						proxy->gamepad.horizontalAxis = inputData.horizontalAxis;
						proxy->gamepad.verticalAxis = inputData.verticalAxis;
						proxy->mouse.worldX = inputData.mouseX;
						proxy->mouse.worldY = inputData.mouseY;
						unpackInputControllerButtons(inputData.buttonBits, proxy->gamepad);
						unpackMouseControllerButtons(inputData.mouseBits, proxy->mouse);
						proxy->gameObject->behaviour->onInput(proxy->gamepad, proxy->mouse);
						proxy->replicationManager.lastClientInputReceived = inputData.sequenceNumber;
						proxy->nextExpectedInputSequenceNumber = inputData.sequenceNumber + 1;
						proxy->sendInputConfirmation = true;
					}
				}
			}
		}

		// TODO(you): UDP virtual connection lab session
	}
}

void ModuleNetworkingServer::onUpdate()
{
	if (state == ServerState::Listening)
	{
		// Handle networked game object destructions
		for (DelayedDestroyEntry &destroyEntry : netGameObjectsToDestroyWithDelay)
		{
			if (destroyEntry.object != nullptr)
			{
				destroyEntry.delaySeconds -= Time.deltaTime;
				if (destroyEntry.delaySeconds <= 0.0f)
				{
					destroyNetworkObject(destroyEntry.object);
					destroyEntry.object = nullptr;
				}
			}
		}


		for (ClientProxy& proxy : clientProxies)
		{
			if (proxy.connected) {
				if (proxy.sendInputConfirmation == true && Time.time > proxy.lastInputConfirmationTime + INPUT_CONFIRMATION_INTERVAL)
				{
					OutputMemoryStream packet;
					packet << PROTOCOL_ID;
					packet << ServerMessage::InputConfirmation;
					packet << proxy.nextExpectedInputSequenceNumber-1;
					sendPacket(packet, proxy.address);
					proxy.lastInputConfirmationTime = Time.time;
				}

				if (Time.time > proxy.lastPacketReceivedTime + DISCONNECT_TIMEOUT_SECONDS) {
					destroyClientProxy(&proxy);
				}

				// Don't let the client proxy point to a destroyed game object
				if (!IsValid(proxy.gameObject))
				{
					proxy.gameObject = nullptr;
				}

				if (Time.time > proxy.lastReplicationSendTime + REPLICATION_SEND_INTERVAL) {
					if (proxy.replicationManager.replicationCommands.size() > 0) {
						OutputMemoryStream packet;
						proxy.replicationManager.Write(packet, &proxy.deliveryManager, proxy.replicationManager.replicationCommands);
						sendPacket(packet, proxy.address);
					}
					proxy.lastReplicationSendTime = Time.time;
				}

				// TODO(you): Reliability on top of UDP lab session
				proxy.deliveryManager.processTimedOutPackets();
			}
		}
	}
}

void ModuleNetworkingServer::onConnectionReset(const sockaddr_in & fromAddress)
{
	// Find the client proxy
	ClientProxy *proxy = getClientProxy(fromAddress);

	if (proxy)
	{
		// Clear the client proxy
		destroyClientProxy(proxy);
	}
}

void ModuleNetworkingServer::onDisconnect()
{
	uint16 netGameObjectsCount;
	GameObject *netGameObjects[MAX_NETWORK_OBJECTS];
	App->modLinkingContext->getNetworkGameObjects(netGameObjects, &netGameObjectsCount);
	for (uint32 i = 0; i < netGameObjectsCount; ++i)
	{
		NetworkDestroy(netGameObjects[i]);
	}

	for (ClientProxy &clientProxy : clientProxies)
	{
		destroyClientProxy(&clientProxy);
	}
	
	nextClientId = 0;

	state = ServerState::Stopped;
}



//////////////////////////////////////////////////////////////////////
// Client proxies
//////////////////////////////////////////////////////////////////////

ModuleNetworkingServer::ClientProxy * ModuleNetworkingServer::createClientProxy()
{
	// If it does not exist, pick an empty entry
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (!clientProxies[i].connected)
		{
			clientProxies[i].lastPacketReceivedTime = Time.time;
			return &clientProxies[i];
		}
	}

	return nullptr;
}

ModuleNetworkingServer::ClientProxy * ModuleNetworkingServer::getClientProxy(const sockaddr_in &clientAddress)
{
	// Try to find the client
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (clientProxies[i].address.sin_addr.S_un.S_addr == clientAddress.sin_addr.S_un.S_addr &&
			clientProxies[i].address.sin_port == clientAddress.sin_port)
		{
			return &clientProxies[i];
		}
	}

	return nullptr;
}

void ModuleNetworkingServer::destroyClientProxy(ClientProxy *clientProxy)
{
	// Destroy the object from all clients
	if (IsValid(clientProxy->gameObject))
	{
		destroyNetworkObject(clientProxy->gameObject);
	}

    *clientProxy = {};
}

ModuleNetworkingServer::ClientProxy* ModuleNetworkingServer::getClientProxy(int clientIndex)
{
	return &clientProxies[clientIndex];
}


//////////////////////////////////////////////////////////////////////
// Spawning
//////////////////////////////////////////////////////////////////////

GameObject * ModuleNetworkingServer::spawnPlayer(uint8 spaceshipType, vec2 initialPosition, float initialAngle)
{
	// Create a new game object with the player properties
	GameObject *gameObject = NetworkInstantiate();
	gameObject->position = initialPosition;
	gameObject->size = { 100, 100 };
	gameObject->angle = initialAngle;

	// Create sprite
	gameObject->sprite = App->modRender->addSprite(gameObject);
	gameObject->sprite->texture = App->modResources->knightIdleImg;
	gameObject->sprite->order = 5;
	gameObject->animation = App->modRender->addAnimation(gameObject);
	gameObject->animation->clip = App->modResources->knightIdleClip;

	// Create collider
	gameObject->collider = App->modCollision->addCollider(ColliderType::Player, gameObject);
	gameObject->collider->isTrigger = true; // NOTE(jesus): This object will receive onCollisionTriggered events

	// Create behaviour
	Spaceship * spaceshipBehaviour = App->modBehaviour->addSpaceship(gameObject);
	gameObject->behaviour = spaceshipBehaviour;
	gameObject->behaviour->isServer = true;

	return gameObject;
}


//////////////////////////////////////////////////////////////////////
// Update / destruction
//////////////////////////////////////////////////////////////////////

GameObject * ModuleNetworkingServer::instantiateNetworkObject()
{
	// Create an object into the server
	GameObject * gameObject = Instantiate();

	// Register the object into the linking context
	App->modLinkingContext->registerNetworkGameObject(gameObject);

	// Notify all client proxies' replication manager to create the object remotely
	for (ClientProxy &proxy : clientProxies)
	{
		if (proxy.connected)
		{
			// TODO(you): World state replication lab session
			proxy.replicationManager.Create(gameObject->networkId);
		}
	}

	return gameObject;
}

void ModuleNetworkingServer::updateNetworkObject(GameObject * gameObject)
{
	// Notify all client proxies' replication manager to destroy the object remotely
	for (ClientProxy &proxy : clientProxies)
	{
		if (proxy.connected
			&& !proxy.replicationManager.HasReplicationCommmand(ReplicationAction::Update, gameObject->networkId))
		{
			// TODO(you): World state replication lab session
			proxy.replicationManager.Update(gameObject->networkId);
		}
	}
}

void ModuleNetworkingServer::destroyNetworkObject(GameObject * gameObject)
{
	// Notify all client proxies' replication manager to destroy the object remotely
	for (ClientProxy &proxy : clientProxies)
	{
		if (proxy.connected)
		{
			proxy.replicationManager.Destroy(gameObject->networkId);
		}
	}

	// Assuming the message was received, unregister the network identity
	App->modLinkingContext->unregisterNetworkGameObject(gameObject);

	// Finally, destroy the object from the server
	Destroy(gameObject);
}

void ModuleNetworkingServer::destroyNetworkObject(GameObject * gameObject, float delaySeconds)
{
	uint32 emptyIndex = MAX_GAME_OBJECTS;
	for (uint32 i = 0; i < MAX_GAME_OBJECTS; ++i)
	{
		if (netGameObjectsToDestroyWithDelay[i].object == gameObject)
		{
			float currentDelaySeconds = netGameObjectsToDestroyWithDelay[i].delaySeconds;
			netGameObjectsToDestroyWithDelay[i].delaySeconds = min(currentDelaySeconds, delaySeconds);
			return;
		}
		else if (netGameObjectsToDestroyWithDelay[i].object == nullptr)
		{
			if (emptyIndex == MAX_GAME_OBJECTS)
			{
				emptyIndex = i;
			}
		}
	}

	ASSERT(emptyIndex < MAX_GAME_OBJECTS);

	netGameObjectsToDestroyWithDelay[emptyIndex].object = gameObject;
	netGameObjectsToDestroyWithDelay[emptyIndex].delaySeconds = delaySeconds;
}


//////////////////////////////////////////////////////////////////////
// Global create / update / destruction of network game objects
//////////////////////////////////////////////////////////////////////

GameObject * NetworkInstantiate()
{
	ASSERT(App->modNetServer->isConnected());

	return App->modNetServer->instantiateNetworkObject();
}

void NetworkUpdate(GameObject * gameObject)
{
	ASSERT(App->modNetServer->isConnected());
	ASSERT(gameObject->networkId != 0);

	App->modNetServer->updateNetworkObject(gameObject);
}

void NetworkDestroy(GameObject * gameObject)
{
	NetworkDestroy(gameObject, 0.0f);
}

void NetworkDestroy(GameObject * gameObject, float delaySeconds)
{
	ASSERT(App->modNetServer->isConnected());
	ASSERT(gameObject->networkId != 0);

	App->modNetServer->destroyNetworkObject(gameObject, delaySeconds);
}
