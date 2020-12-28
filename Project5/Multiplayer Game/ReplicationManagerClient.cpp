#include "Networks.h"
#include "ReplicationManagerClient.h"
#include "DeliveryManager.h"
//#include "Application.h"


// TODO(you): World state replication lab session
void ReplicationManagerClient::Read(const InputMemoryStream& packet, DeliveryManagerClient* deliveryManager)
{
	bool processPacket = deliveryManager->processSequenceNumber(packet);

	uint32 server_Input;
	packet >> server_Input;
	
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
			instantiateGameObject(command.networkId, packet, processPacket);
		}
		break;
		case ReplicationAction::Update:
		{
			GameObject disposableObj;
			GameObject* obj = (processPacket) ? App->modLinkingContext->getNetworkGameObject(command.networkId) : &disposableObj;
				
			vec2 server_pos;
			float server_angle;
			packet >> server_pos;
			packet >> server_angle;

			if (command.networkId == App->modNetClient->getNetworkId())
			{
				obj->position = server_pos;
				obj->angle = server_angle;
				for (int i = server_Input; i < App->modNetClient->inputIndex; i++)
				{
					App->modNetClient->ProcessInput(i, obj);
				}
			}

			if (obj->interpolation != nullptr)
			{
				obj->interpolation->SetFinal(server_pos, server_angle);
			}

		}
		break;
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
	
	gameObject  = App->modLinkingContext->getNetworkGameObject(networkId);

	// If the object exists this is may mean that the object have been delated
	if (gameObject != nullptr)
	{
		App->modLinkingContext->unregisterNetworkGameObject(gameObject);
	}

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
	//TODO: If we're not using the texture anymore, change it
	std::string texture_filename;
	packet >> texture_filename;

	switch (behaviour) {
	case BehaviourType::Spaceship:
	{
		gameObject->behaviour = App->modBehaviour->addSpaceship(gameObject);
		
		gameObject->behaviour->isLocalPlayer = (networkId == App->modNetClient->getNetworkId());
		if (processCommand && gameObject->interpolation == nullptr && !gameObject->behaviour->isLocalPlayer)
		{
			gameObject->interpolation = (Interpolation*)App->modComponent->GetComponent<Interpolation>(gameObject);
		}

		gameObject->sprite = App->modRender->addSprite(gameObject);
		gameObject->sprite->order = 5;
		gameObject->sprite->texture = App->modResources->knightIdleImg;
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
		gameObject->collider = App->modCollision->addCollider(ColliderType::Laser, gameObject);
		gameObject->collider->isTrigger = true;
	} break;
	case BehaviourType::None:
	{
		if (processCommand)
		{
			gameObject->sprite = App->modRender->addSprite(gameObject);
			gameObject->sprite->order = 5;
			gameObject->sprite->texture = App->modResources->knightArm;
		}	
	}
		
	break;
	default:
	{

	} break;
	}
}