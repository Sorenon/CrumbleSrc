#include "FMath.h"
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>

bool FMath::equal(float a, float b, float epsilon) {
	return (fabs(a - b) <= epsilon * std::max(fabs(a), fabs(b)));
}

bool FMath::greaterTorE(float larger, float smaller, float epsilon) {
	return larger >= smaller || equal(smaller, larger, epsilon);
}

bool FMath::lessThanOrE(float smaller, float larger, float epsilon) {
	return larger >= smaller || equal(smaller, larger, epsilon);
}

glm::vec3 FMath::convertVector(btVector3 vec) {
	return glm::vec3(vec.getX(), vec.getY(), vec.getZ());
}

btVector3 FMath::convertVector(glm::vec3 vec) {
	return btVector3(vec.x, vec.y, vec.z);
}

glm::quat FMath::convertQuaternion(btQuaternion quat) {
	return glm::quat(quat.getX(), quat.getY(), quat.getZ(), quat.getW());
}

glm::quat FMath::createQuaternion(glm::vec3 rotation) {
	return glm::quat(glm::vec3(rotation.x, 0, 0)) * glm::quat(glm::vec3(0, rotation.y, 0)) * glm::quat(glm::vec3(0, 0, rotation.z));
	// possibly faulty expliantion -> //x (yaw) is around world axis while y & z (yaw & roll) is around local axis
}

glm::vec3 FMath::getNormal(glm::vec3 rotation) {
	glm::vec3 front;
	float pitch = rotation.x;
	float yaw = rotation.y;

	front.x = sin(yaw) * cos(pitch);
	front.y = -sin(pitch);
	front.z = -(cos(yaw) * cos(pitch));
	return glm::normalize(front);
}


