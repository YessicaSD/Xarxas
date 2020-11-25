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
	packet << PROTOCOL_ID;
	packet << ServerMessage::Replication;
	
	for (ReplicationCommand command : commands) {
		packet << command.networkId;
		packet << command.action;

		switch (command.action) {
			case ReplicationAction::Create: {
				GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(command.networkId);
				packet << gameObject->position;
				packet << gameObject->angle;
				packet << std::string(gameObject->sprite->texture->filename);
				//TODO JAUME: Add behaviour (is it a spaceship or a bullet?)
				}break;
			case ReplicationAction::Update:{
					//TODO JAUME: Fill this
				}break;
			case ReplicationAction::None: {
				//Ignored...
				}break;
			case ReplicationAction::Destroy: {

				} break;
		}
	}

	//TODO JAUME: Clear the vector after it's been written
	//Or put all actions to none if we decide to create a plain C array
	//We may not need the "None" case if we're using a vector
}