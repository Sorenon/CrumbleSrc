#pragma once
#include <vector>
#include <deque>

#include <glm/glm.hpp>

#include "World.h"

using namespace glm;

struct PathNode {
	ivec3 pos;
	ivec3 posFrom;
	Face from;

public:
	PathNode(ivec3 posIn, ivec3 posFromIn) {
		pos = posIn;
		posFrom = posFromIn;
	};
};

class Pathfinder {
public:
	std::vector<PathNode> visited;
	std::deque<PathNode> frontier;

public:
	Pathfinder();
	~Pathfinder();

	void FloodFill(const PathNode& start, int radius);
};
