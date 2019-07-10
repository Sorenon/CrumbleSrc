#pragma once
#include <float.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>

namespace FMath
{
	bool equal(float a, float b, float epsilon = FLT_EPSILON);

	//Are two floats greater than or equal to eachother
	bool greaterTorE(float larger, float smaller, float epsilon = FLT_EPSILON);

	//Are two floats less than or equal to eachother
	bool lessThanOrE(float smaller, float larger, float epsilon = FLT_EPSILON);

	glm::quat createQuaternion(glm::vec3 rotation);//Use this method to create quaternions from vectors rather than glm::quat(eulerVector)

	glm::vec3 getNormal(glm::vec3 rotation);
	glm::vec3 getForward(float yaw);
};

namespace btglmConvert
{
	glm::quat Quaternion(btQuaternion quat);
	btQuaternion Quaternion(glm::quat quat);

	glm::vec3 Vector(btVector3 vec);
	btVector3 Vector(glm::vec3 vec);
}

