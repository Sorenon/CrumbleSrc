#pragma once

#include <glm/glm.hpp>
#include "Entity.h"

class EntityFoo : public Entity
{
public:
	glm::ivec3 destination = glm::ivec3(7, 64, 8);
	int pathRefresh = 20;

public:
	EntityFoo();
	~EntityFoo();

	virtual void UpdateSingleThread();
	virtual void UpdateMultiThread();

	virtual void Render(float t, GameRenderer* renderer);

	bool shouldRebuildPath();
};

