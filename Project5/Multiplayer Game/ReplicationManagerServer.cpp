#include "Networks.h"
#include "ReplicationManagerServer.h"
#include "DeliveryManager.h";

// TODO(you): World state replication lab session

void ReplicationManagerServer::Create(uint32 networkId)
{
	replicationCommands.push_back(ReplicationCommand(ReplicationAction::Create, networkId));
}

void ReplicationManagerServer::Update(uint32 networkId)
{
	replicationCommands.push_back(ReplicationCommand(ReplicationAction::Update, networkId));
}

void ReplicationManagerServer::Destroy(uint32 networkId)
{
	replicationCommands.push_back(ReplicationCommand(ReplicationAction::Destroy, networkId));
}

void ReplicationManagerServer::Write(OutputMemoryStream& packet, DeliveryManagerServer* deliveryManager, std::vector<ReplicationCommand>& commands)
{
	packet << PROTOCOL_ID;
	packet << ServerMessage::Replication;

	Delivery* delivery = deliveryManager->writeSequenceNumber(packet);

	packet << lastClientInputReceived;

	for (ReplicationCommand command : commands) {
		switch (command.action) {
		case ReplicationAction::Create: {
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(command.networkId);
			if (gameObject != nullptr) {
				packet << command.networkId;
				packet << command.action;

				BehaviourType behaviour = gameObject->behaviour == nullptr ? BehaviourType::None : gameObject->behaviour->type();
				packet << behaviour;
				packet << gameObject->position;
				packet << gameObject->angle;
				packet << gameObject->size;
				packet << gameObject->tag;

				if (gameObject->sprite->texture != nullptr)
					packet << std::string(gameObject->sprite->texture->filename);
				else
					packet << std::string("noTexture");

				delivery->indispensableCommands.push_back(command);
			}
		}break;
		case ReplicationAction::Update: {
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(command.networkId);
			if (gameObject != nullptr) {
				packet << command.networkId;
				packet << command.action;

				BehaviourType behaviour = gameObject->behaviour == nullptr ? BehaviourType::None : gameObject->behaviour->type();
				packet << behaviour;
				if (gameObject->behaviour == nullptr) {
					packet << gameObject->position;
					packet << gameObject->angle;
					packet << gameObject->active;
				}
				else {
					gameObject->behaviour->write(packet);
				}
			}
		}break;
		case ReplicationAction::Destroy: {
			packet << command.networkId;
			packet << command.action;

			//If we're destroying, we want to make sure that that packet gets sent
			delivery->indispensableCommands.push_back(command);
			break;
		}
		}
	}
	commands.clear();
}

bool ReplicationManagerServer::HasReplicationCommmand(ReplicationAction action, uint32 networkId)
{
	for (ReplicationCommand command : replicationCommands)
	{
		if (command.action == action
			&& command.networkId == networkId)
		{
			return true;
		}
	}
	return false;
}