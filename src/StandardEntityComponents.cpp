#include "StandardEntityComponents.h"

vec3 components::transform::getInterpPos(float t)
{
	return glm::mix(prevPosition, position, t);
}

vec3 components::transform::getInterpRot(float t)
{
	return glm::mix(prevRotation, rotation, t);
}
