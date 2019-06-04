#pragma once
#include <float.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>

namespace FMath {
	bool equal(float a, float b, float epsilon = FLT_EPSILON);

	//Are two floats greater than or equal to eachother
	bool greaterTorE(float larger, float smaller, float epsilon = FLT_EPSILON);
	
	//Are two floats less than or equal to eachother
	bool lessThanOrE(float smaller, float larger, float epsilon = FLT_EPSILON);

	glm::vec3 convertVector(btVector3 vec);
	btVector3 convertVector(glm::vec3 vec);
	
	glm::quat convertQuaternion(btQuaternion quat);

	glm::quat createQuaternion(glm::vec3 rotation);

	glm::vec3 getNormal(glm::vec3 rotation);
};

