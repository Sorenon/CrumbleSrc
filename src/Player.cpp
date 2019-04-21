#include "Player.h"
#include <iostream>

#include <glm/glm.hpp>

#include "globals.h"
#include "Chunk.h"
#include "Input.h"

using namespace glm;

Player::Player() {
	transform.position = vec3(0, 90, 0);
	transform.step();

	//for (int x = 0; x < 16; x++) {
	//	for (int y = 0; y < 256; y++) {
	//		for (int z = 0; z < 16; z++) {
	//			if (chunk.getBlock(x, y, z) != 0) {
	//				AABB aabb(vec3(0, 0, 0), vec3(1, 1, 1));
	//				world.push_back(aabb + vec3(x, y, z));
	//			}
	//		}
	//	}
	//}
}

void Player::UpdateSingleThread() {
}

void Player::UpdateMultiThread() {
	transform.step();

	velocity.y -= 28 * CrumbleGlobals::FIXED_TIMESTEP;			//Apply gravity

	vec3 forward = transform.getForward();
	vec3 right = glm::cross(forward, Vectors::UP);
	vec3 wishVel;

	Input &input = Input::INSTANCE;

	if (input.kbJump.executeOnce()) {
		if (onGround) {
			velocity.y += 9.0f;
		}
	}

	if (input.kbNoClip.executeOnce()) {
		noClip = !noClip;
	}

	wishVel += forward * (input.axForward.getModifier() * (noClip ? 1.0f : sprinting ? 6.0f : 4.0f));
	wishVel += right * (input.axRight.getModifier() * (noClip ? 1.0f : 4.0f));
	wishVel += glm::vec3(0.0f, input.axUp.getModifier(), 0.0f);

	if (noClip) {
		velocity = wishVel;
		transform.position += velocity;
	} else {
		wishVel.y = 0;

		if (onGround) {
			ApplyFriction(10.0f);
			WalkGround(wishVel);
		} else {
			ApplyFriction(1.5f);
			velocity.y *= 0.98f;
			WalkAir(wishVel);
		}

		Move();
	}
}

void Player::ApplyFriction(float friction) {
	vec3 vel = velocity;

	//We don't care about yVel in friction 
	vel.y = 0;

	const float speed = glm::length(vel);

	if (speed != 0) {
		const float drop = speed * friction * CrumbleGlobals::FIXED_TIMESTEP;

		vel = vel * (std::fmax(speed - drop, 0.0f) / speed);

		velocity.x = vel.x;
		velocity.z = vel.z;
	}
}

void Player::WalkGround(vec3 wishVel) {
	wishVel.y = 0;

	if (glm::all(glm::equal(wishVel, Vectors::ZERO)))
		return;

	vec3 wishDir = glm::normalize(wishVel);

	const float wishSpeed = glm::length(wishVel);
	const float accel = 15.0f;

	Accelerate(wishDir, wishSpeed, accel);
}

void Player::WalkAir(vec3 wishVel) {
	wishVel.y = 0;

	if (glm::all(glm::equal(wishVel, Vectors::ZERO)))
		return;

	vec3 wishDir = glm::normalize(wishVel);

	const float wishSpeed = glm::length(wishVel);
	const float accel = 3.0f;

	Accelerate(wishDir, wishSpeed, accel);
}

void Player::Accelerate(vec3 wishDir, float wishSpeed, float acceleration) {
	const float currentSpeed = glm::dot(velocity, wishDir);
	const float addSpeed = wishSpeed - currentSpeed;//How much we need to accellerate to hit target speed

	if (addSpeed <= 0)//Already faster than target speed
		return;

	float accelSpeed = acceleration * CrumbleGlobals::FIXED_TIMESTEP * wishSpeed;//The max amount we can accelerate this tick

	if (accelSpeed > addSpeed) {
		accelSpeed = addSpeed;
	}

	velocity += (wishDir * accelSpeed);
}

vec3 Player::getEyePos(float t) {
	return transform.getInterpPos(t) + eyeHeight;
}

vec3 Player::getEyePos() {
	return transform.position + eyeHeight;
}

AABB Player::getLocalBoundingBox() {
	return collider + transform.position;
}

void Player::Move() {
	glm::vec3 move = velocity * CrumbleGlobals::FIXED_TIMESTEP;	//How far the player expects to move 
	AABB playerCol = collider + transform.position;

	std::vector<AABB> worldColliders;
	{//Find all blocks (as AABBs) the player may collide with
		worldColliders = world.getOverlappingBlocks(playerCol.expandByVelocity(velocity));
	}

	{//Collide along y axis
		const float y = move.y;

		for (AABB aabb : worldColliders) {
			aabb.clipY(playerCol, move.y);
		}

		celing.clipY(playerCol, move.y);

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
		playerCol = collider + transform.position;
	}

	{//Collide along x axis
		const float x = move.x;

		for (AABB aabb : worldColliders) {
			aabb.clipX(playerCol, move.x);
		}

		if (x != move.x) {
			velocity.x = 0;
		}

		transform.position.x += move.x;
		playerCol = collider + transform.position;
	}

	{//Collide along z axis
		const float z = move.z;

		for (AABB aabb : worldColliders) {
			aabb.clipZ(playerCol, move.z);
		}

		if (z != move.z) {
			velocity.z = 0;
		}

		transform.position.z += move.z;
	}
}
