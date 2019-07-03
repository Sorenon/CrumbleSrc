#include "StandardEntityComponents.h"

vec3 components::transform::getInterpPos(float t)
{
	return glm::mix(prevPosition, position, t);
}
