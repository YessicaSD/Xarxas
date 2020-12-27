#include "Networks.h"
#include "json.hpp"
#include "ModuleResources.h"

using json = nlohmann::json;


#if defined(USE_TASK_MANAGER)

void ModuleResources::TaskLoadTexture::execute()
{
	*texture = App->modTextures->loadTexture(filename);
}

#endif


bool ModuleResources::init()
{
	background = App->modTextures->loadTexture("background.jpg");

#if !defined(USE_TASK_MANAGER)
	space = App->modTextures->loadTexture("space_background.jpg");
	asteroid1 = App->modTextures->loadTexture("asteroid1.png");
	asteroid2 = App->modTextures->loadTexture("asteroid2.png");
	spacecraft1 = App->modTextures->loadTexture("spacecraft1.png");
	spacecraft2 = App->modTextures->loadTexture("spacecraft2.png");
	spacecraft3 = App->modTextures->loadTexture("spacecraft3.png");
	loadingFinished = true;
	completionRatio = 1.0f;
#else
	loadTextureAsync("space_background.jpg", &space);
	loadTextureAsync("asteroid1.png",        &asteroid1);
	loadTextureAsync("asteroid2.png",        &asteroid2);
	loadTextureAsync("spacecraft1.png",      &spacecraft1);
	loadTextureAsync("spacecraft2.png",      &spacecraft2);
	loadTextureAsync("spacecraft3.png",      &spacecraft3);
	loadTextureAsync("laser.png",            &laser);
	loadTextureAsync("explosion1.png",       &explosion1);
	loadTextureAsync("LivingArmor_tex.png",  &knightAttackImg);
#endif

	audioClipLaser = App->modSound->loadAudioClip("laser.wav");
	audioClipExplosion = App->modSound->loadAudioClip("explosion.wav");
	//App->modSound->playAudioClip(audioClipExplosion);

	return true;
}

#if defined(USE_TASK_MANAGER)

void ModuleResources::loadTextureAsync(const char * filename, Texture **texturePtrAddress)
{
	ASSERT(taskCount < MAX_RESOURCES);
	
	TaskLoadTexture *task = &tasks[taskCount++];
	task->owner = this;
	task->filename = filename;
	task->texture = texturePtrAddress;

	App->modTaskManager->scheduleTask(task, this);
}

void ModuleResources::onTaskFinished(Task * task)
{
	ASSERT(task != nullptr);

	TaskLoadTexture *taskLoadTexture = dynamic_cast<TaskLoadTexture*>(task);

	for (uint32 i = 0; i < taskCount; ++i)
	{
		if (task == &tasks[i])
		{
			finishedTaskCount++;
			task = nullptr;
			break;
		}
	}

	ASSERT(task == nullptr);

	if (finishedTaskCount == taskCount)
	{
		finishedLoading = true;

		// Create the explosion animation clip
		explosionClip = App->modRender->addAnimationClip();
		explosionClip->frameTime = 0.1f;
		explosionClip->loop = false;
		for (int i = 0; i < 16; ++i)
		{
			float x = (i % 4) / 4.0f;
			float y = (i / 4) / 4.0f;
			float w = 1.0f / 4.0f;
			float h = 1.0f / 4.0f;
			explosionClip->addFrameRect(vec4{ x, y, w, h });
		}

		CreateJSONAnim(&knightAttack, "LivingArmor_tex.json");
	}
}

//INFO: We can get the image from the json
void ModuleResources::CreateJSONAnim(AnimationClip** clip, const std::string& json_path) {
	(*clip) = App->modRender->addAnimationClip();
	//(*clip)->frameTime = 0.1f;
	(*clip)->frameTime = 1.f / 30.f;
	(*clip)->loop = false;

	std::ifstream jsonFile(json_path);
	if (!jsonFile) {
		LOG("Ivalid path");
	}
	json jsonObj = json::parse(jsonFile);
	json::iterator rects = jsonObj.find("SubTexture");
	for (json::iterator iter = rects->begin(); iter != rects->end(); ++iter) {
		float x, y, w, h;
		(*iter).at("x").get_to(x);
		(*iter).at("y").get_to(y);
		(*iter).at("width").get_to(w);
		(*iter).at("height").get_to(h);
		(*clip)->addFrameRect(vec4{x / 4096.f, y / 4096.f, w / 4096.f, h / 4096.f });
	}
	jsonFile.close();
}

#endif
