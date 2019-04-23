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
		glm::vec3 direction = (destination - transform.position);

		glm::vec3 directionNorm(0,0,0);

		if (!glm::all(glm::equal(direction, Vectors::ZERO))) {
			directionNorm = glm::normalize(direction);
		}

		if (direction.x != 0) {
			direction.x /= CrumbleGlobals::FIXED_TIMESTEP;
		}

		if (direction.z != 0) {
			direction.z /= CrumbleGlobals::FIXED_TIMESTEP;
		}

		velocity.x = fabsf(direction.x) < fabsf(directionNorm.x) ? direction.x : directionNorm.x;
		velocity.z = fabsf(direction.z) < fabsf(directionNorm.z) ? direction.z : directionNorm.z;
	
		std::cout << velocity.x << std::endl;
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
