#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "../Chunk.h"
#include "ShaderProgram.h"
#include "ShaderConstructor.h"
#include "../FileUtil.h"
#include "../World.h"

class GameRenderer {
public:
	ShaderProgram texturedProgram = ShaderProgram(ShaderConstructor(FileUtil::readFile("E:/CProjects/Crumble/CrumbleSrc/src/Shaders/textured.vert").c_str(), GL_VERTEX_SHADER), ShaderConstructor(FileUtil::readFile("E:/CProjects/Crumble/CrumbleSrc/src/Shaders/textured.frag").c_str(), GL_FRAGMENT_SHADER));
	ShaderProgram texColourProgram = ShaderProgram(ShaderConstructor(FileUtil::readFile("E:/CProjects/Crumble/CrumbleSrc/src/Shaders/texColour.vert").c_str(), GL_VERTEX_SHADER), ShaderConstructor(FileUtil::readFile("E:/CProjects/Crumble/CrumbleSrc/src/Shaders/texColour.frag").c_str(), GL_FRAGMENT_SHADER));

	GLint colourIDTexCol = glGetUniformLocation(texColourProgram.id, "colour");
	t_VAO cubeVAO = createCubeVAO();
	t_VAO blockLineVAO = createLineCubeVAO();
	t_VAO planeVAO = createPlane();
	GLuint texture;
	GLuint textureArrow;
	float renderDistance = 12;

	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;

public:
	GameRenderer();
	~GameRenderer();

	t_VAO createCubeVAO();
	t_VAO createLineCubeVAO();
	t_VAO createPlane();
	t_VAO createPlane(float x, float y, float z, float height, float width);
	t_VAO createSubChunkVAO(SubChunk &subChunk, SubChunk &above, SubChunk &below, SubChunk &right, SubChunk &left, SubChunk &front, SubChunk &back);
	float *createXFace(float x, float y, float z, bool flipped);
	float *createYFace(float x, float y, float z, bool flipped);
	float *createZFace(float x, float y, float z, bool flipped);

	GLuint generateTexture(const char* path);

	void doRender(float t);
	void renderScene(float t);

	void renderPortalStencil();

	void debugDrawPath();
	void debugDrawColliders();
	void debugDrawBulletDebug();

	void updateWorld(World* world);//Remake VAOs
	void renderWorld(World& world);//Render VAOs
	void renderWorld(SubWorld& world);//Render VAOs

	void renderEntities(float t);//Render entities and bullet entities

	void renderUI(float t);
};

