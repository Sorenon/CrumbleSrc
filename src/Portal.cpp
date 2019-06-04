#include "Portal.h"

#include "AABB.h"
#include "Plane.h"
#include "globals.h"
#include "Rendering/GameRenderer.h"

Portal::Portal(glm::vec3 positionIn, glm::vec2 halfExtentsIn, Face facingIn, glm::vec3 exitIn) : collider(halfExtentsIn.x, halfExtentsIn.y, facingIn, positionIn), plane(positionIn, glm::vec3(facingIn.angle, 0)) {
	position = positionIn;
	halfExtents = halfExtentsIn;
	facing = facingIn;
	exit = exitIn;

	quadVAO = p_gameRenderer->createQuad(-halfExtentsIn.x, -halfExtentsIn.y, 0, halfExtentsIn.x * 2, halfExtentsIn.y * 2);
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
