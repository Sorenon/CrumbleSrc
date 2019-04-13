#pragma once
#include <vector>

#include <glm/glm.hpp>

#include "Transform.h"
#include "AABB.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

using namespace glm;

class Player {
public:
	Transform transform;
	vec3 velocity;
	vec3 eyeHeight = vec3(0.0f, 1.8f, 0.0f);

	bool noClip = false;
	bool onGround = false;
	bool sprinting = false;

	AABB celing = AABB(vec3(-2, -1, -2), vec3(7, 0, 7));

	AABB collider = AABB(vec3(-0.4, 0, -0.4), vec3(0.4, 1.9, 0.4));
public:
	Player();

	void Update(GLFWwindow* window);
	
	void ApplyFriction(float friction);
	void WalkGround(vec3 wishVel);
	void WalkAir(vec3 wishVel);
	void Accelerate(vec3 wishDir, float wishSpeed, float acceleration);

	vec3 getEyePos(float t);
	vec3 getEyePos();

	AABB getLocalBoundingBox();

	void Move();
};

