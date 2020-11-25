#include "Networks.h"
#include "ReplicationManagerServer.h"

// TODO(you): World state replication lab session

void ReplicationManagerServer::Create(uint32 networkId)
{
	commands.push_back(ReplicationCommand(ReplicationAction::Create, networkId));
}

void ReplicationManagerServer::Update(uint32 networkId)
{
	commands.push_back(ReplicationCommand(ReplicationAction::Update, networkId));
}

void ReplicationManagerServer::Destroy(uint32 networkId)
{
	commands.push_back(ReplicationCommand(ReplicationAction::Destroy, networkId));
}

void ReplicationManagerServer::Write(OutputMemoryStream& packet)
{
	for (ReplicationCommand command : commands) {
		switch (command.action) {
			case ReplicationAction::Create: {
				GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(command.networkId);
				packet << gameObject->position;
				packet << gameObject->angle;
				//packet << gameObject->
				//sprite
				//behaviour (is it a spaceship or a bullet?)
			}break;
		}
	}

	//INFO: Go through the command vector
	//INFO: Write it
	//INFO: Create a big packet
	//INFO: Send it
}