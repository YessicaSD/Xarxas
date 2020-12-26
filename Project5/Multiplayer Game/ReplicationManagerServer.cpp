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

void ReplicationManagerServer::Write(OutputMemoryStream& packet, DeliveryManagerServer* deliveryManager, std::vector<ReplicationCommand> &commands, uint32 playerNetworkId)
{
	packet << PROTOCOL_ID;
	packet << ServerMessage::Replication;

	Delivery * newDelivery = deliveryManager->writeSequenceNumber(packet);

	for (ReplicationCommand command : commands) {
		packet << command.networkId;
		packet << command.action;

		switch (command.action) {
			case ReplicationAction::Create: {
				GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(command.networkId);
				BehaviourType behaviour = gameObject->behaviour == nullptr ? BehaviourType::None : gameObject->behaviour->type();
				packet << behaviour;
				packet << gameObject->position;
				packet << gameObject->angle;
				packet << gameObject->size;
				packet << std::string(gameObject->sprite->texture->filename);
				switch (behaviour) {
					case BehaviourType::Spaceship: {
						//Is isLocalPlayer true on the client?
						packet << (gameObject->networkId == playerNetworkId);
					} break;
					default: {

					} break;
				}
				}break;
			case ReplicationAction::Update:{
				GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(command.networkId);
				if (gameObject != nullptr) {
					packet << gameObject->position;
					packet << gameObject->angle;
				}
				}break;
			case ReplicationAction::Destroy: {
				//If we're destroying, we want to make sure that that packet gets sent
				newDelivery->indispensableCommands.push_back(command);
				if (newDelivery->delegate == nullptr) {
					newDelivery->delegate = new DeliveryDelegateDestroy(newDelivery);
				}
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
