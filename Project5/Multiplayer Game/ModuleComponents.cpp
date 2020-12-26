#include "Networks.h"
#include "ModuleComponents.h"
#include "Maths.h"

bool ModuleComponents::update()
{
	for (int i = 0; i < allComponents.size(); i++)
	{
		allComponents[i]->Update();
	}

	return true;
}

void ModuleComponents::DeleteComponent(Component* component)
{
	auto componentIter = std::find(allComponents.begin(), allComponents.end(), component);
	if (componentIter != allComponents.end())
	{
		delete (*componentIter);
		allComponents.erase(componentIter);
	}
}

void Interpolation::Update()
{
	if (doLerp)
	{
			//Lerp position
			owner->position = lerp(initial_position, final_position, Time.deltaTime / REPLICATION_SEND_INTERVAL);
	
		

		//Lerp angle
		owner->angle = lerp(initial_angle, final_angle, Time.deltaTime/ REPLICATION_SEND_INTERVAL);
	}
	
}
