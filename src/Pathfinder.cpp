#include "Pathfinder.h"

#include <glm/glm.hpp>

#include "World.h"
#include "globals.h"

using namespace glm;

Pathfinder::Pathfinder()
{
}


Pathfinder::~Pathfinder()
{
}

void Pathfinder::FloodFill(const PathNode& start, int radius) {
	frontier.push_back(start);
	PathNode pastNode = start;

	while (!frontier.empty()) {
		PathNode& current = frontier.front();

		for (const Face facing : Faces::horizontal) {
			ivec3 pos = current.pos + facing;

			if (distance(glm::vec3(start.pos), glm::vec3(pos)) > radius) {
				continue;
			}

			for (PathNode& node : visited) {
				if (node.pos == pos) {
					goto skip;
				}
			}

			for (PathNode& node : frontier) {
				if (node.pos == pos) {
					goto skip;
				}
			}

			current = start;
			frontier.push_back(PathNode(pos, current.pos));

		skip:;//GOTO
		}

		visited.push_back(current);

		frontier.pop_front();
	}

	for (PathNode &node : visited) {
		world.setBlock(node.pos.x, node.pos.y, node.pos.z, 1);
	}
}
