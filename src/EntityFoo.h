#pragma once

#include <glm/glm.hpp>
#include "Entity.h"

class EntityFoo : public Entity {
public:
	//glm::vec3 destination = glm::vec3(5, 64, 5);

public:
	EntityFoo();
	~EntityFoo();

	virtual void UpdateSingleThread();
	virtual void UpdateMultiThread();

	virtual void Render(float t, GameRenderer* renderer);
};

