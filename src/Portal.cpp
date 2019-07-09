#include "Portal.h"
#include <iostream>

#include "AABB.h"
#include "Plane.h"
#include "globals.h"
#include "FMath.h"
#include "Rendering/GameRenderer.h"

Portal::Portal(glm::vec3 positionIn, glm::vec2 halfExtentsIn, Face facingIn, glm::vec3 exitIn)
	: collider(halfExtentsIn.x, halfExtentsIn.y, facingIn, positionIn), plane(exitIn, facingIn.angle) {
	position = positionIn;
	halfExtents = halfExtentsIn;
	facing = facingIn;
	exit = exitIn;

	quadVAO = p_gameRenderer->createQuad(-halfExtentsIn.x, -halfExtentsIn.y, 0, halfExtentsIn.x * 2, halfExtentsIn.y * 2);

	//glm::quat direction = FMath::createQuaternion(glm::vec3(facingIn.angle, 0.0f));
	//glm::vec3 up = direction * Vectors::UP;
	//glm::vec3 right = direction * Vectors::RIGHT;

	//plane = Plane(positionIn + up * -halfExtents.y, direction * glm::quat(glm::vec3(glm::radians(-90.0f), 0, 0)));
	//plane = Plane(positionIn + up * halfExtents.y, direction * glm::quat(glm::vec3(glm::radians(90.0f), 0, 0)));
	//plane = Plane(positionIn + right * -halfExtents.x, direction * glm::quat(glm::vec3(0, 0, glm::radians(90.0f))));
	//plane = Plane(positionIn + right * halfExtents.x, direction * glm::quat(glm::vec3(0, 0, glm::radians(-90.0f))));
}

const AABB2D& Portal::getCollider() {
	return collider;
}

const glm::vec3& Portal::getPosition() {
	return position;
}

const glm::vec3& Portal::getExit() {
	return exit;
}

const Plane& Portal::getPlane() {
	return plane;
}

const Face& Portal::getFacing() {
	return facing;
}

const t_VAO& Portal::getVAO() {
	return quadVAO;
}
