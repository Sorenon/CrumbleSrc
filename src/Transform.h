#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

class Transform
{
public:
	vec3 prevPosition;
	vec3 prevRotation;

	vec3 position;
	vec3 rotation;

public:
	vec3 getForward();
	vec3 getLook(float t = 1.0f);

	void step();

	vec3 getInterpPos(float t);
	vec3 getInterpRot(float t);
};

