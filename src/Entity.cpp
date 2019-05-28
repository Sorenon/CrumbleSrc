#include "Entity.h"
#include <vector>
#include <iostream>

#include <glm/glm.hpp>

#include "globals.h"
#include "AABB.h"
#include "Scene.h"
#include "FMath.h"
#include "Portal.h"

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

	Portal& portal = scene.portals[0];

	{//Collide along y axis
		const float y = move.y;

		std::cout << portal.collider.intersectsYportal(entityCol) << std::endl;

		for (AABB aabb : worldColliders) {
			aabb.clipY(entityCol, move.y);
		}

		//portal.collider.clipY(entityCol, move.y);
		portal.collider.portalY(entityCol, move, this, portal.exit);

		if (y != move.y) {
			velocity.y = 0;

			if (y < 0.0f) {
				onGround = true;
			}
			else {
				onGround = false;
			}
		}
		else {
			onGround = false;
		}

		transform.prevPosition.y = transform.position.y;
		transform.position.y += move.y;
		entityCol = getLocalBoundingBox();
	}

	{//Collide along x axis
		const float x = move.x;

		for (AABB aabb : worldColliders) {
			aabb.clipX(entityCol, move.x);
		}
		portal.collider.clipX(entityCol, move.x);

		if (x != move.x) {
			velocity.x = 0;
		}

		transform.prevPosition.x = transform.position.x;
		transform.position.x += move.x;
		entityCol = getLocalBoundingBox();
	}

	{//Collide along z axis
		const float z = move.z;

		for (AABB aabb : worldColliders) {
			aabb.clipZ(entityCol, move.z);
		}
		//scene.portal.collider.clipZ(entityCol, move.z);

		if (z != move.z) {
			velocity.z = 0;
		}

		//scene.portal.collider.portalZ(entityCol, move, this, scene.portal.exit);

		transform.prevPosition.z = transform.position.z;
		transform.position.z += move.z;
	}
}

vec3 Entity::getEyePos(float t) {
	return transform.getInterpPos(t) + eyeHeight;
}

vec3 Entity::getEyePos() {
	return transform.position + eyeHeight;
}