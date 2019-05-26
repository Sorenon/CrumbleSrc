#pragma once
#include <glm/glm.hpp>

typedef struct _Face {
	glm::ivec3 normalVector;
	float angle;
} Face;

namespace Faces {
	const Face Front = { { 0, 0, -1 } ,	glm::radians(0.0f) };
	const Face Behind = { { 0, 0, 1 } ,	glm::radians(180.0f) };
	const Face Right = { { 1, 0, 0 } ,	glm::radians(270.0f) };
	const Face Left = { { -1, 0, 0 } ,	glm::radians(90.0f) };

	//const Face horizontal[] = { Front, Behind, Right, Left };
	const Face horizontal[] = { Front, Left, Behind, Right };

	const Face* getFace(glm::ivec3 dir);
};