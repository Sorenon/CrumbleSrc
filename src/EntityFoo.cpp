#include "EntityFoo.h"
#include <math.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "globals.h"
#include "Rendering/GameRenderer.h"

EntityFoo::EntityFoo() {
	//collider = AABB(vec3(-0.25f, 0, -0.25f), vec3(0.25f, 0.5f, 0.25f));
	collider = AABB(vec3(-0.5f, 0, -0.5f), vec3(0.5f, 1, 0.5f));
}


EntityFoo::~EntityFoo() {
}

void EntityFoo::UpdateSingleThread() {

}

void EntityFoo::UpdateMultiThread() {
	transform.step();

	velocity.y -= 20 * CrumbleGlobals::FIXED_TIMESTEP;			//Apply gravity
	velocity.y *= 0.98f;

	if (onGround) {
		float speed = 2.5f;

		glm::vec3 distance = (destination - transform.position);
		distance.y = 0;

		glm::vec3 direction(0,0,0);

		if (!glm::all(glm::equal(distance, Vectors::ZERO))) {
			direction = glm::normalize(distance) * speed;
		}

		if (distance.x != 0) {
			distance.x /= CrumbleGlobals::FIXED_TIMESTEP;
		}

		if (distance.z != 0) {
			distance.z /= CrumbleGlobals::FIXED_TIMESTEP;
		}

		velocity.x = fabsf(distance.x) < fabsf(direction.x) ? distance.x : direction.x;
		velocity.z = fabsf(distance.z) < fabsf(direction.z) ? distance.z : direction.z;
	
		//std::cout << velocity.x << std::endl;
	}

	Move();
}

void EntityFoo::Render(float t, GameRenderer * renderer) {
	glBindVertexArray(renderer->cubeVAO.id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderer->texture);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, transform.getInterpPos(t));
	model = glm::translate(model, -glm::vec3(0.5f, 0, 0.5f));
	glUniformMatrix4fv(renderer->texturedProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));

	glDrawArrays(GL_TRIANGLES, 0, renderer->cubeVAO.vertices);
}
