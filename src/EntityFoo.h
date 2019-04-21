#pragma once

#include "Entity.h"

class EntityFoo : public Entity {
public:
	EntityFoo();
	~EntityFoo();

	virtual void UpdateSingleThread();
	virtual void UpdateMultiThread();
};

