#include "ShaderConstructor.h"
#include <glad/glad.h>
#include <iostream>

ShaderConstructor::ShaderConstructor(const char* code, GLuint type) {
	id = glCreateShader(type);

	glShaderSource(id, 1, &code, NULL);
	glCompileShader(id);

	int success;
	char infoLog[512];
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE) {
		glGetShaderInfoLog(id, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::" << getName(type) << "COMPILATION_FAILED\n" << infoLog << std::endl;
	}
}

ShaderConstructor::~ShaderConstructor() {
	glDeleteShader(id);
}

const GLuint ShaderConstructor::getID() {
	return id;
}

std::string ShaderConstructor::getName(GLuint shaderId) {
	switch (shaderId) {
	case GL_FRAGMENT_SHADER: return "fragment";
	case GL_VERTEX_SHADER: return "vertex";
	}

	return std::string();
}
