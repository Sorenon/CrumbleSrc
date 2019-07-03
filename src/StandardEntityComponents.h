#pragma once
#include <glm/glm.hpp>

#include "AABB.h"
#include "Pathfinder.h"

using namespace glm;

class Scene;

namespace components {
	struct transform {
		vec3 prevPosition;
		vec3 prevRotation;

		vec3 position;
		vec3 rotation;

		Scene* scene = nullptr;

	public:
		vec3 getInterpPos(float t);
		vec3 getInterpRot(float t); //No need to call this for localplayer because localplayer rotation is updated every frame
	};

	struct kinematic_ridgedbody {
		float eyeHeight = 0.5f; //Center of mass
		AABB collider;
		float gravity = 20.0f; // m/s^2
		float drag = 0.98f;
		bool noClip = false;

		vec3 velocity; //Velocity in m/s
		bool onGround = false;
	};

	struct renderable {
		//TODO: model variable
		//Could split this component into animated renderable and static renderable, etc.
	};

	struct player_movement {
		float noClipSpeed = 10.0f;
		float walkSpeed = 4.0f;

	public:
		static void ApplyFriction(vec3& velocity, float friction);
		static void Walk(vec3& velocity, vec3 wishVel, float acceleration);
		static void Accelerate(vec3& velocity, vec3 wishDir, float wishSpeed, float acceleration);
	};

	struct pathfinder {
		ivec3 destination;
		Pathfinder pathfinder;
		int pathRefreshCooldown = 30.0f;
	};

	struct pathfinding_movement {

	};
};