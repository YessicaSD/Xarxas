#include "Networks.h"
#include "ReplicationManagerClient.h"

// TODO(you): World state replication lab session
void ReplicationManagerClient::read(const InputMemoryStream& packet)
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
		vec2 pos;
		packet >> pos;
		float angle;
		packet >> angle;
		uint8 spaceshipType;
		packet >> spaceshipType;
		instantiatePlayerGameObject(spaceshipType, command.networkId, pos, angle);
		//TODO JAUME: Put gameObject->isLocalPlayer to true when it's your spaceship
	} break;
	}
}

void ReplicationManagerClient::instantiatePlayerGameObject(uint8 spaceshipType, uint32 networkId, vec2 initialPosition, float initialAngle) {
	// Create a new game object with the player properties
	GameObject* gameObject = App->modGameObject->Instantiate();
	App->modLinkingContext->registerNetworkGameObjectWithNetworkId(gameObject, networkId);
	gameObject->position = initialPosition;
	gameObject->size = { 100, 100 };
	gameObject->angle = initialAngle;

	// Create sprite
	gameObject->sprite = App->modRender->addSprite(gameObject);
	gameObject->sprite->order = 5;
	if (spaceshipType == 0) {
		gameObject->sprite->texture = App->modResources->spacecraft1;
	}
	else if (spaceshipType == 1) {
		gameObject->sprite->texture = App->modResources->spacecraft2;
	}
	else {
		gameObject->sprite->texture = App->modResources->spacecraft3;
	}

	// Create collider
	gameObject->collider = App->modCollision->addCollider(ColliderType::Player, gameObject);
	gameObject->collider->isTrigger = true;

	// Create behaviour
	Spaceship* spaceshipBehaviour = App->modBehaviour->addSpaceship(gameObject);
	gameObject->behaviour = spaceshipBehaviour;
}