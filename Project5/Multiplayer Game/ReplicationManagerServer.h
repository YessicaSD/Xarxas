#pragma once

class DeliveryManagerServer;

// TODO(you): World state replication lab session
class ReplicationManagerServer
{
public:
	void Create(uint32 networkId);
	void Update(uint32 networkId);
	void Destroy(uint32 networkId);

	static void Write(OutputMemoryStream& packet, DeliveryManagerServer* deliveryManager, std::vector<ReplicationCommand> &commands, uint32 playerNetworkId = 0);

	bool HasReplicationCommmand(ReplicationAction action, uint32 networkId);

	std::vector<ReplicationCommand> replicationCommands;
};