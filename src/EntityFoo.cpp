#include "EntityFoo.h"
#include <math.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "globals.h"
#include "Pathfinder.h"
#include "Rendering/GameRenderer.h"
#include "Scene.h"

EntityFoo::EntityFoo() {
	//collider = AABB(vec3(-0.25f, 0, -0.25f), vec3(0.25f, 0.5f, 0.25f));
	collider = AABB(vec3(-0.49f, 0, -0.49f), vec3(0.49f, 1, 0.49f));
	
	eyeHeight = glm::vec3(0, 0.5f, 0);
}


EntityFoo::~EntityFoo() {
}

void EntityFoo::UpdateSingleThread() {

}

void EntityFoo::UpdateMultiThread() {
	transform.step();

	velocity.y -= 20 * CrumbleGlobals::FIXED_TIMESTEP;	//Apply gravity
	velocity.y *= 0.98f;

	//velocity.x = 0;
	//velocity.z = 0;

	//Pathfinder& pathfinder = *p_pathfinder;

	//if (shouldRebuildPath()) {
	//	pathfinder.FindPath(glm::floor(transform.position), destination, 16);
	//}

	//if (!pathfinder.path.empty()) {
	//	const float speed = onGround ? 2.5f : 1.0f;

	//	PathNode* destinationNode = pathfinder.path[pathfinder.currentNodeIndex];

	//	if (pathfinder.currentNodeIndex != 0) {//Find the next path node
	//		//if (glm::distance(destinationNode->getWorldPos(), transform.position) == 0.0f) {
	//		//	destinationNode = pathfinder.path[--pathfinder.currentNodeIndex];
	//		//}

	//		int nextYChange = 0;
	//		for (int i = pathfinder.currentNodeIndex; i >= 0; i--) {
	//			if (pathfinder.path[i]->pos.y != std::floor(transform.position.y)) {
	//				nextYChange = i;
	//				break;
	//			}
	//		}

	//		//float maxRadius = collider.max.x > (0.75f / 2) ? collider.max.x : 0.75f - collider.max.x;
	//		float maxRadius = collider.max.x;
	//		vec3 difference = transform.position - destinationNode->getWorldPos();

	//		if (std::abs(difference.x) < maxRadius && std::abs(difference.z) < maxRadius && std::abs(difference.y) < 1) {
	//			destinationNode = pathfinder.path[--pathfinder.currentNodeIndex];
	//		}
	//	}

	//	glm::vec3 distanceToNode = destinationNode->getWorldPos() - transform.position;
	//	if (onGround && distanceToNode.y > 0) {
	//		velocity.y += 8.0f;
	//	}
	//	distanceToNode.y = 0;

	//	glm::vec3 directionToNode(0, 0, 0);

	//	if (!glm::all(glm::equal(distanceToNode, Vectors::ZERO))) {
	//		directionToNode = glm::normalize(distanceToNode) * speed;
	//	}

	//	if (distanceToNode.x != 0) {
	//		distanceToNode.x /= CrumbleGlobals::FIXED_TIMESTEP;
	//	}

	//	if (distanceToNode.z != 0) {
	//		distanceToNode.z /= CrumbleGlobals::FIXED_TIMESTEP;
	//	}

	//	velocity.x = fabsf(distanceToNode.x) < fabsf(directionToNode.x) ? distanceToNode.x : directionToNode.x;
	//	velocity.z = fabsf(distanceToNode.z) < fabsf(directionToNode.z) ? distanceToNode.z : directionToNode.z;
	//}
	//else {
	//	//transform.rotation.y += glm::radians(5.0f);
	//}

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

	glDrawArrays(GL_TRIANGLES, 0, renderer->cubeVAO.count);


	if (!p_pathfinder->path.empty()){
		glBindVertexArray(renderer->planeVAO.id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderer->texture);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(p_pathfinder->path[p_pathfinder->currentNodeIndex]->pos) + glm::vec3(0.5f, 2, 0.5f));

		glUniformMatrix4fv(renderer->texturedProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));

		glDrawArrays(GL_TRIANGLES, 0, renderer->planeVAO.count);
	}
}

bool EntityFoo::shouldRebuildPath() {
	Pathfinder& pathfinder = *p_pathfinder;

	if (--pathRefresh <= 0 && onGround) {
		pathRefresh = 20;

		if (pathfinder.path.empty() || pathfinder.path.front()->pos != destination) {
			return true;
		}

		if (glm::distance(glm::vec3(0.5f, 0, 0.5f) + glm::vec3(pathfinder.path[pathfinder.currentNodeIndex]->pos), transform.position) > 1.5f) {
			return true;
		}

		for (int i = pathfinder.currentNodeIndex; i >= 0; i--) {
			glm::ivec3 nodePos = pathfinder.path[i]->pos;
			if (scene.mainWorld.getBlock(nodePos.x, nodePos.y, nodePos.z) != 0) {
				return true;
				break;
			}
		}
	}

	return false;
}
