#include "Networks.h"
#include "ReplicationManagerClient.h"
#include "DeliveryManager.h"

// TODO(you): World state replication lab session
void ReplicationManagerClient::Read(const InputMemoryStream& packet, DeliveryManagerClient * deliveryManager)
{
	bool processPacket = deliveryManager->processSequenceNumber(packet);

	while (packet.RemainingByteCount() != 0)
	{
		ReplicationCommand command;
		packet >> command.networkId;
		packet >> command.action;
		
		GameObject* gObj = App->modLinkingContext->getNetworkGameObject(command.networkId);
		if (gObj != nullptr
			&& gObj->behaviour != nullptr) {
			LOG("behaviour: %i", gObj->behaviour->type());
		}
		else {
			LOG("behaviour: unknown");
		}
		
		LOG("action:    %i", command.action);

		switch (command.action)
		{
			case ReplicationAction::Destroy: 
			{
				GameObject* obj = App->modLinkingContext->getNetworkGameObject(command.networkId);
				App->modLinkingContext->unregisterNetworkGameObject(obj);
				Destroy(obj);
			} break;

			case ReplicationAction::Create: 
			{
				instantiateGameObject(command.networkId, packet, processPacket);
			}
			break;
			case ReplicationAction::Update:
			{
				GameObject helperObj;
				GameObject* obj = (processPacket) ? App->modLinkingContext->getNetworkGameObject(command.networkId) : &helperObj;
			
				ASSERT(obj != nullptr);
				if (obj != nullptr)
				{
					packet >> obj->position;
					packet >> obj->angle;
				}

			}break;
			default: {
				LOG("Invalid case");
			} break;
		}
	}
	
}

void ReplicationManagerClient::instantiateGameObject(uint32 networkId, const InputMemoryStream& packet, bool processCommand) 
{
	//TODO JAUME: Put gameObject->isLocalPlayer to true when it's your spaceship

	GameObject* gameObject = nullptr;
	GameObject disposableGObj;
	if (processCommand)
	{
		gameObject = App->modGameObject->Instantiate();
		App->modLinkingContext->registerNetworkGameObjectWithNetworkId(gameObject, networkId);
	}
	else
	{
		gameObject = &disposableGObj;
	}

	BehaviourType behaviour;
	packet >> behaviour;
	packet >> gameObject->position;
	packet >> gameObject->angle;
	packet >> gameObject->size;
	std::string texture_filename;
	packet >> texture_filename;

	switch (behaviour) {
	case BehaviourType::Spaceship:
	{
		gameObject->behaviour = App->modBehaviour->addSpaceship(gameObject);
		packet >> gameObject->behaviour->isLocalPlayer;

		gameObject->sprite = App->modRender->addSprite(gameObject);
		gameObject->sprite->order = 5;
		if (texture_filename == App->modResources->spacecraft1->filename) {
			gameObject->sprite->texture = App->modResources->spacecraft1;
		}
		else if (texture_filename == App->modResources->spacecraft2->filename) {
			gameObject->sprite->texture = App->modResources->spacecraft2;
		}
		else if (texture_filename == App->modResources->spacecraft3->filename) {
			gameObject->sprite->texture = App->modResources->spacecraft3;
		}
		gameObject->collider = App->modCollision->addCollider(ColliderType::Player, gameObject);
		gameObject->collider->isTrigger = true;
	} break;
	case BehaviourType::Laser:
	{
		gameObject->behaviour = App->modBehaviour->addLaser(gameObject);
		gameObject->sprite = App->modRender->addSprite(gameObject);
		gameObject->sprite->order = 5;
		gameObject->sprite->texture = App->modResources->laser;
		gameObject->collider = App->modCollision->addCollider(ColliderType::Laser, gameObject);
		gameObject->collider->isTrigger = true;
	} break;
	case BehaviourType::None:
	{

	} break;
	default:
	{

	} break;
	}
}