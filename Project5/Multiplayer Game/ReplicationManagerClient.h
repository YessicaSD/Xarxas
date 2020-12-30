#pragma once

// TODO(you): World state replication lab session
class ReplicationManagerClient
{
public:
	void Read(const InputMemoryStream &packet, DeliveryManagerClient * deliveryManager);
	void DestroyGameObject(ReplicationCommand& command);
	void UpdateGameObject(bool processPacket, ReplicationCommand& command, const InputMemoryStream& packet, const uint32& server_Input);
	void CreateGameObject(uint32 networkId, const InputMemoryStream &packet, bool processCommand);
};