#pragma once

#include <glm/glm.hpp>

#include "Faces.h"
#include "Chunk.h"
#include "AABB.h"
#include "Plane.h"

class Portal {
private:
	glm::vec3 position;
	glm::vec2 halfExtents;
	glm::vec3 exit;

	Face facing;
	AABB2D collider;

	t_VAO quadVAO;
	Plane plane;
public:
	Portal(glm::vec3 position, glm::vec2 halfExtents, Face facing, glm::vec3 exit);
	
	const AABB2D& getCollider();
	const glm::vec3& getPosition();
	const glm::vec3& getExit();
	const Plane& getPlane();
	const Face& getFacing();
	const t_VAO& getVAO();
};

