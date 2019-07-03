#pragma once
#include <vector>

#include <glm/glm.hpp>
#include <entt/entity/registry.hpp>

#include "World.h"

class Scene;
class Player;
class Entity;
class PhysicsWorld;
class GameRenderer;
class Pathfinder;

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

//TODO: delete this monstrosity
extern Scene scene;
extern entt::registry::entity_type player;
extern PhysicsWorld* p_physicsWorld;
extern GameRenderer* p_gameRenderer;
extern Pathfinder* p_pathfinder;
extern entt::registry registry;

extern int wWidth;
extern int wHeight;