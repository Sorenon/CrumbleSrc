#include "Plane.h"
#include <math.h>

#include <glm/glm.hpp>

#include "FMath.h"

Plane::Plane(glm::vec3 positionIn, glm::vec3 rotationIn) {
	position = positionIn;
	rotation = rotationIn;

	normal = FMath::getNormal(rotationIn);
	glm::vec3 distVec = -normal * positionIn;

	offset = glm::length(distVec);
	for (int i = 0; i < 3; i++) {
		const float& f = distVec[i];

		if (f != 0) {
			offset *= glm::sign(f);
		}
	}
}

glm::vec4 Plane::asVector() const {
	return glm::vec4(normal, offset);
}

float Plane::getOffset() const {
	return offset;
}

const glm::vec3& Plane::getNormal() const {
	return normal;
}

