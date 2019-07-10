#include "StandardEntitySystems.h"

#include <glm/glm.hpp>
#include <entt/entity/registry.hpp>

#include "StandardEntityComponents.h"
#include "globals.h"
#include "Scene.h"
#include "Input.h"

void standard_entity_systems::update_pathfinder(entt::registry& registry)
{

}

void standard_entity_systems::do_path_movement(entt::registry& registry)
{

}

void standard_entity_systems::move_kinematic_ridgedbodies(entt::registry& registry)
{
	auto view = registry.view<components::transform, components::kinematic_ridgedbody>();

	view.each([](auto entity, auto& trans, auto& rb)
		{
			rb.velocity.y -= rb.gravity * CrumbleGlobals::FIXED_TIMESTEP;
			rb.velocity *= rb.drag;

			glm::vec3 move = rb.velocity * CrumbleGlobals::FIXED_TIMESTEP;	//How far the entity expects to move 

			if (rb.noClip)
			{
				trans.prevPosition = trans.position;
				trans.position += move;
			}
			else
			{
				AABB entityCol = rb.collider + trans.position;
				AABB clippedCol = entityCol;

				std::vector<AABB> worldColliders = trans.scene->mainWorld.getOverlappingBlocks(entityCol.expandByVelocity(move)); //Find all blocks (as AABBs) the entity may collide with

				{//Collide along y axis
					const float moveY = move.y;

					entityCol = rb.collider + trans.position;

					for (AABB aabb : worldColliders)
					{
						aabb.clipY(entityCol, move.y);
					}

					if (moveY != move.y)
					{
						rb.velocity.y = 0;

						if (moveY < 0.0f)
						{
							rb.onGround = true;
						}
						else
						{
							rb.onGround = false;
						}
					}
					else
					{
						rb.onGround = false;
					}

					trans.prevPosition.y = trans.position.y;
					trans.position.y += move.y;
				}

				{//Collide along x axis
					const float x = move.x;

					entityCol = rb.collider + trans.position;

					for (AABB aabb : worldColliders)
					{
						aabb.clipX(entityCol, move.x);
					}

					if (x != move.x)
					{
						rb.velocity.x = 0;
					}

					trans.prevPosition.x = trans.position.x;
					trans.position.x += move.x;
				}

				{//Collide along z axis
					const float moveZ = move.z;

					entityCol = rb.collider + trans.position;

					for (AABB aabb : worldColliders)
					{
						aabb.clipZ(entityCol, move.z);
					}

					if (moveZ != move.z)
					{
						rb.velocity.z = 0;
					}

					trans.prevPosition.z = trans.position.z;
					trans.position.z += move.z;
				}
			}
		});
}

void standard_entity_systems::do_player_movement(entt::registry& registry)
{
	auto view = registry.view<components::transform, components::kinematic_ridgedbody, components::player_movement>();

	view.each([](auto entity, auto& trans, auto& rb, auto& pm)
		{
			vec3 forward = FMath::getForward(trans.rotation.y);
			vec3 right = glm::cross(forward, Vectors::UP);
			vec3 wishVel;

			Input& input = Input::INSTANCE;

			if (input.kbJump.executeOnce())
			{
				if (rb.onGround)
				{
					rb.velocity.y += 9.0f;
				}
			}

			if (input.kbNoClip.executeOnce())
			{
				rb.noClip = !rb.noClip;

				if (rb.noClip)
				{
					rb.gravity = 0.0f;
					rb.drag = 1.0f;
				}
				else
				{
					rb.gravity = 28.0f;
					rb.drag = 0.98f;
				}
			}

			const float moveSpeed = rb.noClip ? pm.noClipSpeed : pm.walkSpeed;

			wishVel += forward * (input.axForward.getModifier() * moveSpeed);
			wishVel += right * (input.axRight.getModifier() * moveSpeed);
			wishVel += glm::vec3(0.0f, input.axUp.getModifier() * moveSpeed, 0.0f);

			if (rb.noClip)
			{
				rb.velocity = wishVel;
			}
			else
			{
				if (rb.onGround)
				{
					components::player_movement::ApplyFriction(rb.velocity, 10.0f);
					components::player_movement::Walk(rb.velocity, wishVel, 15.0f);
				}
				else
				{
					components::player_movement::ApplyFriction(rb.velocity, 1.5f);
					components::player_movement::Walk(rb.velocity, wishVel, 3.0f);
				}
			}
		});

}
