#pragma once

#include <entt/entity/registry.hpp>

namespace standard_entity_systems {
	//TODO: have a dependancy system for systems (e.g. move_kinematic_ridgedbodies can only occur after do_player_movement)


	void update_pathfinder(entt::registry& registry);//pathfinding in crumble is awfull atm, i will reimpliment the features at a later date

	void do_path_movement(entt::registry& registry);
	void do_player_movement(entt::registry& registry);//This only affects one entity so would it be better to not have an entire system for this?

	void move_kinematic_ridgedbodies(entt::registry& registry);
};