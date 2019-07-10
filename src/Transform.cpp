#include "Transform.h"
#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "FMath.h"

using namespace glm;

vec3 Transform::getForward()
{
	vec3 forward;
	float yaw = rotation.y;

	forward.x = (float)(sin(yaw));
	forward.z = (float)-(cos(yaw));

	return glm::normalize(forward);
}

vec3 Transform::getLook(float t)
{
	return FMath::getNormal(getInterpRot(t));
}

void Transform::step()
{
	prevPosition = position;
	prevRotation = rotation;
}

vec3 Transform::getInterpRot(float t)
{
	return glm::mix(prevRotation, rotation, t);
}

vec3 Transform::getInterpPos(float t)
{
	return glm::mix(prevPosition, position, t);
}
