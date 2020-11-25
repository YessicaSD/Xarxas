#include "Networks.h"
#include "ReplicationManagerClient.h"

// TODO(you): World state replication lab session
void ReplicationManagerClient::Read(const InputMemoryStream& packet)
{

	while (packet.RemainingByteCount() != 0)
	{
		ReplicationCommand command;
		packet >> command.networkId;
		packet >> command.action;

		switch (command.action)
		{
		case ReplicationAction::Destroy: {
			GameObject* obj = App->modLinkingContext->getNetworkGameObject(command.networkId);
			Destroy(obj);
		} break;
		case ReplicationAction::Create: {
			//TODO JAUME: Differentiate between different objects
			instantiatePlayerGameObject(command.networkId, packet);
			//TODO JAUME: Put gameObject->isLocalPlayer to true when it's your spaceship
		} break;
		case ReplicationAction::Update:
		{
		}break;
		}
	}
	
}

void ReplicationManagerClient::instantiatePlayerGameObject(uint32 networkId, const InputMemoryStream& packet) {
	GameObject* gameObject = App->modGameObject->Instantiate();
	App->modLinkingContext->registerNetworkGameObjectWithNetworkId(gameObject, networkId);
	
	packet >> gameObject->position;
	gameObject->size = { 100, 100 };
	packet >> gameObject->angle;

	gameObject->sprite = App->modRender->addSprite(gameObject);
	gameObject->sprite->order = 5;
	std::string texture_filename;
	packet >> texture_filename;
	if (texture_filename == App->modResources->spacecraft1->filename) {
		gameObject->sprite->texture = App->modResources->spacecraft1;
	}
	else if (texture_filename == App->modResources->spacecraft2->filename) {
		gameObject->sprite->texture = App->modResources->spacecraft2;
	}
	else if (texture_filename == App->modResources->spacecraft3->filename) {
		gameObject->sprite->texture = App->modResources->spacecraft3;
	}

	// Create collider
	gameObject->collider = App->modCollision->addCollider(ColliderType::Player, gameObject);
	gameObject->collider->isTrigger = true;

	// Create behaviour
	Spaceship* spaceshipBehaviour = App->modBehaviour->addSpaceship(gameObject);
	gameObject->behaviour = spaceshipBehaviour;
}