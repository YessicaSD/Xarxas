#pragma once

// TODO(you): World state replication lab session
class ReplicationManagerServer
{
public:
	void Create(uint32 networkId);
	void Update(uint32 networkId);
	void Destroy(uint32 networkId);

	void Write(OutputMemoryStream& packet);

	std::vector<ReplicationCommand> commands;
};