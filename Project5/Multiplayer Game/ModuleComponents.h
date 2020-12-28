#pragma once
#include <vector>
// #include "Module.h"
struct Component
{
	GameObject* owner = nullptr;
	virtual void Update() {};
};

class ModuleComponents : public Module
{
public:
	bool update() override;
	bool cleanUp() override;
	template <class T>
	Component* GetComponent(GameObject* owner);
	void DeleteComponent(Component* component);
private:
	std::vector<Component*> allComponents;
};

struct Interpolation : Component
{
	bool doLerp = false;

	vec2 initial_position = vec2{ 0.0f, 0.0f };
	float initial_angle = 0.0f;

	vec2 final_position = vec2{ 0.0f, 0.0f };
	float final_angle = 0.0f;

	float secondsElapsed = 0.0f;

	Interpolation(GameObject* owner)
	{
		this->owner = owner;
		
		//secondsElapsed = Time.time;
	}
	void SetFinal(vec2 f_pos, float f_angle);
	void Update() override;

};

template<class T>
inline Component* ModuleComponents::GetComponent(GameObject* owner)
{
	Component* newComponent = new T(owner);
	allComponents.push_back(newComponent);
	return newComponent;
}
