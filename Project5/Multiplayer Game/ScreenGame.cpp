
#include "Networks.h"

GameObject *spaceTopLeft = nullptr;
GameObject *spaceTopRight = nullptr;
GameObject *spaceBottomLeft = nullptr;
GameObject *spaceBottomRight = nullptr;

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
	/*spaceTopRight = Instantiate();
	spaceTopRight->sprite = App->modRender->addSprite(spaceTopRight);
	spaceTopRight->sprite->texture = App->modResources->space;
	spaceTopRight->sprite->order = -1;
	spaceBottomLeft = Instantiate();
	spaceBottomLeft->sprite = App->modRender->addSprite(spaceBottomLeft);
	spaceBottomLeft->sprite->texture = App->modResources->space;
	spaceBottomLeft->sprite->order = -1;
	spaceBottomRight = Instantiate();
	spaceBottomRight->sprite = App->modRender->addSprite(spaceBottomRight);
	spaceBottomRight->sprite->texture = App->modResources->space;
	spaceBottomRight->sprite->order = -1;*/
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
			//spaceTopLeft->position = bgSize * floor(camPos / bgSize);
			/*spaceTopRight->position = bgSize * (floor(camPos / bgSize) + vec2{ 1.0f, 0.0f });
			spaceBottomLeft->position = bgSize * (floor(camPos / bgSize) + vec2{ 0.0f, 1.0f });
			spaceBottomRight->position = bgSize * (floor(camPos / bgSize) + vec2{ 1.0f, 1.0f });;*/
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
	/*Destroy(spaceTopRight);
	Destroy(spaceBottomLeft);
	Destroy(spaceBottomRight);*/
}
