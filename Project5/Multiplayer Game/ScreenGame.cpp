
#include "Networks.h"

GameObject *spaceTopLeft = nullptr;
GameObject* grounds[4] = {nullptr, nullptr, nullptr, nullptr};

void ScreenGame::enable()
{
	if (isServer)
	{
		App->modNetServer->setListenPort(serverPort);
		App->modNetServer->setEnabled(true);
	}
	else
	{
		App->modNetClient->setServerAddress(serverAddress, serverPort);
		App->modNetClient->setPlayerInfo(playerName, spaceshipType);
		App->modNetClient->setEnabled(true);
	}

	spaceTopLeft = Instantiate();
	spaceTopLeft->sprite = App->modRender->addSprite(spaceTopLeft);
	spaceTopLeft->sprite->texture = App->modResources->space;
	spaceTopLeft->sprite->order = -1;

	lastScreenSize = { 0, 0 };
	// Generate World
	//Height
	float height = 480;
	grounds[0] = Instantiate();
	grounds[0]->sprite = App->modRender->addSprite(grounds[0]);
	grounds[0]->sprite->texture = App->modResources->ground;
	grounds[0]->sprite->order = 5;
	grounds[0]->position = { 0,height };
	grounds[0]->size = { 1200,100 };


	grounds[1] = Instantiate();
	grounds[1]->sprite = App->modRender->addSprite(grounds[1]);
	grounds[1]->sprite->texture = App->modResources->ground;
	grounds[1]->sprite->order = 5;
	grounds[1]->position = { 0,-height };
	grounds[1]->angle = 180;
	grounds[1]->size = { 1200,100 };

	//Width
	float width = 635;
	grounds[2] = Instantiate();
	grounds[2]->sprite = App->modRender->addSprite(grounds[2]);
	grounds[2]->sprite->texture = App->modResources->ground;
	grounds[2]->sprite->order = 5;
	grounds[2]->position = { -width, 0 };
	grounds[2]->angle = 90;
	grounds[2]->size = { 1200,100 };

	grounds[3] = Instantiate();
	grounds[3] ->sprite = App->modRender->addSprite(grounds[3]);
	grounds[3] ->sprite->texture = App->modResources->ground;
	grounds[3] ->sprite->order = 5;
	grounds[3] ->position = { width, 0 };
	grounds[3] ->angle = -90;
	grounds[3] ->size = { 1200,100 };

	
	App->modSound->playAudioClip(App->modResources->audioClipEnviroment);

}

void ScreenGame::update()
{
	if (!(App->modNetServer->isConnected() || App->modNetClient->isConnected()))
	{
		App->modScreen->swapScreensWithTransition(this, App->modScreen->screenMainMenu);
	}
	else
	{
		if ((lastScreenSize.x != Window.width || lastScreenSize.y != Window.height))
		{
			vec2 camPos = App->modRender->cameraPosition;
			vec2 bgSize = spaceTopLeft->sprite->texture->size;
			float resultHeight = ((float)Window.width / bgSize.x) * bgSize.y;
			bool idHightInof = resultHeight >= Window.height;

			if(Window.height>= Window.width || !idHightInof)
				spaceTopLeft->size = vec2{ ((float)Window.height  / bgSize.y) *   bgSize.x, (float)Window.height};
			else
				spaceTopLeft->size = vec2{ (float) Window.width, resultHeight };

			lastScreenSize = { (float) Window.width, (float)Window.height};
		}
	}
}

void ScreenGame::gui()
{
	if (!isServer)
	{
		uint32 networId = App->modNetClient->getNetworkId();
		if (networId != 0)
		{
			GameObject* client = App->modLinkingContext->getNetworkGameObject(networId);
			if (client)
			{
				Spaceship* behavior = (Spaceship*)client->behaviour;
				ImGui::Begin("Score");
				ImGui::TextColored(ImVec4(1.0,0.0,0,1.0),"Number of Kills: %i", behavior->numberKills);
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(66.0/255.0F, 230/255.0F, 245/255.0F, 1.0), "Number of Dead: %i", behavior->numberDeads);
				ImGui::End();
			}

		}
	}
	
}

void ScreenGame::disable()
{
	Destroy(spaceTopLeft);
	for(int i = 0; i < 4; i++)
	{
		Destroy(grounds[i]);
	}
}
