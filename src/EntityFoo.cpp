#include "EntityFoo.h"
#include <math.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "globals.h"
#include "Pathfinder.h"
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

	velocity.x = 0;
	velocity.z = 0;

	if (!p_pathfinder->path.empty()) {
		const float speed = onGround ? 2.5f : 1.0f;

		Pathfinder& pathfinder = *p_pathfinder;

		PathNode* destinationNode = pathfinder.path[pathfinder.currentNodeIndex];

		if (pathfinder.currentNodeIndex != 0 && glm::distance(glm::vec3(destinationNode->pos) + glm::vec3(0.5f, 0, 0.5f), transform.position) == 0.0f) {
			destinationNode = pathfinder.path[--pathfinder.currentNodeIndex];
		}

		glm::vec3 distanceToNode = ((glm::vec3(destinationNode->pos) + glm::vec3(0.5f, 0, 0.5f)) - transform.position);
		if (onGround && distanceToNode.y > 0) {
			velocity.y += 8.0f;
		}
		distanceToNode.y = 0;

		glm::vec3 directionToNode(0, 0, 0);

		if (!glm::all(glm::equal(distanceToNode, Vectors::ZERO))) {
			directionToNode = glm::normalize(distanceToNode) * speed;
		}

		if (distanceToNode.x != 0) {
			distanceToNode.x /= CrumbleGlobals::FIXED_TIMESTEP;
		}

		if (distanceToNode.z != 0) {
			distanceToNode.z /= CrumbleGlobals::FIXED_TIMESTEP;
		}

		velocity.x = fabsf(distanceToNode.x) < fabsf(directionToNode.x) ? distanceToNode.x : directionToNode.x;
		velocity.z = fabsf(distanceToNode.z) < fabsf(directionToNode.z) ? distanceToNode.z : directionToNode.z;
	}
	else {
		transform.rotation.y += glm::radians(5.0f);
	}

	//std::cout << velocity.x << std::endl;


	Move();
}

void EntityFoo::Render(float t, GameRenderer * renderer) {
	glBindVertexArray(renderer->cubeVAO.id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderer->texture);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, transform.getInterpPos(t));

	model = glm::rotate(model, transform.rotation.x, Vectors::RIGHT);
	model = glm::rotate(model, transform.rotation.y, Vectors::UP);
	model = glm::translate(model, -glm::vec3(0.5f, 0, 0.5f));

	glUniformMatrix4fv(renderer->texturedProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));

	glDrawArrays(GL_TRIANGLES, 0, renderer->cubeVAO.vertices);
}
