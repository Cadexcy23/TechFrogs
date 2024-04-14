#pragma once
#ifndef Artist
#include "Artist.h"
#endif
#ifndef vector
#include <vector>
#endif

class Pathfinder {
public:

	static std::vector<std::vector<bool>> findPaths(std::vector<std::vector<bool>> nodeMap, SDL_Point start, std::vector<std::vector<SDL_Point>>* paths);

};