#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "FMath.h"

class Plane
{
private:
	glm::vec3 position;

	glm::vec3 normal;
	float offset;
public:
	Plane(glm::vec3 position = glm::vec3(), glm::quat rotation = glm::quat());
	Plane(glm::vec3 position, glm::vec3 normal);

	//Plane(glm::vec3 position, glm::vec3 rotation) : Plane(position, FMath::createQuaternion(rotation)) {};

	glm::vec4 asVector() const;
	const glm::vec3& getNormal() const;
	float getOffset() const;

	static int getSignFromVector(glm::vec3 vec);
};

