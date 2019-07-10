#include "StandardEntityComponents.h"

#include <glm/glm.hpp>

#include "globals.h"

using namespace glm;

vec3 components::transform::getInterpPos(float t)
{
	return glm::mix(prevPosition, position, t);
}

vec3 components::transform::getInterpRot(float t)
{
	return glm::mix(prevRotation, rotation, t);
}

void components::player_movement::ApplyFriction(glm::vec3& velocity, float friction)
{
	vec3 vel = velocity;

	//We don't care about yVel in friction 
	vel.y = 0;

	const float speed = glm::length(vel);

	if (speed != 0)
	{
		const float drop = speed * friction * CrumbleGlobals::FIXED_TIMESTEP;

		vel *= (std::fmax(speed - drop, 0.0f) / speed);

		velocity.x = vel.x;
		velocity.z = vel.z;
	}
}

void components::player_movement::Walk(glm::vec3& velocity, glm::vec3 wishVel, float acceleration)
{
	wishVel.y = 0;

	if (glm::all(glm::equal(wishVel, Vectors::ZERO)))
		return;

	Accelerate(velocity, glm::normalize(wishVel), glm::length(wishVel), acceleration);
}

void components::player_movement::Accelerate(glm::vec3& velocity, glm::vec3 wishDir, float wishSpeed, float acceleration)
{
	const float currentSpeed = glm::dot(velocity, wishDir);
	const float addSpeed = wishSpeed - currentSpeed;//How much we need to accellerate to hit target speed

	if (addSpeed <= 0)//Already faster than target speed
		return;

	float accelSpeed = acceleration * CrumbleGlobals::FIXED_TIMESTEP * wishSpeed;//The max amount we can accelerate this tick

	if (accelSpeed > addSpeed)
	{
		accelSpeed = addSpeed;
	}

	velocity += (wishDir * accelSpeed);
}
