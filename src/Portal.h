#pragma once

#include <glm/glm.hpp>

#include "Faces.h"
#include "Chunk.h"
#include "AABB.h"

class Portal {
public:
	glm::vec3 position;
	Face facing = Faces::Front;
	AABB2D collider = AABB2D(0, 0, Faces::Front, { 0,0,0 });

	glm::vec3 exit;

	t_VAO planeVAO;

public:
	Portal();
	~Portal();
};

