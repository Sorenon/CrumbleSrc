#pragma once

#include <glm/glm.hpp>

class Plane
{
private:
	glm::vec3 position;
	glm::vec3 rotation;

	glm::vec3 normal;
	float offset;
public:
	Plane(glm::vec3 position, glm::vec3 rotation);

	glm::vec4 asVector() const;
	const glm::vec3& getNormal() const;
	float getOffset() const;
};

