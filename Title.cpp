#include "Title.h"
#include "Controller.h"
#include "Button.h"
#include "Artist.h"
#include "Map.h"
#include "Character.h"

std::vector<SDL_Texture*> buttonTex;
SDL_Texture* titleBG;



void startMap()
{
	Map map;
	Character character;
	//switch game state to in map
	Controller::state = Controller::STATE_MAP;
	//load map data
	map.loadMap("NOT IMPLEMEMTED YET");

}

void quit()
{
	Controller::quit = true;
}

void Title::loadTitle()
{
	Artist artist;
	titleBG = artist.loadTexture("Resource/title/BG.png");

	buttonTex.push_back(artist.loadTexture("Resource/title/button0.png"));
	buttonTex.push_back(artist.loadTexture("Resource/title/button1.png"));
	buttonTex.push_back(artist.loadTexture("Resource/title/button2.png"));

	//load buttons
	Button button;
	Controller::buttonList.push_back(button.makeButton(buttonTex, 1920 / 2 - 128, 1080 / 2 - 64, 100, startMap));
	Controller::buttonList.push_back(button.makeButton(buttonTex, 1920 - 256, 1080 - 128, 100, quit));
}

void Title::draw()
{
	Button button;
	Artist artist;
	switch (Controller::state)
	{
	case Controller::STATE_TITLE:
		artist.drawImage(titleBG, 0, 0, 1920, 1080);
		button.updateButtonVector(Controller::buttonList);
		break;
	}
}