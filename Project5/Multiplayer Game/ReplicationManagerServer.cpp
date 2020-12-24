#include "Networks.h"
#include "ReplicationManagerServer.h"
#include "DeliveryManager.h";

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

void ReplicationManagerServer::Write(OutputMemoryStream& packet, DeliveryManagerServer* deliveryManager)
{
	packet << PROTOCOL_ID;
	packet << ServerMessage::Replication;
	

	Delivery * newDelivery = deliveryManager->writeSequenceNumber(packet);
	

	//TODO JAUME: Register callbacks onto the next delivery

	for (ReplicationCommand command : commands) {
		packet << command.networkId;
		packet << command.action;

		switch (command.action) {
			case ReplicationAction::Create: {
				GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(command.networkId);
				packet << gameObject->behaviour->type();
				packet << gameObject->position;
				packet << gameObject->angle;
				packet << gameObject->size;
				packet << std::string(gameObject->sprite->texture->filename);
				}break;
			case ReplicationAction::Update:{
				GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(command.networkId);
				if (gameObject != nullptr) {
					packet << gameObject->position;
					packet << gameObject->angle;
				}
				}break;
		}
	}
	commands.clear();
}