#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <bitset>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <stb_image.h>
#include "btBulletDynamicsCommon.h"

#include "Player.h"
#include "Chunk.h"
#include "ShaderProgram.h"
#include "ShaderConstructor.h"
#include "FileUtil.h"
#include "AABB.h"
#include "Transform.h"
#include "globals.h"
#include "World.h"
#include "Input.h"
#include "bcSimpleBroadphase.h"
#include "FMath.h"

GLuint generateTexture(const char* path);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

t_VAO createCubeVAO();
t_VAO createLineCubeVAO();
t_VAO createGUIPlain();
t_VAO createSubChunkVAO(SubChunk &subChunk, SubChunk &above, SubChunk &below, SubChunk &right, SubChunk &left, SubChunk &front, SubChunk &back);
float *createXFace(int x, int y, int z, bool flipped);
float *createYFace(int x, int y, int z, bool flipped);
float *createZFace(int x, int y, int z, bool flipped);
float *makePoint(float x, float y, float z, float r, float g, float b);
void end();

void initBullet();

struct bcDebugDrawer : btIDebugDraw {
public:
	int mode = 0;
	std::vector<float> vertices;
	int count = 0;

	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
		float *point = makePoint(from.getX(), from.getY(), from.getZ(), color.getX(), color.getY(), color.getZ());
		vertices.insert(vertices.end(), point, &point[5]);
		delete[] point;

		point = makePoint(to.getX(), to.getY(), to.getZ(), color.getX(), color.getY(), color.getZ());
		vertices.insert(vertices.end(), point, &point[5]);
		delete[] point;

		count += 2;
	}

	virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {}

	virtual void reportErrorWarning(const char* warningString) {}

	virtual void draw3dText(const btVector3& location, const char* textString) {}

	virtual void setDebugMode(int debugMode) {
		mode = debugMode;
	}

	virtual int getDebugMode() const {
		return mode;
	}
};

GLFWwindow* window;
const int windowStartWidth = 854;
const int windowStartHeight = 480;
int wWidth = windowStartWidth;
int wHeight = windowStartHeight;

Player *p_player;
World world;

btBroadphaseInterface*                  overlappingPairCache;
btDefaultCollisionConfiguration*        collisionConfiguration;
btCollisionDispatcher*                  dispatcher;
btSequentialImpulseConstraintSolver*    solver;
btDiscreteDynamicsWorld*                dynamicsWorld;
bcDebugDrawer* debugDraw;

btAlignedObjectArray<btCollisionShape*> collisionShapes;
btRigidBody* rbCube;

int main(int argc, char* argv[]) {
	initBullet();

	world.createChunk(0, 0, new Chunk(4));
	world.setBlock(5, 65, 5, 1);


	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(windowStartWidth, windowStartHeight, "Crumble", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window\n";
		std::cin.ignore();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	stbi_set_flip_vertically_on_load(true);

	ShaderProgram texturedProgram(ShaderConstructor(FileUtil::readFile("E:/CProjects/Crumble/CrumbleSrc/src/Shaders/textured.vert").c_str(), GL_VERTEX_SHADER), ShaderConstructor(FileUtil::readFile("E:/CProjects/Crumble/CrumbleSrc/src/Shaders/textured.frag").c_str(), GL_FRAGMENT_SHADER));
	ShaderProgram texColourProgram(ShaderConstructor(FileUtil::readFile("E:/CProjects/Crumble/CrumbleSrc/src/Shaders/texColour.vert").c_str(), GL_VERTEX_SHADER), ShaderConstructor(FileUtil::readFile("E:/CProjects/Crumble/CrumbleSrc/src/Shaders/texColour.frag").c_str(), GL_FRAGMENT_SHADER));

	GLint viewIDTex = glGetUniformLocation(texturedProgram.id, "view");
	GLint modelIDTex = glGetUniformLocation(texturedProgram.id, "model");
	GLint projIDTex = glGetUniformLocation(texturedProgram.id, "projection");

	GLint viewIDTexCol = glGetUniformLocation(texColourProgram.id, "view");
	GLint modelIDTexCol = glGetUniformLocation(texColourProgram.id, "model");
	GLint projIDTexCol = glGetUniformLocation(texColourProgram.id, "projection");
	GLint alphaIDTexCol = glGetUniformLocation(texColourProgram.id, "alpha");

	GLuint texture = generateTexture("C:/Users/Sorenon/Crumble/src/main/resources/grass.png");

	t_VAO cubeVAO = createCubeVAO();
	t_VAO blockLineVAO = createLineCubeVAO();
	t_VAO plane = createGUIPlain();

	Input& input = Input::INSTANCE;
	input.init(window);

	Player player;
	p_player = &player;
	player.transform.rotation.x = glm::radians(180.0f);

	double lastFrame = glfwGetTime();
	float deltaTime;
	float accumulator = 0;
	int ticksThisFrame = 0;
	glfwSwapInterval(1);

	while (!glfwWindowShouldClose(window)) {
		double frameStart = glfwGetTime();
		deltaTime = frameStart - lastFrame;
		lastFrame = frameStart;

		accumulator += deltaTime;

		ticksThisFrame = 0;
		while (accumulator > CrumbleGlobals::FIXED_TIMESTEP) {
			//dynamicsWorld->stepSimulation(CrumbleGlobals::FIXED_TIMESTEP, 10, 1 / 120.f);

			input.processInput();

			player.Update(window);

			accumulator -= CrumbleGlobals::FIXED_TIMESTEP;

			ticksThisFrame++;
		}

		dynamicsWorld->stepSimulation(1 / 60.f, 10, 1 / 120.f);

		{
			float sensitivity = 0.15f;

			glm::vec3 &rotation = p_player->transform.rotation;

			rotation.x += glm::radians(input.deltaY * -sensitivity);
			rotation.y += glm::radians(input.deltaX * sensitivity);

			if (rotation.x > glm::radians(90.0f))
				rotation.x = glm::radians(90.0f);
			if (rotation.x < -glm::radians(90.0f))
				rotation.x = -glm::radians(90.0f);

			input.deltaX = 0;
			input.deltaY = 0;
		}

		if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS) {
			btTransform trans;
			trans.setIdentity();
			trans.setOrigin(btVector3(5, 70, 5));

			rbCube->activate();
			rbCube->setWorldTransform(trans);

			rbCube->setLinearVelocity(btVector3(0, 0, 0));
			rbCube->setAngularVelocity(btVector3(0, 0, 0));
		}

		float t = accumulator / CrumbleGlobals::FIXED_TIMESTEP;

		//int chunksUpdated = 0;
		for (auto pair : world.chunks) {//Remake VAOs
			//break;
			Chunk &chunk = *pair.second;
			chunkID id = pair.first;
			int x = id >> 32;
			int z = (int)id;

			if (chunk.needsUpdate) {
				//std::cout << "Updating Chunk " << ++chunksUpdated << std::endl;

				for (int i = 0; i < 16; i++) {
					SubChunk *subChunk = chunk.subChunks[i];
					if (subChunk != nullptr && subChunk->needsUpdate) {

						SubChunk &above = i + 1 >= 16 ? SubChunk::EMPTY : chunk.getSubChunkSafe(i + 1);
						SubChunk &below = i - 1 < 0 ? SubChunk::EMPTY : chunk.getSubChunkSafe(i - 1);

						SubChunk &right = world.getChunkSafe(x + 1, z).getSubChunkSafe(i);
						SubChunk &left = world.getChunkSafe(x - 1, z).getSubChunkSafe(i);

						SubChunk &front = world.getChunkSafe(x, z - 1).getSubChunkSafe(i);
						SubChunk &back = world.getChunkSafe(x, z + 1).getSubChunkSafe(i);

						t_VAO oldVAO = chunk.subChunkVAOs[i];
						if (oldVAO.id != 0) {
							glDeleteVertexArrays(1, &oldVAO.id);
							glDeleteBuffers(1, &oldVAO.VBO);
						}

						chunk.subChunkVAOs[i] = createSubChunkVAO(*subChunk, above, below, right, left, front, back);

						subChunk->needsUpdate = false;
					}
				}
				chunk.needsUpdate = false;
			}
		}

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		texturedProgram.activate();

		float renderDistance = 12;
		glm::mat4 projection = glm::perspective(glm::radians(40.0f), (float)wWidth / (float)wHeight, 0.05f, renderDistance * 16 * (float)sqrt(2));
		glUniformMatrix4fv(projIDTex, 1, GL_FALSE, glm::value_ptr(projection));
		//glUniformMatrix4fv(projID, 1, GL_FALSE, glm::value_ptr(glm::scale(projection, glm::vec3(0.5f, 0.5f, 1)))); //Interesting effect

		glm::mat4 view = glm::mat4(1.0f);
		view = glm::rotate(view, player.transform.rotation.x, Vectors::RIGHT);
		view = glm::rotate(view, player.transform.rotation.y, Vectors::UP);
		view = glm::translate(view, -player.getEyePos(t));

		glUniformMatrix4fv(viewIDTex, 1, GL_FALSE, glm::value_ptr(view));

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		texturedProgram.activate();

		for (auto pair : world.chunks) { //Render world
			Chunk &chunk = *pair.second;
			chunkID id = pair.first;
			int x = id >> 32;
			int z = (int)id;

			for (int i = 0; i < 16; i++) {
				t_VAO &vao = chunk.subChunkVAOs[i];

				if (vao.id != 0) {
					glBindVertexArray(vao.id);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, texture);

					glm::mat4 model = glm::mat4(1.0f);
					model = glm::translate(model, glm::vec3(x * 16, i * 16, z * 16));
					glUniformMatrix4fv(modelIDTex, 1, GL_FALSE, glm::value_ptr(model));

					glDrawArrays(GL_TRIANGLES, 0, vao.vertices);
				}
			}
		}

		{//Render bullet object
			btTransform trans;
			rbCube->getMotionState()->getWorldTransform(trans);

			btVector3 transOrigin = trans.getOrigin();

			glBindVertexArray(cubeVAO.id);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);

			glm::mat4 model = glm::mat4(1.0f);
			trans.getOpenGLMatrix(glm::value_ptr(model));
			model = glm::translate(model, -glm::vec3(0.5f, 0.5f, 0.5f));//TODO: impove cube VBO 
			glUniformMatrix4fv(modelIDTex, 1, GL_FALSE, glm::value_ptr(model));

			glDrawArrays(GL_TRIANGLES, 0, cubeVAO.vertices);
		}

		for (auto &it : world.chunks) {
			Chunk *chunk = it.second;

			std::unordered_map<glm::ivec3, btCollisionObject*, HashFunc_ivec3, HashFunc_ivec3>::iterator it2 = chunk->storage.begin();
			auto end = chunk->storage.end();

			while (it2 != end) {
				auto pair = *it2;

				glBindVertexArray(cubeVAO.id);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, 0);

				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(pair.first) + glm::vec3(0.1f, 0.1f, 0.1f));//TODO: impove cube VBO 
				glUniformMatrix4fv(modelIDTex, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_TRIANGLES, 0, cubeVAO.vertices);

				++it2;
			}
		}

		{//Draw bullet debug
			dynamicsWorld->debugDrawWorld();
			std::vector<float> &vertices = debugDraw->vertices;
			if (!vertices.empty()) {
				GLuint VBO, VAO;
				glGenVertexArrays(1, &VAO);
				glGenBuffers(1, &VBO);

				glBindVertexArray(VAO);

				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);


				// position attribute
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
				// texture coord attribute
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
				glEnableVertexAttribArray(1);

				glm::mat4 model = glm::mat4(1.0f);
				glUniformMatrix4fv(modelIDTex, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_LINES, 0, debugDraw->count);

				glDeleteVertexArrays(1, &VAO);
				glDeleteBuffers(1, &VBO);

				vertices.clear();
				debugDraw->count = 0;
			}
		}

		RayTraceResult result = world.rayTrace(p_player->getEyePos(t), p_player->transform.getLook(t));

		texColourProgram.activate();

		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glUniformMatrix4fv(viewIDTexCol, 1, GL_FALSE, glm::value_ptr(view));
		glUniform1f(alphaIDTexCol, 0.4f);

		if (result.hit) {
			glDisable(GL_CULL_FACE);
			glLineWidth(2.5f);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glUniformMatrix4fv(projIDTexCol, 1, GL_FALSE, glm::value_ptr(glm::scale(projection, glm::vec3(1, 1, 0.999f))));//Emulating a slightly lower FOV

			glBindVertexArray(blockLineVAO.id);

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(result.hitPos));
			glUniformMatrix4fv(modelIDTexCol, 1, GL_FALSE, glm::value_ptr(model));

			glDrawArrays(GL_LINES, 0, blockLineVAO.vertices);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		glUniformMatrix4fv(projIDTexCol, 1, GL_FALSE, glm::value_ptr(projection));

		{//Render gui
			glEnable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);

			glUniformMatrix4fv(viewIDTexCol, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
			glBindVertexArray(plane.id);

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(-0.5, -0.5, -30));
			glUniformMatrix4fv(modelIDTexCol, 1, GL_FALSE, glm::value_ptr(model));

			glDrawArrays(GL_TRIANGLES, 0, plane.vertices);
		}

		glDisable(GL_BLEND);

		while (input.kbPlace.execute()) {
			result = world.rayTrace(p_player->getEyePos(t), p_player->transform.getLook(t));
			glm::ivec3 placePos = result.hitPos + result.face;
			AABB playerCol = p_player->getLocalBoundingBox();

			if ((AABB::blockAABB + placePos).overlaps(playerCol)) {
				//if (player.onGround && !(AABB::blockAABB + placePos).overlaps(playerCol + Vectors::UP) && world.getOverlappingBlocks(playerCol.expandByVelocity(glm::vec3(0, 1, 0))).empty()) {
				//	world.setBlock(placePos.x, placePos.y, placePos.z, 1);
				//	player.transform.position.y += 0.25f;
				//	player.velocity.y += 7.8f;
				//}
			} else {
				world.setBlock(placePos.x, placePos.y, placePos.z, 1);
			}
		}

		while (input.kbAttack.execute()) {
			result = world.rayTrace(p_player->getEyePos(t), p_player->transform.getLook(t));

			if (result.hit) {
				world.setBlock(result.hitPos.x, result.hitPos.y, result.hitPos.z, 0);
			}
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	end();
	return 0;
}

t_VAO createCubeVAO() {
	std::vector<float> vertices;

	//To do: add boilerplate
	float *side = createYFace(0, 1, 0, false);
	vertices.insert(vertices.end(), side, &side[6 * 5]);
	delete[] side;

	side = createYFace(0, 0, 0, true);
	vertices.insert(vertices.end(), side, &side[6 * 5]);
	delete[] side;

	side = createXFace(1, 0, 0, false);
	vertices.insert(vertices.end(), side, &side[6 * 5]);
	delete[] side;

	side = createXFace(0, 0, 0, true);
	vertices.insert(vertices.end(), side, &side[6 * 5]);
	delete[] side;

	side = createZFace(0, 0, 1, false);
	vertices.insert(vertices.end(), side, &side[6 * 5]);
	delete[] side;

	side = createZFace(0, 0, 0, true);
	vertices.insert(vertices.end(), side, &side[6 * 5]);
	delete[] side;

	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	if (!vertices.empty()) {
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	}

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	return { VAO, VBO, 6 * 6 };
}

t_VAO createLineCubeVAO() {
	float vertices[] = {
		0.0f, 0.0f, 0.0f,  0.0f, 0.0f,//Front 8
		1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
		0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
		0.0f, 0.0f, 0.0f,  0.0f, 0.0f,

		0.0f, 0.0f, 1.0f,  0.0f, 0.0f,//Back 8
		1.0f, 0.0f, 1.0f,  1.0f, 0.0f,
		1.0f, 0.0f, 1.0f,  1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,  0.0f, 1.0f,
		0.0f, 1.0f, 1.0f,  0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,  0.0f, 0.0f,

		0.0f, 1.0f, 1.0f,  1.0f, 0.0f,//Left 4
		0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,  0.0f, 0.0f,

		1.0f, 1.0f, 1.0f,  1.0f, 0.0f,//Right 4
		1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,  0.0f, 0.0f,
	};

	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	return { VAO, VBO, 8 * 2 + 4 * 2 };
}

t_VAO createGUIPlain() {
	float *side = createZFace(0, 0, 0, false);

	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float[5 * 6]), &side[0], GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	delete[] side;

	return { VAO, VBO, 6 * 6 };
}

t_VAO createSubChunkVAO(SubChunk & subChunk, SubChunk & above, SubChunk & below, SubChunk & right, SubChunk & left, SubChunk & front, SubChunk & back) {
	std::vector<float> vertices;

	int count = 0;
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			for (int z = 0; z < 16; z++) {
				if (subChunk.getBlock(x, y, z) != 0) {
					{//Yfaces
						if (y + 1 >= 16 ? above.getBlock(x, 0, z) == 0 : subChunk.getBlock(x, y + 1, z) == 0) {//Top side
							count++;

							float *side = createYFace(x, y + 1, z, false);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
						if (y - 1 < 0 ? below.getBlock(x, 15, z) == 0 : subChunk.getBlock(x, y - 1, z) == 0) {//Bottom side
							count++;

							float *side = createYFace(x, y, z, true);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
					}
					{//Xfaces
						if (x + 1 >= 16 ? right.getBlock(0, y, z) == 0 : subChunk.getBlock(x + 1, y, z) == 0) {//Right side
							count++;

							float *side = createXFace(x + 1, y, z, false);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}

						if (x - 1 < 0 ? left.getBlock(15, y, z) == 0 : subChunk.getBlock(x - 1, y, z) == 0) {//Bottom side
							count++;

							float *side = createXFace(x, y, z, true);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
					}
					{//ZFaces
						if (z + 1 >= 16 ? back.getBlock(x, y, 0) == 0 : subChunk.getBlock(x, y, z + 1) == 0) {//Back side
							count++;

							float *side = createZFace(x, y, z + 1, false);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
						if (z - 1 < 0 ? front.getBlock(x, y, 15) == 0 : subChunk.getBlock(x, y, z - 1) == 0) {//Front side
							count++;

							float *side = createZFace(x, y, z, true);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
					}
				}
			}
		}
	}

	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	if (!vertices.empty()) {
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	}

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	return { VAO, VBO, count * 6 };
}

float * createYFace(int x, int y, int z, bool flipped) {
	float *face;

	if (flipped) {
		face = new float[5 * 6]{
		0.0f + x,  0.0f + y, 0.0f + z,  0.0f, 1.0f,
		1.0f + x,  0.0f + y, 0.0f + z,  1.0f, 1.0f,
		1.0f + x,  0.0f + y, 1.0f + z,  1.0f, 0.0f,

		1.0f + x,  0.0f + y, 1.0f + z,  1.0f, 0.0f,
		0.0f + x,  0.0f + y, 1.0f + z,  0.0f, 0.0f,
		0.0f + x,  0.0f + y, 0.0f + z,  0.0f, 1.0f,
		};
	} else {
		face = new float[5 * 6]{
		0.0f + x,  0.0f + y, 0.0f + z,  0.0f, 1.0f,
		1.0f + x,  0.0f + y, 1.0f + z,  1.0f, 0.0f,
		1.0f + x,  0.0f + y, 0.0f + z,  1.0f, 1.0f,

		1.0f + x,  0.0f + y, 1.0f + z,  1.0f, 0.0f,
		0.0f + x,  0.0f + y, 0.0f + z,  0.0f, 1.0f,
		0.0f + x,  0.0f + y, 1.0f + z,  0.0f, 0.0f,
		};
	}

	return face;
}

float * createXFace(int x, int y, int z, bool flipped) {
	float *face;

	if (flipped) {
		face = new float[5 * 6]{
		0.0f + x, 1.0f + y, 1.0f + z,  1.0f, 0.0f,
		0.0f + x, 1.0f + y, 0.0f + z,  1.0f, 1.0f,
		0.0f + x, 0.0f + y, 0.0f + z,  0.0f, 1.0f,

		0.0f + x, 0.0f + y, 0.0f + z,  0.0f, 1.0f,
		0.0f + x, 0.0f + y, 1.0f + z,  0.0f, 0.0f,
		0.0f + x, 1.0f + y, 1.0f + z,  1.0f, 0.0f,
		};
	} else {
		face = new float[5 * 6]{
		0.0f + x, 1.0f + y, 1.0f + z,  1.0f, 0.0f,
		0.0f + x, 0.0f + y, 0.0f + z,  0.0f, 1.0f,
		0.0f + x, 1.0f + y, 0.0f + z,  1.0f, 1.0f,

		0.0f + x, 0.0f + y, 0.0f + z,  0.0f, 1.0f,
		0.0f + x, 1.0f + y, 1.0f + z,  1.0f, 0.0f,
		0.0f + x, 0.0f + y, 1.0f + z,  0.0f, 0.0f,
		};
	}

	return face;
}

float * createZFace(int x, int y, int z, bool flipped) {
	float *face;

	if (flipped) {
		face = new float[5 * 6]{
		0.0f + x, 0.0f + y,  0.0f + z,  0.0f, 0.0f,
		1.0f + x, 1.0f + y,  0.0f + z,  1.0f, 1.0f,
		1.0f + x, 0.0f + y,  0.0f + z,  1.0f, 0.0f,

		1.0f + x, 1.0f + y,  0.0f + z,  1.0f, 1.0f,
		0.0f + x, 0.0f + y,  0.0f + z,  0.0f, 0.0f,
		0.0f + x, 1.0f + y,  0.0f + z,  0.0f, 1.0f,
		};
	} else {
		face = new float[5 * 6]{
		0.0f + x, 0.0f + y,  0.0f + z,  0.0f, 0.0f,
		1.0f + x, 0.0f + y,  0.0f + z,  1.0f, 0.0f,
		1.0f + x, 1.0f + y,  0.0f + z,  1.0f, 1.0f,

		1.0f + x, 1.0f + y,  0.0f + z,  1.0f, 1.0f,
		0.0f + x, 1.0f + y,  0.0f + z,  0.0f, 1.0f,
		0.0f + x, 0.0f + y,  0.0f + z,  0.0f, 0.0f,
		};
	}

	return face;
}

float * makePoint(float x, float y, float z, float r, float g, float b) {
	//return new float[6]{x, y, z, r, g, b};
	return new float[5]{ x, y, z, 0, 0 };
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	wWidth = width;
	wHeight = height;
}

GLuint generateTexture(const char* path) {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int width, height, nrChannels;

	unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	return texture;
}

void initBullet() {
	overlappingPairCache = new bcSimpleBroadphase();
	//overlappingPairCache = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	solver = new btSequentialImpulseConstraintSolver;
	debugDraw = new bcDebugDrawer();

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
	dynamicsWorld->setDebugDrawer(debugDraw);
	debugDraw->setDebugMode(debugDraw->DBG_DrawAabb);

	{
		((bcSimpleBroadphase*)overlappingPairCache)->collisionWorld = dynamicsWorld;
	}

	{
		//btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
		btCollisionShape* groundShape = new btBoxShape(btVector3(20, 1, 20));

		btTransform startTransform;
		startTransform.setIdentity();

		btScalar mass(0.f);

		//startTransform.setOrigin(btVector3(0, 63.5f, 0));
		startTransform.setOrigin(btVector3(0, 0, 0));

		btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);

		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, groundShape, btVector3(0, 0, 0));
		btRigidBody* rb = new btRigidBody(rbInfo);

		dynamicsWorld->addRigidBody(rb);
	}

	{
		btCollisionShape* boxCollisionShape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));

		btTransform trans;
		trans.setIdentity();
		trans.setOrigin(btVector3(5, 70, 5));
		btDefaultMotionState* motionstate = new btDefaultMotionState(trans);

		int mass = 1;
		btVector3 localInertia;
		boxCollisionShape->calculateLocalInertia(mass, localInertia);

		btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionstate, boxCollisionShape, localInertia);
		rigidBodyCI.m_restitution = 0.1f;
		rigidBodyCI.m_friction = 0.91f;
		rbCube = new btRigidBody(rigidBodyCI);
		//rbCube->setActivationState(DISABLE_DEACTIVATION);

		dynamicsWorld->addRigidBody(rbCube);
	}
}

void end() {
	glfwTerminate();

	for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState()) {
			delete body->getMotionState();
		}
		dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	//delete collision shapes
	for (int j = 0; j < collisionShapes.size(); j++) {
		btCollisionShape* shape = collisionShapes[j];
		collisionShapes[j] = 0;
		delete shape;
	}

	delete dynamicsWorld;
	delete solver;
	delete overlappingPairCache;
	delete dispatcher;
	delete collisionConfiguration;

	std::cout << "Done";
	std::cin.ignore();
}
