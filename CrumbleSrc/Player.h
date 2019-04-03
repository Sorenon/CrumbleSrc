#pragma once
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Transform.h"
#include "AABB.h"

using namespace glm;

class Player {
public:
	Transform transform;
	vec3 velocity;

	bool noClip = false;
	bool onGround = false;

	//std::vector<AABB> world;
	AABB celing = AABB(vec3(-2, -1, -2), vec3(7, 0, 7));

	AABB collider = AABB(vec3(-0.4, 0, -0.4), vec3(0.4, 1.8, 0.4));
public:
	Player();

	void Update(GLFWwindow* window);
	
	void ApplyFriction(float friction);
	void WalkGround(vec3 wishVel);
	void WalkAir(vec3 wishVel);
	void Accelerate(vec3 wishDir, float wishSpeed, float acceleration);

	void Move();
};

