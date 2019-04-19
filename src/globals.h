#pragma once

#include <glm/glm.hpp>

#include "World.h"

class Player;

namespace Vectors {
	const glm::vec3 UP = glm::vec3(0, 1, 0);
	const glm::vec3 FORWARD = glm::vec3(0, 0, -1);
	const glm::vec3 RIGHT = glm::vec3(1, 0, 0);
	const glm::vec3 ZERO = glm::vec3(0, 0, 0);
};

namespace CrumbleGlobals {
	const float FIXED_TIMESTEP = 1.0f / 30.0f;
	const float PHYSICS_TIMESTEP = 1.0f / 120.0f;
};

extern World world;
extern Player* p_player;

extern int wWidth;
extern int wHeight;