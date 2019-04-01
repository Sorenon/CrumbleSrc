#include "ShaderProgram.h"
#include "ShaderConstructor.h"
#include "FileUtil.h"
#include <iostream>

ShaderProgram::ShaderProgram(ShaderConstructor &vertex, ShaderConstructor &fragment) {
	id = glCreateProgram();
	glAttachShader(id, vertex.getID());
	glAttachShader(id, fragment.getID());
	glLinkProgram(id);

	int success;
	char infoLog[512];
	glGetProgramiv(id, GL_LINK_STATUS, &success);

	if (success == GL_FALSE) {
		glGetProgramInfoLog(id, 512, NULL, infoLog);
		std::cout << "ERROR::SHADERPROGRAM::LINK_FAILED\n" << infoLog << std::endl;
	}
}

ShaderProgram::~ShaderProgram() {
	glDeleteProgram(id);
}

void ShaderProgram::activate() {
	glUseProgram(id);
}
