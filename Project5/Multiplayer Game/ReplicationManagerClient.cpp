#include "Networks.h"
#include "ReplicationManagerClient.h"
#include "DeliveryManager.h"
//#include "Application.h"


// TODO(you): World state replication lab session
void ReplicationManagerClient::Read(const InputMemoryStream& packet, DeliveryManagerClient* deliveryManager)
{
	bool inOrder = deliveryManager->processSequenceNumber(packet);

	uint32 lastInputReceived;
	packet >> lastInputReceived;
	
	while (packet.RemainingByteCount() != 0)
	{
		ReplicationCommand command;
		packet >> command.networkId;
		packet >> command.action;

		switch (command.action)
		{
		case ReplicationAction::Destroy:
		{
			GameObject* obj = App->modLinkingContext->getNetworkGameObject(command.networkId);
			if (obj != nullptr)
			{
				App->modLinkingContext->unregisterNetworkGameObject(obj);
				Destroy(obj);
			}
			
		} break;
		case ReplicationAction::Create:
		{
			CreateGameObject(command.networkId, packet, inOrder);
		}
		break;
		case ReplicationAction::Update:
		{
			UpdateGameObject(inOrder, command, packet, lastInputReceived);
		}
		break;
		default: {
			LOG("Invalid case");
		} break;
		}
	}

}

void ReplicationManagerClient::UpdateGameObject(bool inOrder, ReplicationCommand& command, const InputMemoryStream& packet, const uint32& lastInputReceived)
{
	GameObject disposableObj;
	GameObject* obj = nullptr;
	if (inOrder) {
		obj = App->modLinkingContext->getNetworkGameObject(command.networkId);
		//If it is nullptr, it there was a packet loss where we created this gameobject
		if (obj == nullptr) {
			obj = App->modGameObject->Instantiate();
			App->modLinkingContext->registerNetworkGameObjectWithNetworkId(obj, command.networkId);
		}
	}
	else {
		GameObject disposableObj;
		obj = &disposableObj;
	}

	BehaviourType behaviour;
	packet >> behaviour;

	if (obj->behaviour == nullptr) {
		obj->behaviour = App->modBehaviour->addBehaviour(behaviour, obj);
	}

	if (obj->behaviour == nullptr) {
		packet >> obj->position;
		packet >> obj->angle;
	}
	else {
		obj->behaviour->read(packet, lastInputReceived);
	}
}

void ReplicationManagerClient::CreateGameObject(uint32 networkId, const InputMemoryStream& packet, bool inOrder)
{
	//TODO JAUME: Put gameObject->isLocalPlayer to true when it's your spaceship

	GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkId);

	// If the GameObject exists this is may mean we've lost a Destroy packet
	if (gameObject != nullptr)
	{
		App->modLinkingContext->unregisterNetworkGameObject(gameObject);
		Destroy(gameObject);
	}

	if (inOrder)
	{
		gameObject = App->modGameObject->Instantiate();
		App->modLinkingContext->registerNetworkGameObjectWithNetworkId(gameObject, networkId);
	}
	else
	{
		GameObject disposableGObj;
		gameObject = &disposableGObj;
	}

	BehaviourType behaviour;
	packet >> behaviour;
	packet >> gameObject->position;
	packet >> gameObject->angle;
	packet >> gameObject->size;
	//TODO: If we're not using the texture anymore, change it
	std::string texture_filename;
	packet >> texture_filename;

	switch (behaviour) {
	case BehaviourType::Spaceship:
	{
		gameObject->behaviour = App->modBehaviour->addSpaceship(gameObject);
		
		gameObject->behaviour->isLocalPlayer = (networkId == App->modNetClient->getNetworkId());
		if (inOrder && gameObject->interpolation == nullptr && !gameObject->behaviour->isLocalPlayer)
		{
			gameObject->interpolation = (Interpolation*)App->modComponent->GetComponent<Interpolation>(gameObject);
		}

		gameObject->sprite = App->modRender->addSprite(gameObject);
		gameObject->sprite->order = 5;
		gameObject->sprite->texture = App->modResources->knightIdleImg;
		gameObject->sprite->pivot = vec2{ 0.25f, 0.5f };
		gameObject->collider = App->modCollision->addCollider(ColliderType::Player, gameObject);
		gameObject->collider->isTrigger = true;

		gameObject->animation = App->modRender->addAnimation(gameObject);
		gameObject->animation->clip = App->modResources->knightIdleClip;
	} break;
	case BehaviourType::Laser:
	{
		gameObject->behaviour = App->modBehaviour->addLaser(gameObject);
		gameObject->sprite = App->modRender->addSprite(gameObject);
		gameObject->sprite->order = 5;
		gameObject->sprite->texture = App->modResources->laser;
		gameObject->animation = App->modRender->addAnimation(gameObject);
		gameObject->animation->clip = App->modResources->laserClip;
		gameObject->collider = App->modCollision->addCollider(ColliderType::Laser, gameObject);
		gameObject->collider->isTrigger = true;
	} break;
	case BehaviourType::None:
	{
		if (inOrder)
		{
			gameObject->sprite = App->modRender->addSprite(gameObject);
			gameObject->sprite->order = 5;
			gameObject->sprite->texture = *App->modResources->textures[texture_filename];

			if (texture_filename.compare("explosion1.png") == 0) {
				gameObject->animation = App->modRender->addAnimation(gameObject);
				gameObject->animation->clip = App->modResources->explosionClip;
			}
		}
	}
		
	break;
	default:
	{

	} break;
	}
}