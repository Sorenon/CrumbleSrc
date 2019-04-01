#pragma once
#include <string>
#include <glad/glad.h>

class ShaderConstructor {
public:
	std::string path;
private:
	GLuint id;

public:
	ShaderConstructor(const char* code, GLuint type);
	~ShaderConstructor();

	const GLuint getID();

	std::string getName(GLuint shaderId);
};