#pragma once
#include <vector>

#include <glm/glm.hpp>

#include "Transform.h"
#include "AABB.h"
#include "Entity.h"

using namespace glm;

class Player : public Entity {
public:
	bool noClip = false;
	bool sprinting = false;

	//AABB collider = AABB(vec3(-0.7f, 0.0f, -0.7f), vec3(0.7f, 1.9f * 2, 0.7f));
public:
	Player();

	virtual void UpdateSingleThread();
	virtual void UpdateMultiThread();
	
	void ApplyFriction(float friction);
	void WalkGround(vec3 wishVel);
	void WalkAir(vec3 wishVel);
	void Accelerate(vec3 wishDir, float wishSpeed, float acceleration);
};

