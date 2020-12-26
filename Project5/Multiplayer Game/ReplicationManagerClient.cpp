#include "Networks.h"
#include "ReplicationManagerClient.h"
#include "DeliveryManager.h"
//#include "Application.h"


// TODO(you): World state replication lab session
void ReplicationManagerClient::Read(const InputMemoryStream& packet, DeliveryManagerClient * deliveryManager)
{
	bool processPacket = deliveryManager->processSequenceNumber(packet);

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
				App->modLinkingContext->unregisterNetworkGameObject(obj);
				Destroy(obj);
			} break;

			case ReplicationAction::Create: 
			{
				instantiateGameObject(command.networkId, packet, processPacket);
<<<<<<< HEAD
				//TODO JAUME: Put gameObject->isLocalPlayer to true when it's your spaceship
=======
>>>>>>> be643306649ddcf2583b7fc009a8f1b2da6a866c
			}
			break;
			case ReplicationAction::Update:
			{
				GameObject helperObj;
				GameObject* obj = (processPacket) ? App->modLinkingContext->getNetworkGameObject(command.networkId) : &helperObj;
			
				if (obj != nullptr)
				{
					if (obj->interpolation != nullptr)
					{
						obj->interpolation->doLerp = true;
						obj->interpolation->initial_position = obj->position;
						obj->interpolation->initial_angle = obj->angle;

						packet >> obj->interpolation->final_position;
						packet >> obj->interpolation->final_angle;
					}
					else
					{
						packet >> obj->position;
						packet >> obj->angle;
					}
					
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
	GameObject helperGO;
	if (processCommand)
	{
		gameObject = App->modGameObject->Instantiate();
		App->modLinkingContext->registerNetworkGameObjectWithNetworkId(gameObject, networkId);
	}
	else
	{
		gameObject = &helperGO;
	}

	BehaviourType behaviour;
	packet >> behaviour;
	packet >> gameObject->position;
	packet >> gameObject->angle;
	packet >> gameObject->size;
	std::string texture_filename;
	packet >> texture_filename;

	if (processCommand)
	{
		switch (behaviour) {
		case BehaviourType::Spaceship:
		{
			gameObject->behaviour = App->modBehaviour->addSpaceship(gameObject);
<<<<<<< HEAD
			if (processCommand && gameObject->interpolation == nullptr)
			{
				 //gameObject->interpolation = (Interpolation*) App->modComponent->GetComponent<Interpolation>(gameObject);
			}
=======
			packet >> gameObject->behaviour->isLocalPlayer;
>>>>>>> be643306649ddcf2583b7fc009a8f1b2da6a866c

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
}