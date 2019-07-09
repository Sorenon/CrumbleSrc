#pragma once
#include <glm/glm.hpp>

#include "FMath.h"

struct Face {
	Face() {};

	Face(glm::ivec3 normalVectorIn, glm::vec3 angleIn) {
		normalVector = normalVectorIn;
		angle = FMath::createQuaternion(angleIn);
	};

	glm::ivec3 normalVector;
	glm::quat angle;

	bool operator==(const Face other) const {
		return angle == other.angle;
	}
};

namespace Faces {
	const Face Front =  Face({ 0,  0, -1 }, { 0, 0, 0 });
	const Face Behind = Face({ 0,  0,  1 }, { 0, glm::radians(180.0f), 0 });
	const Face Right =  Face({ 1,  0,  0 }, { 0, glm::radians(270.0f), 0 });
	const Face Left =   Face({-1,  0,  0 }, { 0, glm::radians(90.0f), 0 });
	const Face Up =     Face({ 0,  1,  0 }, { glm::radians(90.0f), 0, 0 });
	const Face Down =   Face({ 0, -1,  0 }, { glm::radians(270.0f), 0, 0 });

	//const Face horizontal[] = { Front, Behind, Right, Left };
	const Face horizontal[] = { Front, Left, Behind, Right };

	const Face* getFace(glm::ivec3 dir);
};