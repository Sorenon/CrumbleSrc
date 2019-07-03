#include "Plane.h"
#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

Plane::Plane(glm::vec3 positionIn, glm::quat rotationIn) : Plane(positionIn, rotationIn * glm::vec3(0, 0, -1)) { }

Plane::Plane(glm::vec3 positionIn, glm::vec3 normalIn) {
	position = positionIn;
	normal = normalIn;

	glm::vec3 distVec = -normal * positionIn;

	offset = glm::length(distVec);
	offset *= getSignFromVector(distVec);
}

glm::vec4 Plane::asVector() const {
	return glm::vec4(normal, offset);
}

float Plane::getOffset() const {
	return offset;
}

int Plane::getSignFromVector(glm::vec3 vec) {
	int sign = 1;

	for (int i = 0; i < 3; i++) {
		const float& f = vec[i];

		if (fabsf(f) >= 0.01f) {
			sign *= glm::sign(f);
		}
	}
	return sign;
}

const glm::vec3& Plane::getNormal() const {
	return normal;
}

