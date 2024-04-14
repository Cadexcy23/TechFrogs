#include "Map.h"
#include "Artist.h"
//#include "MainMenu.h"
#include "Controller.h"
//#include "Entity.h"
#include "Mixer.h"
#include <fstream>
#include <regex>
#include <map>




Map::map Map::activeMap;

enum tileVars { COMMENT, EMPTY, NAME, COLLISION }; //Add new vars here


std::vector<std::vector<bool>> Map::getMapChunkCollision(SDL_Point topLeft, SDL_Point size)
{
	//make a 2d vector to hold the data
	std::vector<std::vector<bool>> returnChunk;
	//resize it to what we ask
	returnChunk.resize(size.x);
	for (int x = 0; x < returnChunk.size(); x++)
	{
		returnChunk[x].resize(size.y);
	}

	//read map data and pass it to the vector
	for (int x = 0; x < returnChunk.size(); x++)
	{
		for (int y = 0; y < returnChunk[0].size(); y++)
		{
			//make sure we are adding points that are in the map only
			if (x + topLeft.x < 0 || y + topLeft.y < 0 || x + topLeft.x >= Map::activeMap.tileGrid.size() || y + topLeft.y >= Map::activeMap.tileGrid[0].size())
			{
				returnChunk[x][y] = false;
			}
			else
			{
				returnChunk[x][y] = !Map::activeMap.tileGrid[x + topLeft.x][y + topLeft.y].tileType->collision;
			}
		}
	}



	return returnChunk;
}

SDL_Point getMapMousePos()
{
	SDL_Point returnPoint;

	//get mouse pos relative to zoom and panning
	SDL_Point newGoal = { (Controller::mouseX - 1920 / 2) / Map::activeMap.camZoom + Map::activeMap.camPosX, (Controller::mouseY - 1080 / 2) / Map::activeMap.camZoom + Map::activeMap.camPosY };

	//convert rot to radians
	float rads = Map::activeMap.camRot * (M_PI / 180);

	//create rotation matrix based off camRot
	float R[4];
	R[0] = R[3] = cos(rads), R[1] = -(R[2] = -sin(rads));

	//use matrix to rotate 
	float p[2] = { newGoal.x, newGoal.y };
	float o[2] = { Map::activeMap.camPosX, Map::activeMap.camPosY };
	float t0 = p[0] - o[0], t1 = p[1] - o[1];
	p[0] = R[0] * t0 + R[1] * t1 + o[0], p[1] = R[2] * t0 + R[3] * t1 + o[1];

	//set goal to the rotated points
	returnPoint.x = p[0];
	returnPoint.y = p[1];

	return returnPoint;
}

void drawSelector()
{
	//safety check if we have a selected unit
	if (Map::activeMap.selectedCharacterID < 0 || Map::activeMap.selectedCharacterID > Map::activeMap.entityList.characters.size())
	{

	}
	else
	{
		//pointer to map and char
		Map::map* mapPoint = &Map::activeMap;
		Character::character* selectedChar = &mapPoint->entityList.characters[Map::activeMap.selectedCharacterID];
		Artist::drawAnimation(mapPoint->selector, selectedChar->x * 256 - mapPoint->camOffset.x/* - centering.x*/, selectedChar->y * 256 - mapPoint->camOffset.y/* - centering.y*/, 0, 0, 0, 360 - Map::activeMap.camRot, 50);
	}

	//draw selector of what tile the mouse is over
	//get goal tile to hover
	SDL_Point mouseTilePos = getMapMousePos();
	//convert to tiles then back so its right on a tile point
	mouseTilePos.x /= 256;
	mouseTilePos.y /= 256;
	mouseTilePos.x *= 256;
	mouseTilePos.y *= 256;

	//use goal to guide actual to it
	if (Controller::FPSLock)
	{
		SDL_Point dif = { Map::activeMap.tileSelectorPos.x - mouseTilePos.x, Map::activeMap.tileSelectorPos.y - mouseTilePos.y };

		//if dif > 10 do *.1 to make it faster if not then just move 1
		if (abs(dif.x) > 4 || abs(dif.y) > 4)
		{
			Map::activeMap.tileSelectorPos.x -= dif.x * .25;
			Map::activeMap.tileSelectorPos.y -= dif.y * .25;
		}
		else
		{
			Map::activeMap.tileSelectorPos.x -= dif.x;
			Map::activeMap.tileSelectorPos.y -= dif.y;
		}
	}

	//draw selector over actual
	Artist::drawAnimation(Map::activeMap.selector, Map::activeMap.tileSelectorPos.x - Map::activeMap.camOffset.x/* - centering.x*/, Map::activeMap.tileSelectorPos.y - Map::activeMap.camOffset.y/* - centering.y*/);
}

void drawTileInfo(SDL_Point origin, SDL_Point tilePos, SDL_RendererFlip flip) //make it take a offset on where to draw it and also pass in the tile so we can use this modularly and have multiple of something
{
	//draw panel back
	Artist::drawImage(Map::activeMap.tileInfoPanel, origin.x, origin.y, 0, 0, 0, 0, flip);

	//get the chunk of the map texture that is this tile
	SDL_Rect chunk;
	chunk.x = tilePos.x;
	chunk.y = tilePos.y;
	chunk.w = 256;
	chunk.h = 256;

	//shift point left or right depending on flip
	int shift = 176;
	if (flip == SDL_FLIP_HORIZONTAL)
	{
		shift = 27;
	}

	Artist::drawImageChunk(Map::activeMap.backgroundFinal, origin.x + shift, origin.y + 27, chunk, 128, 128, Map::activeMap.camRot);

	//make selector graphic bigger and not go into tile so it doesnt show up
	//fill out info once it is there
}

void renderEntities()
{
	Artist artist;


	//go thru each character
	for (int i = 0; i < Map::activeMap.entityList.characters.size(); i++)
	{
		//draw ents
		Map::activeMap.entityList.characters[i].draw(Map::activeMap.camOffset, 360 - Map::activeMap.camRot);
		//draw ent paths
		Map::activeMap.entityList.characters[i].drawGoalPath(&Map::activeMap.camOffset);

	}
	//add more for more ents

}

void renderBackground()//SOMEDAY MAKE IT SO IT COPIES WHAT WE ALREADY HAVE OVER THE AMOUNT OF TILES WE SHIFT THEN ONLY ADD WHAT WE DONT HAVE     ALSO ADD DECALS BLOODSPLATS/BULLETHOLES//EXPLOSIONS PERMANANT STUFF
{
	Artist artist;

	//update our offset
	Map::activeMap.camOffset = { int(Map::activeMap.camPosX - Map::activeMap.renderDist / 2) / 256 * 256, int(Map::activeMap.camPosY - Map::activeMap.renderDist / 2) / 256 * 256 };

	artist.setRenderTarget(Map::activeMap.backgroundTiles);
	for (int x = 0; x < Map::activeMap.renderDist / 256; x++)//will need changed to include a cap on how much we draw something like res/32 with extra room ALSO a starting point
	{
		for (int y = 0; y < Map::activeMap.renderDist / 256; y++)//will need changed to include a cap on how much we draw something like res/32 with extra room ALSO a starting point
		{
			SDL_Point finalGridPos = { x + Map::activeMap.camOffset.x / 256, y + Map::activeMap.camOffset.y / 256 };//TEMP why?

			if (finalGridPos.x >= 0 && finalGridPos.x < Map::activeMap.tileGrid.size() && finalGridPos.y >= 0 && finalGridPos.y < Map::activeMap.tileGrid[0].size())
			{
				artist.drawImage(Map::activeMap.tileGrid[finalGridPos.x][finalGridPos.y].tileType->tex, x * 256, y * 256, 256, 256);
			}
			else
			{
				artist.drawImage(Map::activeMap.masterTileList[0].tex, x * 256, y * 256, 256, 256);
			}
		}
	}

	artist.setRenderTarget(NULL);
}

std::pair<tileVars, std::smatch> parse_line(std::istream* dataFile) {
	/*
	read line from file and parse it
	IN:  pointer to iostream data file
	OUT: key(unique identifier int), match (desired string from line)
	*/

	// Key/Regex map
	std::map<tileVars, std::regex> rx_map;
	rx_map[COMMENT] = std::regex("^\\s*//.*"); //Matches Comments
	rx_map[EMPTY] = std::regex("^\\s*$");        //Matches empty strings (with white space)
	rx_map[NAME] = std::regex("^\\s*\\[Name\\]\\s*(.*)"); //Add new vars here
	rx_map[COLLISION] = std::regex("^\\s*\\[Collision\\]\\s*(.*)");
	//rx_map[CONNECT] = std::regex("^\\s*\\[Connect\\]\\s*(.*)");

	static std::string s; //This must be static or the smatch var will not work
	std::smatch match;    // Regex matches
	tileVars key;         // Match key

	std::getline(*dataFile, s); //Read in a line (string\n) from the file

	for (std::map<tileVars, std::regex>::iterator iter = rx_map.begin(); iter != rx_map.end(); ++iter) {
		if (std::regex_match(s, (*iter).second)) { //If there is a match
			std::regex_search(s, match, (*iter).second); //get matches
			key = (*iter).first; //get the key associated with the match
			break; //Stop searching and return key match pair
		}
	}
	return std::make_pair(key, match); //Return key match pair
}

Map::masterTile loadTileData(std::string tileName) {
	/*
	RimLike Tile Vars Loading
	IN: tile name name string
	OUT: tile struct with vars set based on data file
	*/

	Artist artist;

	// Tile defaults
	Map::masterTile tile = { NULL, tileName }; //Add new vars here
	std::pair<tileVars, std::smatch> key_match;
	std::string s;

	// Get the texture from png
	tile.tex = artist.loadTexture("Resource/tiles/" + tileName + ".png");
	if (tile.tex == NULL)
	{
		tile.tex = artist.loadTexture("Resource/tiles/error.png");
	}

	// Open tile data file
	std::ifstream tileDataFile;
	tileDataFile.open("Resource/tiles/tileData"); // Open file for read

	if (!tileDataFile.is_open()) return tile; // Return from funciton if file does not exist

	key_match = parse_line(&tileDataFile); //Parse first line

	while (tileDataFile.eof() != 1) { //Loop until end of file

		//if the current line is [Name] something and it matches the given name or default
		if (key_match.first == NAME && (key_match.second.str(1) == tileName || key_match.second.str(1) == "default")) {

			bool nameDefault = (key_match.second.str(1) == "default"); //Makes the following loop not break; on default case

			key_match = parse_line(&tileDataFile); //Parse next line

			// Keep loading parameters until next [Name] or end of file
			while (key_match.first != NAME) {
				//Set variables based on key
				switch (key_match.first) {
					//Add new vars here
				case COLLISION: tile.collision = std::stoi(key_match.second.str(1));
					break;
					//case CONNECT: tile.connect = std::stoi(key_match.second.str(1));
						//break;
				}

				if (tileDataFile.eof()) break; //Break out if the file ends

				key_match = parse_line(&tileDataFile); //Parse next line
			};
			if (!nameDefault) break; //Break out if Name is not default
		}
		else key_match = parse_line(&tileDataFile); //Parse next line
	};
	tileDataFile.close(); //Close the file $_$ 
	return tile;
}

Map::tile getTile(std::string name)
{
	Map::tile returnTile;

	//defaults
	returnTile.roofed = false;

	//look in master list to see if we already have it loaded
	for (int i = 0; i < Map::activeMap.masterTileList.size(); i++)
	{
		if (name == Map::activeMap.masterTileList[i].name)
		{
			returnTile.tileType = &Map::activeMap.masterTileList[i];
			return returnTile;
		}
	}

	//if we dont have it loaded, add it to the master list so we can return with it
	Map::activeMap.masterTileList.push_back(loadTileData(name));
	returnTile.tileType = &Map::activeMap.masterTileList[Map::activeMap.masterTileList.size() - 1];
	return returnTile;
}

void dummyFunc()
{

}

void closeProgram()
{
	Controller::quit = true;
}

void loadMapMenuButtons()
{
	//clear old buttons
	Controller::buttonList.clear();

	//starting point for menu draw
	SDL_Point start = { 1536 , 384 };

	//load each button
	std::vector<SDL_Texture*> unitTex = { Artist::loadTexture("Resource/menus/map/menuUnit0.png"), Artist::loadTexture("Resource/menus/map/menuUnit1.png") };
	Controller::buttonList.push_back(Button::makeButton(unitTex, start.x, start.y, 100, dummyFunc));

	std::vector<SDL_Texture*> guideTex = { Artist::loadTexture("Resource/menus/map/menuGuide0.png"), Artist::loadTexture("Resource/menus/map/menuGuide1.png") };
	Controller::buttonList.push_back(Button::makeButton(guideTex, start.x, start.y + 64 * 1, 100, dummyFunc));

	std::vector<SDL_Texture*> optionsTex = { Artist::loadTexture("Resource/menus/map/menuOptions0.png"), Artist::loadTexture("Resource/menus/map/menuOptions1.png") };
	Controller::buttonList.push_back(Button::makeButton(optionsTex, start.x, start.y + 64 * 2, 100, dummyFunc));

	std::vector<SDL_Texture*> suspendTex = { Artist::loadTexture("Resource/menus/map/menuSuspend0.png"), Artist::loadTexture("Resource/menus/map/menuSuspend1.png") };
	Controller::buttonList.push_back(Button::makeButton(suspendTex, start.x, start.y + 64 * 3, 100, dummyFunc));

	std::vector<SDL_Texture*> endTex = { Artist::loadTexture("Resource/menus/map/menuEnd0.png"), Artist::loadTexture("Resource/menus/map/menuEnd1.png") };
	Controller::buttonList.push_back(Button::makeButton(endTex, start.x, start.y + 64 * 4, 100, closeProgram));

}



void Map::loadMap(std::string path)
{
	Artist artist;
	//Entity entity;


	//load map vars
	Map::activeMap.renderDist = 256 * 38;
	Map::activeMap.backgroundTiles = artist.loadTargetTexture(Map::activeMap.renderDist, Map::activeMap.renderDist);
	Map::activeMap.backgroundFinal = artist.loadTargetTexture(Map::activeMap.renderDist, Map::activeMap.renderDist);
	Map::activeMap.selector = artist.loadAnimationData("characters", "selector", 16);
	activeMap.mapMenuHead = Artist::loadTexture("Resource/menus/map/menuHead.png");
	activeMap.mapMenuFoot = Artist::loadTexture("Resource/menus/map/menuFoot.png");
	activeMap.mapCharacterHead = Artist::loadTexture("Resource/menus/character/menuHead.png");
	activeMap.mapCharacterFoot = Artist::loadTexture("Resource/menus/character/menuFoot.png");
	activeMap.tileInfoPanel = Artist::loadTexture("Resource/HUD/tileInfoPanel.png");
	activeMap.pathGraphics = { Artist::loadTexture("Resource/tiles/NONE.png"), Artist::loadTexture("Resource/tiles/move.png"), Artist::loadTexture("Resource/tiles/attack.png"),
		Artist::loadTexture("Resource/tiles/moveThru.png") };
	Map::activeMap.menuState = Map::MENU_NONE;
	Map::activeMap.selectedCharacterID = -1;
	activeMap.tileSelectorPos = { 0, 0 };
	Map::activeMap.camRot = 0;
	Map::activeMap.camRotGoal = 0;
	Map::activeMap.camPosX = 1920;
	Map::activeMap.camPosY = 1080;
	Map::activeMap.camPosXGoal = 1920;
	Map::activeMap.camPosYGoal = 1080;
	Map::activeMap.camZoom = .5;
	Map::activeMap.camZoomGoal = .5;
	Map::activeMap.camOffset = { int(Map::activeMap.camPosX - Map::activeMap.renderDist / 2), int(Map::activeMap.camPosY - Map::activeMap.renderDist / 2) };


	//set map size
	Map::activeMap.tileGrid.resize(100);
	for (int i = 0; i < Map::activeMap.tileGrid.size(); i++)
	{
		Map::activeMap.tileGrid[i].resize(100);
	}

	//load tiles
	getTile("outOfBounds");
	getTile("grass");
	getTile("dirt");
	getTile("wall");

	//set each tile
	srand(clock());
	for (int x = 0; x < Map::activeMap.tileGrid.size(); x++)
	{
		for (int y = 0; y < Map::activeMap.tileGrid[x].size(); y++)
		{
			int random = rand() % (Map::activeMap.masterTileList.size() - 1);
			Map::activeMap.tileGrid[x][y].tileType = &Map::activeMap.masterTileList[random + 1];
		}
	}

	//wall off the borders
	for (int x = 0; x < Map::activeMap.tileGrid.size(); x++)
	{
		Map::activeMap.tileGrid[x][0] = getTile("wall");
		Map::activeMap.tileGrid[x][Map::activeMap.tileGrid[0].size() - 1] = getTile("wall");;
	}
	for (int y = 0; y < Map::activeMap.tileGrid[0].size(); y++)
	{
		Map::activeMap.tileGrid[0][y] = getTile("wall");;
		Map::activeMap.tileGrid[Map::activeMap.tileGrid.size() - 1][y] = getTile("wall");;
	}

	//create BG image from tiles
	renderBackground();




	//chars
	Character::load();
}

void Map::draw()
{
	if (Controller::state == Controller::STATE_MAP)
	{
		Artist artist;

		//determine if we need to regenerate the background image
		SDL_Point testCamOffset = { (Map::activeMap.camPosX - Map::activeMap.renderDist / 2), (Map::activeMap.camPosY - Map::activeMap.renderDist / 2) };
		SDL_Point offsetDif = { abs(testCamOffset.x - Map::activeMap.camOffset.x), abs(testCamOffset.y - Map::activeMap.camOffset.y) };
		if (offsetDif.x > 256 * 1 || offsetDif.y > 256 * 1)//maybe needs tweaking later? 
		{
			renderBackground(); //def making scrolling not as smooth, check again after we optimize renderBackground func
		}

		//get the point that the cam is on relative to the image
		SDL_Point point = { int(Map::activeMap.camPosX * Map::activeMap.camZoom - Map::activeMap.camOffset.x * Map::activeMap.camZoom), int(Map::activeMap.camPosY * Map::activeMap.camZoom - Map::activeMap.camOffset.y * Map::activeMap.camZoom) };

		//set renmder target to the final BG image
		artist.setRenderTarget(Map::activeMap.backgroundFinal);

		//draw tiles
		artist.drawImage(Map::activeMap.backgroundTiles, 0, 0);

		//render entities
		renderEntities();

		//render selector
		drawSelector();

		//reset render target
		artist.setRenderTarget(NULL);

		//draw the final background to he screen with the given rotation and point
		artist.drawImage(Map::activeMap.backgroundFinal, 1920 / 2 - Map::activeMap.camPosX * Map::activeMap.camZoom + Map::activeMap.camOffset.x * Map::activeMap.camZoom, 1080 / 2 - Map::activeMap.camPosY * Map::activeMap.camZoom + Map::activeMap.camOffset.y * Map::activeMap.camZoom, Map::activeMap.renderDist * Map::activeMap.camZoom, Map::activeMap.renderDist * Map::activeMap.camZoom, Map::activeMap.camRot, &point);





		if (Controller::FPSLock)
		{
			//keep camera rot towards goal
			if (Map::activeMap.camRot != Map::activeMap.camRotGoal)
			{
				//if we are close to the goal set it equal
				if (abs(Map::activeMap.camRot - Map::activeMap.camRotGoal) < .1)
				{
					Map::activeMap.camRot = Map::activeMap.camRotGoal;
				}

				//calc difference from goal then make it how much we need to shift the cam
				float goalDif = Map::activeMap.camRot - Map::activeMap.camRotGoal;
				if (goalDif < 0 && abs(goalDif) > 180)
				{
					goalDif += 360;
				}
				if (goalDif > 0 && abs(goalDif) > 180)
				{
					goalDif -= 360;
				}
				goalDif *= -1;
				Map::activeMap.camRot += goalDif * .1;
				if (Map::activeMap.camRot > 359)
				{
					Map::activeMap.camRot -= 360;
				}
				else if (Map::activeMap.camRot < 0)
				{
					Map::activeMap.camRot += 360;
				}
			}
			//keep zoom going towards goal
			if (Map::activeMap.camZoom != Map::activeMap.camZoomGoal)
			{
				if (abs(Map::activeMap.camZoom - Map::activeMap.camZoomGoal) < .005)
				{
					Map::activeMap.camZoom = Map::activeMap.camZoomGoal;
				}
				float goalDif = Map::activeMap.camZoomGoal - Map::activeMap.camZoom;
				Map::activeMap.camZoom += goalDif * .15;
			}
			//keep cam pos towards goal
			if (Map::activeMap.camPosX != Map::activeMap.camPosXGoal || Map::activeMap.camPosY != Map::activeMap.camPosYGoal)
			{
				int goalDifX = Map::activeMap.camPosXGoal - Map::activeMap.camPosX;
				Map::activeMap.camPosX += goalDifX * .2;

				int goalDifY = Map::activeMap.camPosYGoal - Map::activeMap.camPosY;
				Map::activeMap.camPosY += goalDifY * .2;
			}
		}

		//draw HUD

		//panel of info about the tile and whos on it
		//drawTileInfo({ 1536, 64 }, { getMapMousePos().x / 256 * 256 - Map::activeMap.camOffset.x, getMapMousePos().y / 256 * 256 - Map::activeMap.camOffset.y }, SDL_FLIP_NONE);
		drawTileInfo({ 1536, 64 }, { activeMap.tileSelectorPos.x - Map::activeMap.camOffset.x, activeMap.tileSelectorPos.y - Map::activeMap.camOffset.y }, SDL_FLIP_NONE);

		//second panel of info for the char we have selected (later will have a drop down of stats to compare for combat)
		if (activeMap.selectedCharacterID >= 0)
		{
			drawTileInfo({ 64, 64 }, { int(Map::activeMap.entityList.characters[Map::activeMap.selectedCharacterID].x) * 256 - Map::activeMap.camOffset.x, int(Map::activeMap.entityList.characters[Map::activeMap.selectedCharacterID].y) * 256 - Map::activeMap.camOffset.y }, SDL_FLIP_HORIZONTAL);
		}
		

		//draw button list
		if (activeMap.menuState == MENU_MAP)
		{
			Artist::drawImage(activeMap.mapMenuHead, 1536, 384 - 64);
			Artist::drawImage(activeMap.mapMenuFoot, 1536, 384 + 64 * 5);
			Button::updateButtonVector(Controller::buttonList);
		}




		//TEMP for drawing crosshair
		//artist.changeRenderColor(255, 0, 0, 255);
		//artist.drawLineFromPoints(0, 1080 / 2, 1920, 1080 / 2);
		//artist.changeRenderColor(0, 255, 0, 255);
		//artist.drawLineFromPoints(1920 / 2, 0, 1920 / 2, 1080);
		//artist.changeRenderColor(255, 0, 255, 255);

		////TEMP for drawing some info
		//artist.drawLetters("X:" + std::to_string(Map::activeMap.camPosX) + " Y:" + std::to_string(Map::activeMap.camPosY), 0, 128, Artist::smallFont);
		//artist.drawLetters("Rot:" + std::to_string(Map::activeMap.camRot), 0, 128*2, Artist::smallFont);
		//artist.drawLetters("Zoom:" + std::to_string(Map::activeMap.camZoom), 0, 128*3, Artist::smallFont);
		//artist.drawLetters("Menu State:" + std::to_string(Map::activeMap.menuState), 0, 128*4, Artist::smallFont);

	}
}

void Map::update()
{
	
	//go thru each character
	for (int i = 0; i < Map::activeMap.entityList.characters.size(); i++)
	{
		Character::character* c = &Map::activeMap.entityList.characters[i];
		//thur each update list
		for (int j = 0; j < Map::activeMap.entityList.characters[i].updateList.size(); j++)
		{
			Map::activeMap.entityList.characters[i].updateList[j](c);
		}
	}

	//TEMP?
	if (Map::activeMap.entityList.characters.size() > 0)
	{
		activeMap.camPosX = Map::activeMap.entityList.characters[0].x;
		activeMap.camPosY = Map::activeMap.entityList.characters[0].y;
		activeMap.camPosXGoal = Map::activeMap.entityList.characters[0].x;
		activeMap.camPosYGoal = Map::activeMap.entityList.characters[0].y;
		
	}
}

std::vector<std::vector<bool>> getMapChunkCollision(SDL_Point topLeft, SDL_Point size)
{
	//make a 2d vector to hold the data
	std::vector<std::vector<bool>> returnChunk;
	//resize it to what we ask
	returnChunk.resize(size.x);
	for (int x = 0; x < returnChunk.size(); x++)
	{
		returnChunk[x].resize(size.y);
	}

	//read map data and pass it to the vector
	for (int x = 0; x < returnChunk.size(); x++)
	{
		for (int y = 0; y < returnChunk[0].size(); y++)
		{
			//make sure we are adding points that are in the map only
			if (x + topLeft.x < 0 || x + topLeft.y < 0 || x + topLeft.x >= Map::activeMap.tileGrid.size() || y + topLeft.y >= Map::activeMap.tileGrid[0].size())
			{
				returnChunk[x][y] = false;
			}
			else
			{
				returnChunk[x][y] = !Map::activeMap.tileGrid[x + topLeft.x][y + topLeft.y].tileType->collision;
			}
		}
	}

	return returnChunk;
}

void Map::controller()//MOVE SOME FUNCTIUONALITY TO THIER OWN FUNCTIONS/IN THIER STRUCT
{
	if (Controller::state == Controller::STATE_MAP)
	{
		if (Controller::FPSLock)
		{
			//ROT CAM WITH KEYS SMOOTH
			if (Controller::keyboardStates[SDL_SCANCODE_E] && Controller::keyboardStates[SDL_SCANCODE_LCTRL])
			{
				Map::activeMap.camRotGoal -= 1;
				if (Map::activeMap.camRotGoal < 0)
				{
					Map::activeMap.camRotGoal += 359;
				}
			}
			if (Controller::keyboardStates[SDL_SCANCODE_Q] && Controller::keyboardStates[SDL_SCANCODE_LCTRL])
			{
				Map::activeMap.camRotGoal += 1;
				if (Map::activeMap.camRotGoal > 359)
				{
					Map::activeMap.camRotGoal -= 359;
				}
			}

			//MOVE CAM WITH KEYS
			//get directions we are moving
			bool directions[4] = { 0, 0, 0, 0 };
			if (Controller::keyboardStates[SDL_SCANCODE_UP])
			{
				directions[0] = true;
			}
			if (Controller::keyboardStates[SDL_SCANCODE_DOWN])
			{
				directions[1] = true;
			}
			if (Controller::keyboardStates[SDL_SCANCODE_LEFT])
			{
				directions[2] = true;
			}
			if (Controller::keyboardStates[SDL_SCANCODE_RIGHT])
			{
				directions[3] = true;
			}

			//get the velocity split
			int dirCount = 1;//uncomment and set to 0 to make it split the vel between directions
			//for (int i = 0; i < 4; i++)
			//{
			//	if (directions[i])
			//	{
			//		dirCount++;
			//	}
			//}
			float splitVel = 0;
			if (dirCount > 0)
				splitVel = 5 / dirCount;

			//apply vel to vector
			SDL_Point v = { 0, 0 };
			if (directions[0])
			{
				v.y -= splitVel;
			}
			if (directions[1])
			{
				v.y += splitVel;
			}
			if (directions[2])
			{
				v.x -= splitVel;
			}
			if (directions[3])
			{
				v.x += splitVel;
			}

			//convert to radians
			double theta = Map::activeMap.camRot * (M_PI / 180);

			//transform vel to be relative to the cam rot
			float addX = v.x * cos(theta) + v.y * sin(theta);
			float addY = v.x * sin(theta) - v.y * cos(theta);

			//add it to the cam pos
			Map::activeMap.camPosXGoal += addX / Map::activeMap.camZoom;
			Map::activeMap.camPosYGoal -= addY / Map::activeMap.camZoom;

		}

		//ROT CAM WITH KEYS
		if (Controller::keyboardStates[SDL_SCANCODE_E] == 1 && !Controller::keyboardStates[SDL_SCANCODE_LCTRL])
		{
			Map::activeMap.camRotGoal -= 45;
			if (Map::activeMap.camRotGoal < 0)
			{
				Map::activeMap.camRotGoal += 360;
			}
		}
		if (Controller::keyboardStates[SDL_SCANCODE_Q] == 1 && !Controller::keyboardStates[SDL_SCANCODE_LCTRL])
		{
			Map::activeMap.camRotGoal += 45;
			if (Map::activeMap.camRotGoal > 359)
			{
				Map::activeMap.camRotGoal -= 360;
			}
		}

		//MOVE CAM WITH MOUSE DRAG
		if (Controller::mouseStates[1] == 2)
		{
			//calculate dif in mouse pos
			SDL_Point mouseDif = { Controller::lastMousePos.x - Controller::mouseX, Controller::lastMousePos.y - Controller::mouseY };

			if (abs(mouseDif.x) > 0 || abs(mouseDif.y) > 0)
			{
				//apply dif to vector
				SDL_Point v = { mouseDif.x, mouseDif.y };

				//convert to radians
				double theta = Map::activeMap.camRot * (M_PI / 180);

				//transform vel to be relative to the cam rot
				float addX = v.x * cos(theta) + v.y * sin(theta);
				float addY = v.x * sin(theta) - v.y * cos(theta);

				Map::activeMap.camPosXGoal += addX / Map::activeMap.camZoom;
				Map::activeMap.camPosYGoal -= addY / Map::activeMap.camZoom;
			}
		}

		//ZOOM IN OR OUT WITH MOUSE WHEEL
		if (Controller::mouseWheelMovment.y != 0)
		{
			if (Controller::mouseWheelMovment.y > 0)
			{
				if (Map::activeMap.camZoomGoal <= .25)
				{
					Map::activeMap.camZoomGoal += .25;
				}
				else if (Map::activeMap.camZoomGoal <= .5)
				{
					Map::activeMap.camZoomGoal += 1;
				}
			}
			else if (Controller::mouseWheelMovment.y < 0)
			{
				if (Map::activeMap.camZoomGoal <= .5)
				{
					Map::activeMap.camZoomGoal -= .25;
				}
				if (Map::activeMap.camZoomGoal <= 1)
				{
					Map::activeMap.camZoomGoal -= .5;
				}
			}
			if (Map::activeMap.camZoomGoal > 1)
			{
				Map::activeMap.camZoomGoal = 1;
			}
			else if (Map::activeMap.camZoomGoal < .25)
			{
				Map::activeMap.camZoomGoal = .25;
			}
		}

		//RESET ZOOM
		if (Controller::keyboardStates[SDL_SCANCODE_Z] == 1)
		{
			Map::activeMap.camZoomGoal = .5;
		}

		//LEFT CLICK
		if (Controller::mouseStates[0] == 1)
		{
			if (activeMap.menuState == MENU_NONE)//MAKE SWITCH CASE OF STATES       MAKE ITS OWN FUNC???? so we can do early returns
			{
				SDL_Point mousePoint = getMapMousePos();
				mousePoint.x /= 256;
				mousePoint.y /= 256;
				bool taken = false; //set true if we find someone is on the tile


				//check if we have a selected unit
				if (Map::activeMap.selectedCharacterID >= 0 && Map::activeMap.selectedCharacterID < Map::activeMap.entityList.characters.size())
				{
					//get pointer for selected char
					Character::character* tempChar = &Map::activeMap.entityList.characters[Map::activeMap.selectedCharacterID];

					
					//Get map point at center of requested tile
					mousePoint.x = mousePoint.x * 256 + 128;
					mousePoint.y = mousePoint.y * 256 + 128;

					//check if its diff than last tried goal
					if (mousePoint.x == tempChar->lastTriedGoal.x && mousePoint.y == tempChar->lastTriedGoal.y)
					{
						//already tried
					}
					else
					{
						//set last tried goal
						tempChar->lastTriedGoal = mousePoint;
						//set goal
						tempChar->goal = mousePoint;

						//figure out which point is up left most between the goal and entity
						SDL_Point topLeft = tempChar->goal; //default to goal
						if (tempChar->x < topLeft.x)
						{
							topLeft.x = tempChar->x;
						}
						if (tempChar->y < topLeft.y)
						{
							topLeft.y = tempChar->y;
						}
						//convert to tile pos
						topLeft.x /= 256;
						topLeft.y /= 256;
						//move it up left to acount for extra room givin
						topLeft.x -= 10;
						topLeft.y -= 10;

						//get distance of ent and goal in tiles
						SDL_Point dist = { abs(tempChar->x - tempChar->goal.x),  abs(tempChar->y - tempChar->goal.y) };
						dist.x /= 256;
						dist.y /= 256;
						//give room around to look for paths
						dist.x += 21;
						dist.y += 21;

						if (!tempChar->findPath(getMapChunkCollision(topLeft, dist), topLeft))
						{
							//if we dont find a way to goal set a path point to the tile we are in
							tempChar->path.push_back({ int(tempChar->x) / 256 * 256 + 128, int(tempChar->y) / 256 * 256 + 128 });
							tempChar->goal = { int(tempChar->x) / 256 * 256 + 128, int(tempChar->y) / 256 * 256 + 128 };

						}
					}

					

					//REFERANCE RIMLIKE FOLLOW FINDPATH FUNC
					
					
					
				}
				else
				{
					//SELECT CHAR
					for (int i = 0; i < Map::activeMap.entityList.characters.size(); i++)
					{
						if (Map::activeMap.entityList.characters[i].radiusCollisionDetect(getMapMousePos(), 0))
						{
							Map::activeMap.selectedCharacterID = i;
						}
						//if (Map::activeMap.entityList.characters[i].x == mousePoint.x && Map::activeMap.entityList.characters[i].y == mousePoint.y)
						//{
						//	Map::activeMap.selectedCharacterID = i;
						//}
					}

				}
			}
		}

		//ESCAPE
		if (Controller::keyboardStates[SDL_SCANCODE_ESCAPE] == 1)
		{
			switch (activeMap.menuState)
			{
			case MENU_NONE:
				//clear selection
				if (Map::activeMap.selectedCharacterID >= 0)
				{
					Map::activeMap.selectedCharacterID = -1;
				}
				else //open map menu
				{
					Map::activeMap.menuState = Map::MENU_MAP;
					loadMapMenuButtons();
				}
				break;
			case MENU_MAP:
				Map::activeMap.menuState = Map::MENU_NONE;
				break;
			}
		}

		//TEMP
		//RESTART ANIMATION
		if (Map::activeMap.selectedCharacterID >= 0 && Map::activeMap.selectedCharacterID < Map::activeMap.entityList.characters.size())
		{
			if (Controller::keyboardStates[SDL_SCANCODE_R] == 1)
			{
				Map::activeMap.entityList.characters[Map::activeMap.selectedCharacterID].animationStartTime = clock();
			}
			if (Controller::keyboardStates[SDL_SCANCODE_1] == 1)
			{
				Map::activeMap.entityList.characters[Map::activeMap.selectedCharacterID].animation = 1;
				Map::activeMap.entityList.characters[Map::activeMap.selectedCharacterID].animationStartTime = clock();
			}
			if (Controller::keyboardStates[SDL_SCANCODE_2] == 1)
			{
				Map::activeMap.entityList.characters[Map::activeMap.selectedCharacterID].animation = 2;
				Map::activeMap.entityList.characters[Map::activeMap.selectedCharacterID].animationStartTime = clock();
			}
			if (Controller::keyboardStates[SDL_SCANCODE_3] == 1)
			{
				Map::activeMap.entityList.characters[Map::activeMap.selectedCharacterID].animation = 3;
				Map::activeMap.entityList.characters[Map::activeMap.selectedCharacterID].animationStartTime = clock();
			}
		}
	}
}