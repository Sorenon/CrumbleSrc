#include "Portal.h"

#include "AABB.h"
#include "Plane.h"
#include "globals.h"
#include "Rendering/GameRenderer.h"

Portal::Portal(glm::vec3 positionIn, glm::vec2 sizeIn, Face facingIn, glm::vec3 exitIn) : collider(6, 6, facingIn, positionIn),	plane(positionIn, glm::vec3(facingIn.angle, 0)){
	position = positionIn;
	size = sizeIn;
	facing = facingIn;
	exit = exitIn;

	quadVAO = p_gameRenderer->createPlane(0, 0, 0, 6, 6);
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
