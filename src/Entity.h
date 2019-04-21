#pragma once

#include "Transform.h"

class Entity {
public:
	Transform transform;

public:
	Entity();
	~Entity();

	/*
		Block placement, Interaction with other entities
		Called First
	*/
	virtual void UpdateSingleThread() = 0;

	/*
		Try to put as much logic in here as possible

		Movement, Animations
		Called Second
	*/
	virtual void UpdateMultiThread() = 0;
};

