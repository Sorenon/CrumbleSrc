#pragma once
#include <glad/glad.h>
#include <string>
#include "ShaderConstructor.h"

class ShaderProgram {
public:
	GLuint id;

public:
	ShaderProgram(ShaderConstructor &vertex, ShaderConstructor &fragment);
	ShaderProgram(ShaderConstructor vertex, ShaderConstructor fragment);
	~ShaderProgram();

	void activate();
};