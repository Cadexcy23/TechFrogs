#pragma once
#ifndef SDL
#include <SDL.h>
#endif // !SDL
#ifndef vector
#include <vector>
#endif
#ifndef string
#include <string>
#endif
#ifndef Character
#include "Character.h"
#endif

class Map {
public:

	struct masterTile {
		SDL_Texture* tex;
		std::string name;
		bool collision;
		//whatever else
	};

	struct tile {
		masterTile* tileType;
		bool roofed;
		//other info?
	};

	struct entityList {
		std::vector<Character::character> characters;
		//std::vector<Entity::pawn> pawns;
		//std::vector<Entity::projectile> projectiles;
	};

	static struct map {
		std::vector<std::vector<tile>> tileGrid;
		std::vector<masterTile> masterTileList;
		SDL_Texture* backgroundTiles;
		SDL_Texture* backgroundFinal;
		std::vector<SDL_Texture*> selector;
		SDL_Texture* mapMenuHead;
		SDL_Texture* mapMenuFoot;
		SDL_Texture* mapCharacterHead;
		SDL_Texture* mapCharacterFoot;
		SDL_Texture* tileInfoPanel;
		std::vector<SDL_Texture*> pathGraphics;
		int menuState;
		int selectedCharacterID;
		SDL_Point tileSelectorPos;
		int renderDist;
		double camRot;
		int camRotGoal;
		float camPosX;
		float camPosY;
		float camPosXGoal;
		float camPosYGoal;
		float camZoom;
		float camZoomGoal;
		SDL_Point camOffset;
		entityList entityList;
	};


	enum mapStates {
		MENU_NONE,
		MENU_MAP,
		
	};

	static Map::map activeMap;

	static std::vector<std::vector<bool>> getMapChunkCollision(SDL_Point topLeft, SDL_Point size);

	void loadMap(std::string path);

	void draw();

	static void update();

	void controller();

};