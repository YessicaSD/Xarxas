#pragma once

class DeliveryManagerServer;

// TODO(you): World state replication lab session
class ReplicationManagerServer
{
	

public:
	uint32 lastClientInputReceived = 0;

	void Create(uint32 networkId);
	void Update(uint32 networkId);
	void Destroy(uint32 networkId);

	void Write(OutputMemoryStream& packet, DeliveryManagerServer* deliveryManager, std::vector<ReplicationCommand> &commands);

	bool HasReplicationCommmand(ReplicationAction action, uint32 networkId);

	std::vector<ReplicationCommand> replicationCommands;
};