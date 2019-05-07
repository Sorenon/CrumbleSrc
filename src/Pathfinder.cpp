#include "Pathfinder.h"
#include <deque>
#include <iostream>

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

//void Pathfinder::FindPath(ivec3 startPos, ivec3 endPos, int radius) {
//	std::vector<PathNode*> visited;
//	std::deque<PathNode*> frontier;
//
//	frontier.push_back(new PathNode(startPos));
//
//	while (!frontier.empty()) {
//		std::cout << "scan";
//		PathNode& currentNode = *frontier.front();
//		frontier.pop_front();
//
//		//if (currentNode.previous.has_value()) {
//		//	if (currentNode.pos == currentNode.previous->pos) {
//		//		std::cout << "HTGAT";
//		//	}
//		//}
//
//		for (const Face facing : Faces::horizontal) {
//			ivec3 checkPos = currentNode.pos + facing.vec;
//
//			if (checkPos == endPos) {//Found the path: currentNode == endNode
//				path.push_back(new PathNode(checkPos, currentNode, 0));
//				path.push_back(currentNode);
//
//				//for (PathNode& node : visited) {
//				//	if (node.previous != nullptr) {
//				//		if (node.pos != node.previous->pos) {
//				//			path.push_back(PathNode(node.pos, node.previous, node.distance));
//				//		}
//				//	}
//
//				//	//path.push_back(node);
//				//}
//
//				/*PathNode* reading = &currentNode;
//				if (reading == reading->previous) {
//					std::cout << glm::to_string(reading->pos) << std::endl;
//				}*/
//
//				//PathNode reading = currentNode;
//				//while (reading.previous != nullptr) {
//				//	if (reading.pos == reading.previous->pos) {
//				//		std::cout << glm::to_string(reading.pos) << std::endl;
//				//		break;
//				//	}
//
//				//	reading = *reading.previous;
//				//	path.push_back(PathNode(reading.pos, reading.previous, reading.distance));
//				//}
//
//				return;
//			}
//		
//			if (ManhattanDistance(startPos, checkPos) > radius) {//Out of search radius
//				continue;
//			}
//
//			for (PathNode& node : visited) {//Have we already visited this node
//				if (node.pos == checkPos) {
//					goto skip;
//				}
//			}
//
//			for (PathNode& node : frontier) {//TODO: merge this into bellow loop
//				if (node.pos == checkPos) {
//					goto skip;
//				}
//			}
//
//			int distFromEnd = ManhattanDistance(checkPos, endPos);
//
//			//for (auto it = frontier.begin(); it != frontier.end(); it++) {
//			//	if (distFromEnd < (*it).distance) {//Order the array: Scan the nodes closest to the end first
//			//		frontier.insert(it, PathNode(checkPos, &currentNode, distFromEnd));
//			//		goto skip;
//			//	}
//			//}
//
//			PathNode node = PathNode(checkPos, currentNode, distFromEnd);
//			frontier.push_back(node);
//		skip:;//GOTO
//		}
//
//		visited.push_back(currentNode);
//	}
//
//}

void Pathfinder::FindPath(ivec3 startPos, ivec3 endPos, int radius) {
	for (PathNode* node : allNodes) {
		delete node;
	}
	allNodes = std::vector<PathNode*>();
	path = std::vector<PathNode*>();

	std::vector<PathNode*> visited;
	std::deque<PathNode*> frontier;

	frontier.push_back(new PathNode(startPos));

	int i = 0;

	while (!frontier.empty()) {
		PathNode* currentNode = frontier.front();
		frontier.pop_front();

		for (const Face& face : Faces::horizontal) {
			ivec3 checkPos = currentNode->pos + face.vec;

			if (world.getBlock(checkPos.x, checkPos.y, checkPos.z) != 0) {
				checkPos.y++;

				if (world.getBlock(checkPos.x, checkPos.y, checkPos.z) != 0) {
					continue;
				}
			}
			else if (world.getBlock(checkPos.x, checkPos.y - 1, checkPos.z) == 0) {
				checkPos.y--;
			}

			if (checkPos == endPos) {//Found the path
				PathNode* newNode = new PathNode(checkPos, currentNode, i++);
				path.push_back(newNode);
				path.push_back(currentNode);
				visited.push_back(newNode);
				visited.push_back(currentNode);

				PathNode* reading = currentNode;
				while (reading->previous != nullptr) {//Follow the path back to the start
					reading = reading->previous;
					path.push_back(reading);
				}

				allNodes = visited;
				currentNodeIndex = path.size() - 1;

				return;
			}

			if (ManhattanDistance(startPos, checkPos) > radius) {//Out of search radius
				continue;
			}

			for (PathNode* node : visited) {//Have we already visited this node
				if (node->pos == checkPos) {
					goto skip;
				}
			}

			for (PathNode* node : frontier) {//TODO: merge this into bellow loop
				if (node->pos == checkPos) {
					goto skip;
				}
			}

			PathNode* newNode = new PathNode(checkPos, currentNode, i++);
			frontier.push_back(newNode);

		skip:;//GOTO
		}

		visited.push_back(currentNode);
	}

	allNodes = visited;
	currentNodeIndex = 0;
}

int Pathfinder::ManhattanDistance(ivec3 start, ivec3 end) {
	return fabs(start.x - end.x) + fabs(start.z - end.z);
}
