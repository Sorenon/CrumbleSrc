#include "Player.h"
#include <iostream>

#include <GLFW/glfw3.h>

#include "globals.h"

void Player::Update(GLFWwindow* window) {
	transform.step();

	velocity.y -= 18 * CrumbleGlobals::FIXED_TIMESTEP;			//Apply gravity

	//Do movementy input
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && onGround) {//TODO replace onGround with a collision check for bhopping
		velocity.y += 7.1f;
	}

	vec3 forward = transform.getForward();
	vec3 right = glm::cross(forward, Vectors::UP);
	vec3 wishVel;

	forward *= 5.0f;
	right *= 4.5f;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) wishVel += forward;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) wishVel -= forward;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) wishVel -= right;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) wishVel += right;


	if (onGround) {
		ApplyFriction(10.0f);
		WalkGround(wishVel);
	}
	else {
		ApplyFriction(1.5f);
		WalkAir(wishVel);
	}

	Move();
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

	std::cout << wishSpeed << std::endl;

	Accelerate(wishDir, wishSpeed, accel);
}

void Player::WalkAir(vec3 wishVel) {
	wishVel.y = 0;

	if (glm::all(glm::equal(wishVel, Vectors::ZERO)))
		return;

	vec3 wishDir = glm::normalize(wishVel);

	const float wishSpeed = glm::length(wishVel);
	const float accel = 5.0f;

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

void Player::Move() {
	glm::vec3 move = velocity * CrumbleGlobals::FIXED_TIMESTEP;	//How far the player expects to move 
	AABB playerCol = collider + transform.position;

	{//Collide along y axis
		const float y = move.y;

		world.clipY(playerCol, move.y);
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
		world.clipX(playerCol, move.x);
		if (x != move.x) {
			velocity.x = 0;
		}

		transform.position.x += move.x;
		playerCol = collider + transform.position;
	}

	{//Collide along z axis
		const float z = move.z;
		world.clipZ(playerCol, move.z);
		if (z != move.z) {
			velocity.z = 0;
		}

		transform.position.z += move.z;
	}
}
