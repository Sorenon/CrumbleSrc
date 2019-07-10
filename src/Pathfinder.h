#pragma once
#include <vector>
#include <deque>
#include <iostream>
#include <optional>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "World.h"

using namespace glm;

class PathNode
{
private:

public:
	PathNode* previous = nullptr;

	ivec3 pos;
	const Face* face = nullptr;
	int accumulatedCost = 0;
	int priority = 0;
	bool inPath = false;

public:
	PathNode(ivec3 posIn)
	{
		pos = posIn;
	};

	PathNode(ivec3 posIn, PathNode* previousIn, int accumulatedCostIn = -1, int priorityIn = -1)
	{
		pos = posIn;
		previous = previousIn;

		vec3 diff = previousIn->pos - posIn;
		diff.y = 0;
		face = Faces::getFace(diff);

		accumulatedCost = accumulatedCostIn;
		priority = priorityIn;
	};

	vec3 getWorldPos()
	{
		return vec3(pos) + glm::vec3(0.5f, 0, 0.5f);
	};
};

class Scene;

class Pathfinder
{
public:
	//Scene* scene;
	std::vector<PathNode*> path;
	std::vector<PathNode*> closedSet;
	int currentNodeIndex;

public:
	Pathfinder();
	~Pathfinder();

	void FindPath(ivec3 startPos, ivec3 endPos, int radius);

	int ManhattanDistance(ivec3 start, ivec3 end);
};
