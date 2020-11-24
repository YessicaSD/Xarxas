#include "Networks.h"

// TODO(you): World state replication lab session

enum class ReplicationAction
{
	None, Create, Update, Destroy
};

struct ReplicationCommand
{
	ReplicationAction action;
	uint32 networkId;
};

class ReplicationManagerServer
{
public:
	void Create(uint32 networking);
	void Update(uint32 networking);
	void Destroy(uint32 networking);

	void Write(OutputMemoryStream& packet);

	std::vector<ReplicationCommand> commands;
};