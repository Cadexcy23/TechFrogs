#pragma once
#ifndef vector
#include <vector>
#endif
#ifndef string
#include <string>
#endif
#ifndef Artist
#include "Artist.h"
#endif
#ifndef time
#include <time.h>
#endif
#ifndef Pathfinder
#include "Pathfinder.h"
#endif // !Pathfinder





class Character {
public:

	static struct character {
		std::string name;
		float x;
		float y;
		float xVel;
		float yVel;
		int size = 256;
		float acceleration = 1;
		float maxSpeed = 8;
		std::vector<std::vector<SDL_Texture*>> tex;
		int animation; //what animation we are in
		int animationStartTime; //what time we started the animation we are doing
		int team;
		SDL_Point goal;
		SDL_Point lastTriedGoal;
		std::vector<SDL_Point> path;
		std::vector<void (*)(Character::character* c)> updateList; //functions that the entity will run every tick of the game, need to populate this list after the ent is created

		character(float inX, float inY, std::string inName, std::vector<std::vector<SDL_Texture*>> inTex, int inTeam, float inCel, float inMS) // , std::vector<void (*)(entity* e)> upList)
		{
			name = inName;
			x = inX;
			y = inY;
			tex = inTex;
			team = inTeam;
			acceleration = inCel;
			maxSpeed = inMS;

			xVel = 0;
			yVel = 0;
			animation = 0;
			animationStartTime = clock() + rand() % 1000;
		}

		character() = default;

		bool radiusCollisionDetect(SDL_Point p, float r)
		{
			r += size / 2;
			if (size > 0 && r < 1)
			{
				r = 1;
			}
			return ((p.x - x) * (p.x - x) + (p.y - y) * (p.y - y) < r * r);
		}

		void draw(SDL_Point off, float angle = 0)
		{


			Artist artist;

			

			//TEMP
			//if enemy(team 2) flip
			SDL_RendererFlip flip = SDL_FLIP_NONE;
			if (team != 1)
			{
				flip = SDL_FLIP_HORIZONTAL;
			}

			//get half the size of the char to subtract to center it on its pos
			SDL_Point centering;
			SDL_QueryTexture(tex[0][0], NULL, NULL, &centering.x, &centering.y);
			centering.x /= 2;
			centering.y /= 2;

			//draw char
			artist.drawAnimation(tex[animation], x - off.x - centering.x, y - off.y - centering.y, 0, 0, animationStartTime, angle, Artist::FRAME_TIME, flip);

			//quarter it for the goal
			centering.x /= 4;
			centering.y /= 4;

			//draw goal
			artist.drawAnimation(tex[animation], goal.x - off.x - centering.x, goal.y - off.y - centering.y, 64, 64, animationStartTime, angle, Artist::FRAME_TIME, flip);

			//reset to idle
			//if ((clock() - animationStartTime) / 100 >= tex[animation].size())
			//{
			//	animation = 0;
			//	animationStartTime = clock();
			//}



		}

		void drawGoalPath(SDL_Point* off)
		{
			//draw line to first point on path
			if (path.size() > 0)
			{
				Artist::drawLineFromPoints(path[path.size() - 1].x - off->x, path[path.size() - 1].y - off->y, x - off->x, y - off->y);
			}
			//draw lines from each point on the path
			for (int i = 1; i < path.size(); i++)
			{
				Artist::drawLineFromPoints(path[i].x - off->x, path[i].y - off->y, path[i - 1].x - off->x, path[i - 1].y - off->y);//PROLLY REPLACE WITH A FASTER FUNC LATER and maybe thats relative to the map use ents? eh idk
				
				//make it thicker
				Artist::drawLineFromPoints(path[i].x - off->x + 1, path[i].y - off->y, path[i - 1].x - off->x + 1, path[i - 1].y - off->y);
				Artist::drawLineFromPoints(path[i].x - off->x, path[i].y - off->y + 1, path[i - 1].x - off->x, path[i - 1].y - off->y + 1);
				Artist::drawLineFromPoints(path[i].x - off->x - 1, path[i].y - off->y, path[i - 1].x - off->x - 1, path[i - 1].y - off->y);
				Artist::drawLineFromPoints(path[i].x - off->x, path[i].y - off->y - 1, path[i - 1].x - off->x, path[i - 1].y - off->y - 1);
			}
		}

		bool findPath(std::vector<std::vector<bool>> nodeMap, SDL_Point topLeft)
		{
			Pathfinder pathfinder;

			//clear any old path
			path.clear();

			//make a place to get back the paths
			std::vector<std::vector<SDL_Point>> paths;
			std::vector<std::vector<bool>> possibles;

			//get paths and possibles
			possibles = pathfinder.findPaths(nodeMap, { int(x) / 256 - topLeft.x, int(y) / 256 - topLeft.y }, &paths);//CHECK TO SEE IF THIS STOPS WHEN WE FIND THE GOAL ALMOST DEF DONT SO MAYBE MAKE IT??????

			//check if we can get to goal
			if (!possibles[int(goal.x) / 256 - topLeft.x][int(goal.y) / 256 - topLeft.y])
			{
				bool foundNewGoal = false;
				//if not find the closest open spot wiht a cap on distance MUST BE LESS THAN THE EXTRA ROOM WE GIVE TO LOOK AROUND?
				for (int out = 0; out < 9; out++)
				{
					//make a vector to hold all the spots we need to check
					std::vector<SDL_Point> checkList;

					//resize the list to hold a ring around the goal/the ring we just checked
					checkList.resize((out * 2 + 1) * 4 + 4);

					//convert the goal to a tile and the possible map
					SDL_Point goalTile = { int(goal.x) / 256 - topLeft.x, int(goal.y) / 256 - topLeft.y };

					//get the starting point on the list
					checkList[0] = { goalTile.x , goalTile.y };
					if (x < goal.x)
					{
						checkList[0].x -= (out + 1);
					}
					else if (x > goal.x)
					{
						checkList[0].x += (out + 1);
					}
					if (y < goal.y)
					{
						checkList[0].y -= (out + 1);
					}
					else if (y > goal.y)
					{
						checkList[0].y += (out + 1);
					}

					//direction we are going to fill the ring
					int dir = 0;
					//set dir to left or right dependng on what side of the ring we are on
					if (checkList[0].x < goalTile.x && checkList[0].y > goalTile.y)
					{
						dir = 2;
					}

					//get rest of list based on start
					for (int i = 1; i < checkList.size(); i++)
					{
						bool looking = true;
						//loop until we add next point
						while (looking)
						{
							//get the x/y number to add based on dir
							SDL_Point addToPos = { 0,0 };
							switch (dir)
							{
							case 0:
								addToPos = { 1,0 };
								break;
							case 1:
								addToPos = { 0,1 };
								break;
							case 2:
								addToPos = { -1,0 };
								break;
							case 3:
								addToPos = { 0,-1 };
								break;
							}

							//get the point in the direction from last point
							SDL_Point newPos = { checkList[i - 1].x + addToPos.x, checkList[i - 1].y + addToPos.y };

							//check to see if its a valid point
							if (abs(newPos.x - goalTile.x) <= out + 1 && abs(newPos.y - goalTile.y) <= out + 1)//add check to see if its a new point
							{
								checkList[i] = newPos;
								looking = false;
							}
							else
							{
								dir++;
								if (dir > 3)
									dir = 0;
							}
						}
					}
					//loop thru each spot on the list
					for (int i = 0; i < checkList.size(); i++)
					{
						//if the spot is open set it as goal
						if (possibles[checkList[i].x][checkList[i].y])
						{
							goal = { (checkList[i].x + topLeft.x) * 256 + 128, (checkList[i].y + topLeft.y) * 256 + 128 };
							i = checkList.size() + 1;
							out = 11;
							foundNewGoal = true;
						}
					}
				}
				//if no new goal found
				if (!foundNewGoal)
				{
					return false;
				}
			}

			//construct path from goal back to start
			bool pathing = true;
			//get starting point relative to the node map
			SDL_Point currentPoint = { int(goal.x) / 256 - topLeft.x, int(goal.y) / 256 - topLeft.y };
			SDL_Point mapPoint = { (currentPoint.x + topLeft.x) * 256 + 128, (currentPoint.y + topLeft.y) * 256 + 128 };
			//add it to path
			path.push_back(mapPoint);
			while (pathing)
			{
				//get next point from the path
				currentPoint = { paths[currentPoint.x][currentPoint.y].x, paths[currentPoint.x][currentPoint.y].y };
				//convert node map point to real map point
				mapPoint = { (currentPoint.x + topLeft.x) * 256 + 128, (currentPoint.y + topLeft.y) * 256 + 128 };

				//if we have reached the goal tile stop looking
				if (mapPoint.x / 256 == int(x) / 256 && mapPoint.y / 256 == int(y) / 256)
				{
					pathing = false;
				}
				else //else we add the point
				{
					path.push_back(mapPoint);
				}
			}
			return true;
		}



	};

	static void load();

};