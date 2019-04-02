#include "Player.h"
#include <GLFW/glfw3.h>

#include "globals.h"

void Player::Update(GLFWwindow* window) {
	transform.step();

	//update(window);
	velocity.y -= 18 * FIXED_TIMESTEP;			//Apply gravity

	{//Do movementy input
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && onGround) {//TODO replace onGround with a collision check for bhopping
			velocity.y += 7.1f;
		}

		vec3 forward = transform.getForward();
		vec3 right = glm::cross(forward, Vectors::UP);
		vec3 wishVel;

		forward *= 4.0f;
		right *= 3.8f;

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) wishVel += forward;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) wishVel -= forward;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) wishVel -= right;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) wishVel += right;

		velocity.x = wishVel.x;
		velocity.z = wishVel.z;
	}

	Move();
}

void Player::ApplyFriction(float friction) {
}

void Player::WalkGround(vec3 wishDir) {
}

void Player::WalkAir(vec3 wishDir) {
}

void Player::Accelerate(vec3 wishDir, float wishSpeed, float acceleration) {
}

void Player::Move() {
	glm::vec3 move = velocity * FIXED_TIMESTEP;	//How far the player expects to move 
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
