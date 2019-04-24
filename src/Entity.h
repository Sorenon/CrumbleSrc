#pragma once

#include <glm/glm.hpp>

#include "Transform.h"
#include "AABB.h"

using namespace glm;

class GameRenderer;

class Entity {
public:
	Transform transform;
	vec3 velocity; //Velocity in m/s
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

		There could be a possible issue with bounding boxes overlapping incorrectly
		(Possible sollution: Do movement in two phases)
		 1. Limit velocity by world and create expanded AABB //Multithreaded
		 2. Collide with other expanded AABBs //Single threaded
	*/
	virtual void UpdateMultiThread() = 0;

	AABB getLocalBoundingBox();
	void Move();

	virtual void Render(float t, GameRenderer* renderer) {}
};

