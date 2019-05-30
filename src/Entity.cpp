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
	AABB clippedCol = entityCol;

	Portal& portal = scene.portals[0];
	std::vector<AABB> worldColliders = scene.mainWorld.getOverlappingBlocks(trimColliderForPortal(entityCol.expandByVelocity(move), portal, glm::vec3(0.0f))); //Find all blocks (as AABBs) the entity may collide with

	{//Collide along y axis
		const float moveY = move.y;

		entityCol = getLocalBoundingBox();
		clippedCol = trimColliderForPortal(entityCol, portal, glm::vec3(0.0f, moveY, 0.0f));

		for (AABB aabb : worldColliders) {
			aabb.clipY(clippedCol, move.y);
		}

		//portal.collider.clipY(entityCol, move.y);
		portal.collider.portalY(entityCol, move, this, portal.exit);

		if (moveY != move.y) {
			velocity.y = 0;

			if (moveY < 0.0f) {
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
	}

	{//Collide along x axis
		const float x = move.x;

		entityCol = getLocalBoundingBox();
		clippedCol = trimColliderForPortal(entityCol, portal, glm::vec3(x, 0.0f, 0.0f));

		for (AABB aabb : worldColliders) {
			aabb.clipX(clippedCol, move.x);
		}
		portal.collider.clipX(entityCol, move.x);

		if (x != move.x) {
			velocity.x = 0;
		}

		transform.prevPosition.x = transform.position.x;
		transform.position.x += move.x;
	}

	{//Collide along z axis
		const float moveZ = move.z;

		entityCol = getLocalBoundingBox();
		clippedCol = trimColliderForPortal(entityCol, portal, glm::vec3(0.0f, 0.0f, moveZ));

		for (AABB aabb : worldColliders) {
			aabb.clipZ(clippedCol, move.z);
		}
		portal.collider.clipZ(entityCol, move.z);

		if (moveZ != move.z) {
			velocity.z = 0;
		}

		//scene.portal.collider.portalZ(entityCol, move, this, scene.portal.exit);

		transform.prevPosition.z = transform.position.z;
		transform.position.z += move.z;
	}
}

AABB Entity::trimColliderForPortal(AABB entityCol, Portal& portal, vec3 move) {
	const float epsilon = 0.01f;

	if (portal.facing == Faces::Down) {
		if (portal.collider.surroundsX(entityCol) && portal.collider.surroundsZ(entityCol)) {//Is entity inline with the portal
			AABB expandedCol = entityCol;
			expandedCol.min.y += move.y;

			if (portal.collider.intersectsEpsilonY(expandedCol)) {//Is entity in the portal
				entityCol.min.y = portal.position.y - move.y + epsilon;
			}
		}
	}

	return entityCol;
}

vec3 Entity::getEyePos(float t) {
	return transform.getInterpPos(t) + eyeHeight;
}

vec3 Entity::getEyePos() {
	return transform.position + eyeHeight;
}