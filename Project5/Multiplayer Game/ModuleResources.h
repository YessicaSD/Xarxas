#pragma once

#include <map>

#define USE_TASK_MANAGER

struct Texture;

class ModuleResources : public Module
{
public:

	Texture *background = nullptr;
	Texture *space = nullptr;
	Texture *asteroid1 = nullptr;
	Texture *asteroid2 = nullptr;
	Texture *laser = nullptr;
	Texture *explosion1 = nullptr;
	Texture* knightIdleImg = nullptr;
	Texture* knightArm = nullptr;
	Texture* ground = nullptr;

	AnimationClip *explosionClip = nullptr;
	AnimationClip* knightIdleClip = nullptr;
	AnimationClip* laserClip = nullptr;

	AudioClip *audioClipLaser = nullptr;
	AudioClip *audioClipExplosion = nullptr;

	bool finishedLoading = false;

	std::map<std::string, Texture**> textures;

private:
	

	bool init() override;

#if defined(USE_TASK_MANAGER)
	
	class TaskLoadTexture : public Task
	{
	public:

		const char *filename = nullptr;
		Texture **texture = nullptr;

		void execute() override;
	};

	static const int MAX_RESOURCES = 16;
	TaskLoadTexture tasks[MAX_RESOURCES] = {};
	uint32 taskCount = 0;
	uint32 finishedTaskCount = 0;

	void onTaskFinished(Task *task) override;

	void CreateSameSizeSpritesheetAnim(AnimationClip** clip, float framerate, bool loop, int rows, int columns);

	void CreateJSONAnim(AnimationClip** clip, const std::string& json_path, float framerate, bool loop, float imgWidth, float imgHeight);

	void loadTextureAsync(const char *filename, Texture **texturePtrAddress);

#endif

};

