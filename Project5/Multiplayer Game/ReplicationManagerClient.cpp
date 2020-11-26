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
				App->modLinkingContext->unregisterNetworkGameObject(obj);
				Destroy(obj);
			} break;
			case ReplicationAction::Create: {
				//TODO JAUME: Differentiate between different objects
				instantiateGameObject(command.networkId, packet);
				//TODO JAUME: Put gameObject->isLocalPlayer to true when it's your spaceship
			} break;
			case ReplicationAction::Update:
			{
				GameObject* obj = App->modLinkingContext->getNetworkGameObject(command.networkId);
				if (obj)
				{
					packet >> obj->position;
					packet >> obj->angle;
				}
			}break;
		}
	}
	
}

void ReplicationManagerClient::instantiateGameObject(uint32 networkId, const InputMemoryStream& packet) {
	GameObject* gameObject = App->modGameObject->Instantiate();
	App->modLinkingContext->registerNetworkGameObjectWithNetworkId(gameObject, networkId);
	
	packet >> gameObject->position;
	packet >> gameObject->angle;
	packet >> gameObject->size;

	gameObject->sprite = App->modRender->addSprite(gameObject);
	gameObject->sprite->order = 5;
	std::string texture_filename;
	packet >> texture_filename;
	bool isPlayer = true;
	if (texture_filename == App->modResources->spacecraft1->filename) {
		gameObject->sprite->texture = App->modResources->spacecraft1;
	}
	else if (texture_filename == App->modResources->spacecraft2->filename) {
		gameObject->sprite->texture = App->modResources->spacecraft2;
	}
	else if (texture_filename == App->modResources->spacecraft3->filename) {
		gameObject->sprite->texture = App->modResources->spacecraft3;
	}
	else if (texture_filename == App->modResources->laser->filename) {
		isPlayer = false;
		gameObject->sprite->texture = App->modResources->laser;
	}
	// Create collider
	gameObject->collider = App->modCollision->addCollider(ColliderType::Player, gameObject);
	gameObject->collider->isTrigger = true;

	if (isPlayer)
	{
		// Create behaviour
		Spaceship* spaceshipBehaviour = App->modBehaviour->addSpaceship(gameObject);
		gameObject->behaviour = spaceshipBehaviour;
	}
	else
	{
		// Create behaviour
		Laser* LaserBehaviour = App->modBehaviour->addLaser(gameObject);
		gameObject->behaviour = LaserBehaviour;
	}
}