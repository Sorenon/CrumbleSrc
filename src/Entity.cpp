#include "Entity.h"
#include <vector>
#include <iostream>

#include <glm/glm.hpp>

#include "globals.h"
#include "AABB.h"
#include "Scene.h"
#include "FMath.h"

Entity::Entity() {
}


Entity::~Entity() {
}

AABB Entity::getLocalBoundingBox() {
	return collider + transform.position;
}

void Entity::Move() {
	glm::vec3 move = velocity * CrumbleGlobals::FIXED_TIMESTEP;	//How far the entity expects to move 
	AABB entityCol = getLocalBoundingBox();

	std::vector<AABB> worldColliders = scene.mainWorld.getOverlappingBlocks(entityCol.expandByVelocity(velocity)); //Find all blocks (as AABBs) the entity may collide with

	{//Collide along y axis
		const float y = move.y;

		for (AABB aabb : worldColliders) {
			aabb.clipY(entityCol, move.y);
		}

		if (y != move.y) {
			velocity.y = 0;

			if (y < 0.0f) {
				onGround = true;
			} else {
				onGround = false;
			}
		} else {
			onGround = false;
		}

		transform.position.y += move.y;
		entityCol = getLocalBoundingBox();
	}

	{//Collide along x axis
		const float x = move.x;

		for (AABB aabb : worldColliders) {
			aabb.clipX(entityCol, move.x);
		}

		if (x != move.x) {
			velocity.x = 0;
		}

		transform.position.x += move.x;
		entityCol = getLocalBoundingBox();
	}

	{//Collide along z axis
		const float z = move.z;

		for (AABB aabb : worldColliders) {
			aabb.clipZ(entityCol, move.z);
		}

		if (z != move.z) {
			velocity.z = 0;
		}

		const float oldZPos = transform.position.z;

		if (FMath::greaterTorE(transform.position.z, scene.portal.position.z) && FMath::lessThanOrE(transform.position.z + move.z, scene.portal.position.z)) {
			float diffZ = scene.portal.position.z - transform.position.z;
			//std::cout << diffZ - move.z << std::endl;
			transform.position.z = scene.portal.exit.z - 3 - (diffZ - move.z);
			transform.prevPosition.z = transform.position.z - move.z;
		}
		else {
			transform.position.z += move.z;
		}

		const float newZPos = transform.position.z;
		//std::cout << oldZPos - newZPos << std::endl;
	}
}