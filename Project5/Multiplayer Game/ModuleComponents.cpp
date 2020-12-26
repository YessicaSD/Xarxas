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

void Interpolation::SetFinal(vec2 f_pos, float f_angle)
{
	initial_angle = owner->angle;
	initial_position = owner->position;

	final_position = f_pos;
	final_angle = f_angle;

	secondsElapsed = 0;
	doLerp = true;
	//secondsElapsed = Time.time - secondsElapsed;
}

void Interpolation::Update()
{
	if (doLerp)
	{
		secondsElapsed += Time.deltaTime;

		float t = secondsElapsed / App->modNetClient->secondsSinceLastReplication;
		//LOG("%f",t);
		if (t > 1)
		{
			t = 1;
			doLerp = false;
		}
		
		//Lerp position
		owner->position = lerp(initial_position, final_position, t);
		
		//Lerp angle
		owner->angle = lerp(initial_angle, final_angle, t);
		

		
	}
	
}
