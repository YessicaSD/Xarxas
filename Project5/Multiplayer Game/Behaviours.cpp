#include "Networks.h"
#include "Behaviours.h"



void Laser::start()
{
	gameObject->networkInterpolationEnabled = false;

	App->modSound->playAudioClip(App->modResources->audioClipLaser);
}

void Laser::update()
{
	secondsSinceCreation += Time.deltaTime;

	const float pixelsPerSecond = 1000.0f;
	vec2 direction = vec2FromDegrees(gameObject->angle);
	vec2 winPos = App->modRender->WorldToScreen({ 635,480 });
	//LOG("%f", winPos.y);
	if (gameObject->position.y >= 400 || gameObject->position.y <= -400)
	{
		direction.y = -direction.y;
		gameObject->angle = (atan2(direction.y, direction.x)  * RADTODEGREE) + 90.0F;

	}

	if (gameObject->position.x >= 600 || gameObject->position.x <= -600)
	{
		direction.x = -direction.x;
		gameObject->angle = (atan2(direction.y, direction.x) * RADTODEGREE) + 90.0F;

	}
	gameObject->position += direction * pixelsPerSecond * Time.deltaTime;

	if (isServer)
	{
		const float neutralTimeSeconds = 0.1f;
		if (secondsSinceCreation > neutralTimeSeconds && gameObject->collider == nullptr) {
			gameObject->collider = App->modCollision->addCollider(ColliderType::Laser, gameObject);
			gameObject->collider->isTrigger = true;
		}

		const float lifetimeSeconds = 2.0f;
		if (secondsSinceCreation >= lifetimeSeconds) {
			LOG("destroying laser");
			NetworkDestroy(gameObject);
		}
	}
}


void Spaceship::start()
{
	if (!started) {
		lifebar = Instantiate();
		lifebar->sprite = App->modRender->addSprite(lifebar);
		lifebar->sprite->pivot = vec2{ 0.0f, 0.5f };
		lifebar->sprite->order = 5;
		gameObject->angle = 0;
		UpdateLifebar();

		weapon = Instantiate();
		weapon->sprite = App->modRender->addSprite(weapon);
		const float armDistance = -0.65f;
		weapon->sprite->pivot = vec2{ 0.5f, armDistance };
		vec2 bgSize = App->modResources->knightArm->size;
		float spriteHeight = 50.f;
		weapon->size = { -(spriteHeight / bgSize.y) * bgSize.x , spriteHeight };
		weapon->sprite->texture = App->modResources->knightArm;
		weapon->sprite->order = 6;

		started = true;
	}
}

void Spaceship::onInput(const InputController &input, const MouseController & mouseInput)
{
	if (!gameObject->active)
		return;

	if (input.horizontalAxis != 0.0f || input.verticalAxis != 0.0f)
	{
		/*const float rotateSpeed = 180.0f;
		gameObject->angle += input.horizontalAxis * rotateSpeed * Time.deltaTime;*/
		
		const float advanceSpeed = 200.0f;
		gameObject->position.x += input.horizontalAxis * advanceSpeed * Time.deltaTime;
		gameObject->position.y -= input.verticalAxis * advanceSpeed * Time.deltaTime;

		Flip(input.horizontalAxis);

		if (isServer)
		{
			NetworkUpdate(gameObject);
		}
	}

	float weaponAngle = atan2(mouseInput.worldY - weapon->position.y, mouseInput.worldX - weapon->position.x) * (180.f / PI) - 90.f;
	SortWeapon();
	if (weapon->angle != weaponAngle)
	{
		weapon->angle = weaponAngle;
		if (isServer) {
			NetworkUpdate(gameObject);
		}
	}

	if (mouseInput.buttons[0] == ButtonState::Press)
	{
		if (isServer)
		{
			GameObject *laser = NetworkInstantiate();

			laser->position = weapon->position;
			laser->angle = weapon->angle + 180;
			laser->size = { 12, 80 };

			laser->sprite = App->modRender->addSprite(laser);
			laser->sprite->pivot = vec2{ 0.5, 1.f };
			laser->sprite->order = 3;
			laser->sprite->texture = App->modResources->laser;
			laser->animation = App->modRender->addAnimation(laser);
			laser->animation->clip = App->modResources->laserClip;

			Laser *laserBehaviour = App->modBehaviour->addLaser(laser);
			laserBehaviour->isServer = isServer;

			laser->tag = gameObject->networkId;
		}
	}
}

void Spaceship::SortWeapon()
{
	if (weapon->size.x > 0.f) {
		weapon->sprite->order = (weapon->angle < -30.f && weapon->angle > -150.f ? 6 : 4);
	}
	else {
		weapon->sprite->order = (weapon->angle > 30.f || weapon->angle < -210.f ? 6 : 4);
	}
}

void Spaceship::Flip(const float horizontalAxis)
{
	if (horizontalAxis > 0.f) {
		gameObject->size.x = -abs(gameObject->size.x);
		weapon->size.x = abs(weapon->size.x);
	}
	else if (horizontalAxis < 0.f) {
		gameObject->size.x = abs(gameObject->size.x);
		weapon->size.x = -abs(weapon->size.x);
	}
}

void Spaceship::update()
{
	lifebar->position = gameObject->position + vec2{ -50.0f, -50.0f };
	const vec2 shoulderPos = vec2{ 17.5f * sign(weapon->size.x), -14.f };
	weapon->position = gameObject->position + shoulderPos;
	if (isServer)
	{
		if (!gameObject->active && deadLapse < Time.time)
		{
			gameObject->active = true;
			if (gameObject->collider == nullptr)
			{
				gameObject->collider = App->modCollision->addCollider(ColliderType::Player, gameObject);
				gameObject->collider->isTrigger = true;
			}
			hitPoints = MAX_HIT_POINTS;
			UpdateLifebar();
			NetworkUpdate(gameObject);
		}
	}
}

void Spaceship::UpdateLifebar()
{
	static const vec4 colorAlive = vec4{ 0.2f, 1.0f, 0.1f, 0.5f };
	static const vec4 colorDead = vec4{ 1.0f, 0.2f, 0.1f, 0.5f };
	const float lifeRatio = max(0.01f, (float)(hitPoints) / (MAX_HIT_POINTS));
	lifebar->size = vec2{ lifeRatio * 80.0f, 5.0f };
	lifebar->sprite->color = lerp(colorDead, colorAlive, lifeRatio);
}

void Spaceship::destroy()
{
	Destroy(lifebar);
	Destroy(weapon);
}

void Spaceship::onCollisionTriggered(Collider &c1, Collider &c2)
{
	if (c2.type == ColliderType::Laser)
	{
		if (isServer)
		{
			NetworkDestroy(c2.gameObject); // Destroy the laser
		
			if (hitPoints > 0)
			{
				hitPoints--;
			}

			float size = 30 + 50.0f * Random.next();
			vec2 position = gameObject->position + 50.0f * vec2{Random.next() - 0.5f, Random.next() - 0.5f};

			if (hitPoints <= 0)
			{
				// Centered big explosion
				size = 250.0f + 100.0f * Random.next();
				position = gameObject->position;
				gameObject->active = false;
				weapon->active = false;
				if (gameObject->collider != nullptr)
				{
					App->modCollision->removeCollider(gameObject->collider);
					gameObject->collider = nullptr;
				}
				
			
				deadLapse = Time.time + 0.5;
				//NetworkDestroy(gameObject);
			}
			NetworkUpdate(gameObject);

			GameObject *explosion = NetworkInstantiate();
			explosion->position = position;
			explosion->size = vec2{ size, size };
			explosion->angle = 365.0f * Random.next();

			explosion->sprite = App->modRender->addSprite(explosion);
			explosion->sprite->texture = App->modResources->explosion1;
			explosion->sprite->order = 100;

			explosion->animation = App->modRender->addAnimation(explosion);
			explosion->animation->clip = App->modResources->explosionClip;

			NetworkDestroy(explosion, 2.0f);

			// NOTE(jesus): Only played in the server right now...
			// You need to somehow make this happen in clients
			App->modSound->playAudioClip(App->modResources->audioClipExplosion);
		}
	}
}

void Spaceship::write(OutputMemoryStream & packet)
{
	Behaviour::write(packet);
	packet << weapon->angle;
	packet << hitPoints;
}

void Spaceship::read(const InputMemoryStream & packet, uint32 lastInputReceived)
{
	start();

	vec2 server_pos;
	float server_angle;
	uint8 hitPoints;
	bool active = true;
	packet >> server_pos;
	packet >> server_angle;
	packet >> active;
	packet >> weapon->angle;
	gameObject->active = active;
	if(weapon)
		weapon->active = active;
	if (!active)
	{
		if (gameObject->collider != nullptr)
		{
			App->modCollision->removeCollider(gameObject->collider);
			gameObject->collider = nullptr;
		}
	}
	else if(gameObject->collider == nullptr)
	{
		gameObject->collider = App->modCollision->addCollider(ColliderType::Player,gameObject);
		gameObject->collider->isTrigger = true;
	}

	packet >> hitPoints;

	if (lifebar != nullptr && hitPoints != this->hitPoints)
	{
		this->hitPoints = hitPoints;
		UpdateLifebar();
	
	}

	Flip(server_pos.x - gameObject->position.x);
	SortWeapon();

	if (gameObject->networkId == App->modNetClient->getNetworkId())
	{
		gameObject->position = server_pos;
		gameObject->angle = server_angle;
		for (int i = lastInputReceived + 1; i < App->modNetClient->inputIndex; i++)
		{
			App->modNetClient->ProcessInput(i, gameObject);
		}
	}

	if (gameObject->interpolation != nullptr)
	{
		gameObject->interpolation->SetFinal(server_pos, server_angle);
	}

}
