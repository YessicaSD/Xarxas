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
	loadTextureAsync("1847.jpg", &space);
	loadTextureAsync("asteroid1.png",        &asteroid1);
	loadTextureAsync("asteroid2.png",        &asteroid2);
	loadTextureAsync("projectile.png",            &laser);
	loadTextureAsync("explosion1.png",       &explosion1);
	loadTextureAsync("LivingArmorIdle_tex.png", &knightIdleImg);
	loadTextureAsync("Arm.png", &knightArm);
	loadTextureAsync("ground.png", &ground);
	loadTextureAsync("win.png", &win);
	loadTextureAsync("koDead.png", &ko);

#endif

	//audioClipLaser = App->modSound->loadAudioClip("laser.wav");
	//audioClipExplosion = App->modSound->loadAudioClip("explosion.wav");
	audioClipEnviroment = App->modSound->loadAudioClip("music.wav");
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
	
	textures[filename] = texturePtrAddress;

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

		CreateSameSizeSpritesheetAnim(&explosionClip, 30.f, false, 4, 4);
		CreateSameSizeSpritesheetAnim(&laserClip, 30.f, true, 3, 2);
		CreateJSONAnim(&knightIdleClip, "LivingArmorIdle_tex.json", 30.f, true, 4096.f, 2048.f);
	}
}

void ModuleResources::CreateSameSizeSpritesheetAnim(AnimationClip** clip, float framerate, bool loop, int rows, int columns) {
	(*clip) = App->modRender->addAnimationClip();
	(*clip)->frameTime = 1.f / framerate;
	(*clip)->loop = loop;
	float w = 1.0f / (float)columns;
	float h = 1.0f / (float)rows;
	for (int i = 0; i < rows * columns; ++i)
	{
		float x = (i % columns) / (float)columns;
		float y = (i / columns) / (float)rows;
		(*clip)->addFrameRect(vec4{ x, y, w, h });
	}
}

//INFO: We can get the image from the json
void ModuleResources::CreateJSONAnim(AnimationClip** clip, const std::string& json_path, float framerate, bool loop, float imgWidth, float imgHeight) {
	(*clip) = App->modRender->addAnimationClip();
	(*clip)->frameTime = 1.f / framerate;
	(*clip)->loop = loop;

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
		(*clip)->addFrameRect(vec4{x / imgWidth, y / imgHeight, w / imgWidth, h / imgHeight });
	}
	jsonFile.close();
}

#endif
