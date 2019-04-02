#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Transform.h"
#include "AABB.h"

using namespace glm;

class Player {
public:
	Transform transform;
	vec3 velocity;

	bool onGround = false;

	AABB world = AABB(vec3(-5, -1, -5), vec3(0, 0, 0));
	AABB celing = AABB(vec3(-7, -2, -7), vec3(2, -1, 2));

	AABB collider = AABB(vec3(-0.4, 0, -0.4), vec3(0.4, 1.8, 0.4));
public:
	void Update(GLFWwindow* window);
	
	void ApplyFriction(float friction);
	void WalkGround(vec3 wishDir);
	void WalkAir(vec3 wishDir);
	void Accelerate(vec3 wishDir, float wishSpeed, float acceleration);

	void Move();
};

