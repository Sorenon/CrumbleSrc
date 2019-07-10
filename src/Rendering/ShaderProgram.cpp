#include "Rendering/ShaderProgram.h"

#include <iostream>

#include "Rendering/ShaderConstructor.h"
#include "FileUtil.h"

ShaderProgram::ShaderProgram(ShaderConstructor& vertex, ShaderConstructor& fragment)
{
	id = glCreateProgram();
	glAttachShader(id, vertex.getID());
	glAttachShader(id, fragment.getID());
	glLinkProgram(id);

	int success;
	char infoLog[512];
	glGetProgramiv(id, GL_LINK_STATUS, &success);

	if (success == GL_FALSE)
	{
		glGetProgramInfoLog(id, 512, NULL, infoLog);
		std::cout << "ERROR::SHADERPROGRAM::LINK_FAILED\n" << infoLog << std::endl;
	}
}

ShaderProgram::ShaderProgram(ShaderConstructor vertex, ShaderConstructor fragment)
{
	id = glCreateProgram();
	glAttachShader(id, vertex.getID());
	glAttachShader(id, fragment.getID());
	glLinkProgram(id);

	int success;
	char infoLog[512];
	glGetProgramiv(id, GL_LINK_STATUS, &success);

	if (success == GL_FALSE)
	{
		glGetProgramInfoLog(id, 512, NULL, infoLog);
		std::cout << "ERROR::SHADERPROGRAM::LINK_FAILED\n" << infoLog << std::endl;
	}

	viewID = glGetUniformLocation(id, "view");
	modelID = glGetUniformLocation(id, "model");
	projID = glGetUniformLocation(id, "projection");
	clipPlaneID = glGetUniformLocation(id, "clipPlane");
}

ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(id);
}

void ShaderProgram::activate()
{
	glUseProgram(id);
}
