#pragma once
#include <glm/glm.hpp>

typedef struct _Face {
	glm::ivec3 normalVector;
	glm::vec2 angle;

	bool operator==(const _Face other) {
		return angle == other.angle;
	}
} Face;

namespace Faces {
	const Face Front =  { {  0,  0, -1 } ,	{ 0, 0 } };
	const Face Behind = { {  0,  0,  1 } ,	{ 0, glm::radians(180.0f) } };
	const Face Right =  { {  1,  0,  0 } ,	{ 0, glm::radians(270.0f) } };
	const Face Left =   { { -1,  0,  0 } ,	{ 0, glm::radians(90.0f) } };
	const Face Up =     { {  0,  1,  0 } ,	{ glm::radians(90.0f), 0 } };
	const Face Down =   { {  0, -1,  0 } ,	{ glm::radians(270.0f), 0 } };

	//const Face horizontal[] = { Front, Behind, Right, Left };
	const Face horizontal[] = { Front, Left, Behind, Right };

	const Face* getFace(glm::ivec3 dir);
};