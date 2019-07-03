#pragma once
#include <glm/glm.hpp>

#include "AABB.h"

using namespace glm;

namespace components {
	struct transform {
		vec3 prevPosition;
		vec3 prevRotation;

		vec3 position;
		vec3 rotation;

	public:
		vec3 getInterpPos(float t);
		vec3 getInterpRot(float t); //No need to call this for player because player rotation is updated every frame
	};

	struct kinematic_ridgedbody {
		float eyeHeight = 0.5f; //Center of mass
		AABB collider;
		float gravity = 20.0f; // m/s^2
		float drag = 0.98f;

		vec3 velocity; //Velocity in m/s
		bool onGround = false;
	};

	struct renderable {
		//TODO: model variable
		//Could split this component into animated renderable and static renderable, etc.
	};

	struct player_movement {
		bool noClip = false;
	};
};