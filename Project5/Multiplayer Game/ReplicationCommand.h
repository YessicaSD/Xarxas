#pragma once

// TODO(you): World state replication lab session
enum class ReplicationAction
{
	None, Create, Update, Destroy
};

struct ReplicationCommand
{
	ReplicationAction action;
	uint32 networkId;

	ReplicationCommand(ReplicationAction action, uint32 networkId) : 
		action(action), networkId(networkId)
	{
	}
};