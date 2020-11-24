#pragma once

// TODO(you): World state replication lab session
class ReplicationManagerServer
{
public:
	void Create(uint32 networking);
	void Update(uint32 networking);
	void Destroy(uint32 networking);

	void Write(OutputMemoryStream& packet);

	std::vector<ReplicationCommand> commands;
};