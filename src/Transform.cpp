#include "Transform.h"
#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

vec3 Transform::getForward() {
	vec3 forward;
	float yaw = rotation.y;

	forward.x = (float)(sin(yaw));
	forward.z = (float)-(cos(yaw));

	return glm::normalize(forward);
}

//Static
vec3 Transform::getLook(vec3 mixedRotation) {
	vec3 front;
	float pitch = mixedRotation.x;
	float yaw = mixedRotation.y;

	front.x = sin(yaw) * cos(pitch);
	front.y = -sin(pitch);
	front.z = -(cos(yaw) * cos(pitch));
	return glm::normalize(front);
}

vec3 Transform::getLook(float t) {
	return getLook(getInterpRot(t));
}

void Transform::step() {
	prevPosition = vec3(position);
	prevRotation = vec3(rotation);
}

vec3 Transform::getInterpRot(float t) {
	return glm::mix(prevRotation, rotation, t);
}

vec3 Transform::getInterpPos(float t) {
	return glm::mix(prevPosition, position, t);
}
