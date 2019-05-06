#pragma once
#include <vector>
#include <deque>
#include <iostream>
#include <optional>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "World.h"

using namespace glm;

class PathNode {
private:

public:
	PathNode* previous = nullptr;

	ivec3 pos;
	const Face* face = nullptr;
	int distance = 0;

public:
	PathNode(ivec3 posIn) {
		pos = posIn;
		//face = nullptr;

		face = &Faces::Front;
	};

	PathNode(ivec3 posIn, PathNode* previousIn, int dist) {
		pos = posIn;
		previous = previousIn;

		face = Faces::getFace(previousIn->pos - posIn);

		distance = dist;
	};
};

class Pathfinder {
public:
	std::vector<PathNode*> path;
	std::vector<PathNode*> allNodes;
	int currentNode;

public:
	Pathfinder();
	~Pathfinder();

	void FindPath(ivec3 startPos, ivec3 endPos, int radius);
	
	int ManhattanDistance(ivec3 start, ivec3 end);
};
