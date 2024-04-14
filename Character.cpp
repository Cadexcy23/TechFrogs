#include "Character.h"
#include "Map.h"
#include "Pathfinder.h"
#include "Controller.h"


void velocity(Character::character* c)
{
	
	if (!Controller::FPSLock)
	{
		return;
	}
	c->x += c->xVel;
	c->y += c->yVel;
}

void friction(Character::character* c)
{
	if (!Controller::FPSLock)
	{
		return;
	}

	if (abs(c->xVel) < .1)
	{
		c->xVel = 0;
	}
	else
	{
		c->xVel *= 0.9;
	}
	if (abs(c->yVel) < .1)
	{
		c->yVel = 0;
	}
	else
	{
		c->yVel *= 0.9;
	}
}

void moveToGoal(Character::character* c)
{
	if (Controller::FPSLock)
	{
		//check if we even need to try moving
		if (c->x != c->goal.x || c->y != c->goal.y)
		{
			//if path is clear dont try to move
			if (c->path.size() > 0)
			{
				//get size of path so we can look at last one 
				int size = c->path.size() - 1;
				//get direction to move
				SDL_Point dir = { 0, 0 };
				if (abs(c->path[size].x - c->x))
					dir.x = (c->path[size].x - c->x) / abs(c->path[size].x - c->x);
				if (abs(c->path[size].y - c->y))
					dir.y = (c->path[size].y - c->y) / abs(c->path[size].y - c->y);

				//if we are within our speed of the path pos set it to the path pos
				if (abs(c->x - c->path[size].x) <= c->acceleration)
				{
					c->x = c->path[size].x;
				}
				else
				{
					//add dir*speed to vel
					c->x += dir.x * c->acceleration;
				}
				if (abs(c->y - c->path[size].y) <= c->acceleration)
				{
					c->y = c->path[size].y;
				}
				else
				{
					c->y += dir.y * c->acceleration;
				}

				//when we reach each point on the path clear it
				if (c->x == c->path[size].x && c->y == c->path[size].y)
				{
					c->path.pop_back();
				}
			}
		}
	}
}

bool walk(Character::character* c)
{
	//check if we are trying to move
	bool moving = false;

	if (!Controller::FPSLock)
	{
		return moving;
	}
	
	//get directions we are moving
	bool directions[4] = { 0, 0, 0, 0 };
	if (Controller::keyboardStates[SDL_SCANCODE_W])
	{
		directions[0] = true;
		moving = true;
	}
	if (Controller::keyboardStates[SDL_SCANCODE_S])
	{
		directions[1] = true;
		moving = true;
	}
	if (Controller::keyboardStates[SDL_SCANCODE_A])
	{
		directions[2] = true;
		moving = true;
	}
	if (Controller::keyboardStates[SDL_SCANCODE_D])
	{
		directions[3] = true;
		moving = true;
	}

	if (!moving)
		return moving;

	//check if accel add would put us over max speed
	float adjustedAccel = c->acceleration;
	if (abs(c->xVel) + abs(c->yVel) + c->acceleration > c->maxSpeed)
	{
		//if so reduce it to make it exactly hit max speed
		adjustedAccel = c->maxSpeed - (abs(c->xVel) + abs(c->yVel));
	}

	//get the velocity split
	int dirCount = 0;
	for (int i = 0; i < 4; i++)
	{
		if (directions[i])
		{
			dirCount++;
		}
	}
	float splitVel = 0;
	if (dirCount > 0)
		splitVel = adjustedAccel / dirCount;
	//apply vel to vector
	float vX = 0;
	float vY = 0;
	if (directions[0])
	{
		vY -= splitVel;
	}
	if (directions[1])
	{
		vY += splitVel;
	}
	if (directions[2])
	{
		vX -= splitVel;
	}
	if (directions[3])
	{
		vX += splitVel;
	}

	//convert to radians
	double theta = Map::activeMap.camRot * (M_PI / 180);

	//transform vel to be relative to the cam rot
	float addX = vX * cos(theta) + vY * sin(theta);
	float addY = vX * sin(theta) - vY * cos(theta);//CALC DIRECTIOS AND SPLIT VEL AFTER WE DO THIS INSTEAD???? almost might be easier since we can just look at the vector directins rather than counting directions

	//add it to char vel
	c->xVel += addX;
	c->yVel -= addY;

	return moving;
}

void player(Character::character* c)
{
	walk(c);
	//DEBUG
	if (Controller::FPSLock)
	{
		printf("Vel:%f , Pos:%f,%f\n", abs(c->xVel) + abs(c->yVel), c->x, c->y);
	}
	velocity(c);
	//maybe add a func that places you square on a unit with no decimal when you stop moving
	friction(c);
}

//TEMP

void Character::load()
{
	Artist artist;
	std::vector<std::vector<SDL_Texture*>> tempFrog;
	tempFrog.resize(4);
	//idle
	tempFrog[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle0.png"));
	tempFrog[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle1.png"));
	tempFrog[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle2.png"));
	tempFrog[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle3.png"));
	tempFrog[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle4.png"));
	tempFrog[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle5.png"));
	tempFrog[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle6.png"));
	tempFrog[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle7.png"));
	tempFrog[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle8.png"));
	tempFrog[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle9.png"));
	tempFrog[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle10.png"));
	tempFrog[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle11.png"));
	tempFrog[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle12.png"));
	//walk
	tempFrog[1].push_back(artist.loadTexture("Resource/characters/testFrog/frogWalk0.png"));
	tempFrog[1].push_back(artist.loadTexture("Resource/characters/testFrog/frogWalk1.png"));
	tempFrog[1].push_back(artist.loadTexture("Resource/characters/testFrog/frogWalk2.png"));
	tempFrog[1].push_back(artist.loadTexture("Resource/characters/testFrog/frogWalk3.png"));
	//get healed
	tempFrog[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal0.png"));
	tempFrog[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal1.png"));
	tempFrog[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal2.png"));
	tempFrog[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal3.png"));
	tempFrog[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal4.png"));
	tempFrog[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal5.png"));
	tempFrog[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal6.png"));
	tempFrog[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal7.png"));
	//heal
	tempFrog[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal0.png"));
	tempFrog[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal1.png"));
	tempFrog[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal2.png"));
	tempFrog[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal3.png"));
	tempFrog[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal4.png"));
	tempFrog[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal5.png"));
	tempFrog[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal6.png"));
	tempFrog[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal7.png"));


	Map::activeMap.entityList.characters.push_back(character(256 * 1 + 128, 256 * 1 + 128, "Frog", tempFrog, 1, 1, 8));
	Map::activeMap.entityList.characters[0].updateList.push_back(player);


	std::vector<std::vector<SDL_Texture*>> tempFrog2;

	tempFrog2.resize(4);
	//idle
	tempFrog2[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle0.png"));
	tempFrog2[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle1.png"));
	tempFrog2[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle2.png"));
	tempFrog2[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle3.png"));
	tempFrog2[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle4.png"));
	tempFrog2[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle5.png"));
	tempFrog2[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle6.png"));
	tempFrog2[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle7.png"));
	tempFrog2[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle8.png"));
	tempFrog2[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle9.png"));
	tempFrog2[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle10.png"));
	tempFrog2[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle11.png"));
	tempFrog2[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle12.png"));
	//walk
	tempFrog2[1].push_back(artist.loadTexture("Resource/characters/testFrog/frogWalk0.png"));
	tempFrog2[1].push_back(artist.loadTexture("Resource/characters/testFrog/frogWalk1.png"));
	tempFrog2[1].push_back(artist.loadTexture("Resource/characters/testFrog/frogWalk2.png"));
	tempFrog2[1].push_back(artist.loadTexture("Resource/characters/testFrog/frogWalk3.png"));
	//get healed
	tempFrog2[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal0.png"));
	tempFrog2[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal1.png"));
	tempFrog2[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal2.png"));
	tempFrog2[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal3.png"));
	tempFrog2[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal4.png"));
	tempFrog2[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal5.png"));
	tempFrog2[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal6.png"));
	tempFrog2[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal7.png"));
	//heal
	tempFrog2[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal0.png"));
	tempFrog2[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal1.png"));
	tempFrog2[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal2.png"));
	tempFrog2[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal3.png"));
	tempFrog2[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal4.png"));
	tempFrog2[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal5.png"));
	tempFrog2[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal6.png"));
	tempFrog2[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal7.png"));

	int R = rand() % 256, G = rand() % 256, B = rand() % 256;

	for (int j = 0; j < tempFrog2.size(); j++)
	{
		for (int i = 0; i < tempFrog2[j].size(); i++)
		{
			SDL_SetTextureColorMod(tempFrog2[j][i], R, G, B);
		}
	}

	Map::activeMap.entityList.characters.push_back(character(256 * 2 + 128, 256 * 2 + 128, "Frog2", tempFrog2, 1, 1, 8));
	Map::activeMap.entityList.characters[1].updateList.push_back(moveToGoal);
	Map::activeMap.entityList.characters[1].updateList.push_back(velocity);
	Map::activeMap.entityList.characters[1].updateList.push_back(friction);

	std::vector<std::vector<SDL_Texture*>> tempFrog3;

	tempFrog3.resize(4);
	//idle
	tempFrog3[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle0.png"));
	tempFrog3[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle1.png"));
	tempFrog3[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle2.png"));
	tempFrog3[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle3.png"));
	tempFrog3[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle4.png"));
	tempFrog3[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle5.png"));
	tempFrog3[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle6.png"));
	tempFrog3[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle7.png"));
	tempFrog3[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle8.png"));
	tempFrog3[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle9.png"));
	tempFrog3[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle10.png"));
	tempFrog3[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle11.png"));
	tempFrog3[0].push_back(artist.loadTexture("Resource/characters/testFrog/frogIdle12.png"));
	//walk
	tempFrog3[1].push_back(artist.loadTexture("Resource/characters/testFrog/frogWalk0.png"));
	tempFrog3[1].push_back(artist.loadTexture("Resource/characters/testFrog/frogWalk1.png"));
	tempFrog3[1].push_back(artist.loadTexture("Resource/characters/testFrog/frogWalk2.png"));
	tempFrog3[1].push_back(artist.loadTexture("Resource/characters/testFrog/frogWalk3.png"));
	//get healed
	tempFrog3[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal0.png"));
	tempFrog3[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal1.png"));
	tempFrog3[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal2.png"));
	tempFrog3[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal3.png"));
	tempFrog3[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal4.png"));
	tempFrog3[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal5.png"));
	tempFrog3[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal6.png"));
	tempFrog3[2].push_back(artist.loadTexture("Resource/characters/testFrog/frogGetHeal7.png"));
	//heal
	tempFrog3[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal0.png"));
	tempFrog3[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal1.png"));
	tempFrog3[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal2.png"));
	tempFrog3[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal3.png"));
	tempFrog3[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal4.png"));
	tempFrog3[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal5.png"));
	tempFrog3[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal6.png"));
	tempFrog3[3].push_back(artist.loadTexture("Resource/characters/testFrog/frogHeal7.png"));

	R = rand() % 256, G = rand() % 256, B = rand() % 256;

	for (int j = 0; j < tempFrog3.size(); j++)
	{
		for (int i = 0; i < tempFrog3[j].size(); i++)
		{
			SDL_SetTextureColorMod(tempFrog3[j][i], R, G, B);
		}
	}

	Map::activeMap.entityList.characters.push_back(character(256 * 3 + 128, 256 * 3 + 128, "Frog3", tempFrog3, 2, 2, 16));
	Map::activeMap.entityList.characters[2].updateList.push_back(moveToGoal);
	Map::activeMap.entityList.characters[2].updateList.push_back(velocity);
	Map::activeMap.entityList.characters[2].updateList.push_back(friction);
}

