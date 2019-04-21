#pragma once

#include <glm/glm.hpp>

#include "Transform.h"
#include "AABB.h"

using namespace glm;

class Entity {
public:
	Transform transform;
	vec3 velocity;
	AABB collider = AABB(0,0,0,0,0,0);

	bool onGround = false;
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

	AABB getLocalBoundingBox();

	void Move();
};

