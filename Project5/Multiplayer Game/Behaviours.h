#pragma once


enum class BehaviourType : uint8;

struct Behaviour
{
	GameObject *gameObject = nullptr;

	bool isServer = false;
	bool isLocalPlayer = false;

	virtual BehaviourType type() const = 0;

	virtual void start() { }

	virtual void onInput(const InputController &input, const MouseController &mouseInput) { }

	virtual void update() { }

	virtual void destroy() { }

	virtual void onCollisionTriggered(Collider &c1, Collider &c2) { }

	virtual void write(OutputMemoryStream &packet)
	{
		packet << gameObject->position;
		packet << gameObject->angle;
	}

	virtual void read(const InputMemoryStream &packet, uint32 lastInputReceived)
	{
		packet >> gameObject->position;
		packet >> gameObject->angle;
	}
};


enum class BehaviourType : uint8
{
	None,
	Spaceship,
	Laser,
};


struct Laser : public Behaviour
{
	float secondsSinceCreation = 0.0f;

	BehaviourType type() const override { return BehaviourType::Laser; }

	void start() override;

	void update() override;
};


struct Spaceship : public Behaviour
{
	static const uint8 MAX_HIT_POINTS = 5;
	uint8 hitPoints = MAX_HIT_POINTS;

	GameObject* weapon = nullptr;
	GameObject *lifebar = nullptr;

	BehaviourType type() const override { return BehaviourType::Spaceship; }

	void start() override;

	void onInput(const InputController &input, const MouseController & mouseInput) override;

	void update() override;

	void UpdateLifebar();

	void destroy() override;

	void onCollisionTriggered(Collider &c1, Collider &c2) override;

	void write(OutputMemoryStream &packet) override;

	void read(const InputMemoryStream &packet, uint32 lastInputReceived) override;
};
