#pragma once
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <vector>
#include <unordered_map>
#include <bitset>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "ShaderProgram.h"
#include "ShaderConstructor.h"
#include "FileUtil.h"
#include "AABB.h"
#include "Transform.h"
#include "globals.h"
#include "Player.h"
#include "Chunk.h"
#include "World.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
GLuint generateTexture(const char* path);

typedef struct {
	GLuint id = 0;
	GLuint VBO = 0;
	int vertices = 0;
}t_VAO;

GLuint createVAO();
t_VAO createVAOFromChunk();
t_VAO createVAOFromSubChunk(int index);
t_VAO createSubChunkVAO(SubChunk &subChunk, SubChunk &above, SubChunk &below, SubChunk &right, SubChunk &left, SubChunk &front, SubChunk &back);
float *createXFace(int x, int y, int z, bool flipped);
float *createYFace(int x, int y, int z, bool flipped);
float *createZFace(int x, int y, int z, bool flipped);
void end();

GLFWwindow* window;
const int windowStartWidth = 854;
const int windowStartHeight = 480;
int wWidth = windowStartWidth;
int wHeight = windowStartHeight;

bool firstMouse = true;
float yaw = 0.0f;
float pitch = 0.0f;
float lastX = 0.0f;
float lastY = 0.0f;
Player *p_player;
Chunk rightChunk;
Chunk leftChunk;
Chunk middleChunk;


int main(int argc, char* argv[]) {

	World world;
	world.createChunk(0, 0);

	return 0;
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
	glfwSetCursorPosCallback(window, mouse_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	stbi_set_flip_vertically_on_load(true);

	ShaderConstructor vertex(FileUtil::readFile("E:/CProjects/CrumbleSrc/CrumbleSrc/Shaders/normal.vert").c_str(), GL_VERTEX_SHADER);
	ShaderConstructor fragment(FileUtil::readFile("E:/CProjects/CrumbleSrc/CrumbleSrc/Shaders/normal.frag").c_str(), GL_FRAGMENT_SHADER);

	ShaderProgram program(vertex, fragment);

	fragment.~ShaderConstructor();
	vertex.~ShaderConstructor();

	GLint viewID = glGetUniformLocation(program.id, "view");
	GLint modelID = glGetUniformLocation(program.id, "model");
	GLint projID = glGetUniformLocation(program.id, "projection");

	t_VAO chunkVAO = createVAOFromChunk();
	t_VAO subChunkVAOs[16];

	GLuint texture = generateTexture("C:/Users/Sorenon/Crumble/src/main/resources/grass.png");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	Player player;
	p_player = &player;

	double lastFrame = glfwGetTime();
	float deltaTime;
	float accumulator = 0;
	int ticksThisFrame = 0;

	while (!glfwWindowShouldClose(window)) {
		double frameStart = glfwGetTime();
		deltaTime = frameStart - lastFrame;
		lastFrame = frameStart;

		accumulator += deltaTime;

		ticksThisFrame = 0;
		while (accumulator > CrumbleGlobals::FIXED_TIMESTEP) {
			processInput(window);

			player.Update(window);

			accumulator -= CrumbleGlobals::FIXED_TIMESTEP;

			ticksThisFrame++;
		}

		float t = accumulator / CrumbleGlobals::FIXED_TIMESTEP;

		if (middleChunk.needsUpdate) {
			for (int i = 0; i < 16; i++) {
				SubChunk *subChunk = middleChunk.subChunks[i];
				if (subChunk != nullptr && subChunk->needsUpdate) {

					SubChunk &above = i + 1 >= 16 ? SubChunk::EMPTY : middleChunk.getSubChunk(i + 1);
					SubChunk &below = i - 1 < 0 ? SubChunk::EMPTY : middleChunk.getSubChunk(i - 1);
					SubChunk &right = rightChunk.getSubChunk(i);
					SubChunk &left = leftChunk.getSubChunk(i);

					subChunkVAOs[i] = createSubChunkVAO(*subChunk, above, below, right, left, SubChunk::EMPTY, SubChunk::EMPTY);

					subChunk->needsUpdate = false;
				}
			}
			middleChunk.needsUpdate = false;
		}

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		program.activate();
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
		glUniformMatrix4fv(projID, 1, GL_FALSE, glm::value_ptr(projection));

		{
			glm::mat4 view = glm::mat4(1.0f);
			view = glm::rotate(view, player.transform.rotation.x, Vectors::RIGHT);
			view = glm::rotate(view, player.transform.rotation.y, Vectors::UP);
			view = glm::translate(view, -player.transform.getInterpPos(t) - glm::vec3(0, 1.8, 0));

			glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));
		}

		for (int i = 0; i < 16; i++) {
			t_VAO &vao = subChunkVAOs[i];

			if (vao.id != 0) {
				glBindVertexArray(vao.id);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture);

				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0, i * 16, 0));
				glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_TRIANGLES, 0, vao.vertices);
			}
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	end();
	return 0;
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

void processInput(GLFWwindow * window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS) {
		p_player->velocity = Vectors::ZERO;
		p_player->transform.position = glm::vec3(-1, 1, -1);
	}
	if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS)
		p_player->noClip = !p_player->noClip;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		middleChunk.setBlock(0, 64, 0, 1);
}

void mouse_callback(GLFWwindow * window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.25f;
	xoffset *= sensitivity;
	yoffset *= -sensitivity;

	glm::vec3 &rotation = p_player->transform.rotation;

	rotation.x += glm::radians(yoffset);
	rotation.y += glm::radians(xoffset);

	if (rotation.x > glm::radians(89.0f))
		rotation.x = glm::radians(89.0f);
	if (rotation.x < -glm::radians(89.0f))
		rotation.x = -glm::radians(89.0f);
}

GLuint createVAO() {
	float vertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	return VAO;
}

t_VAO createVAOFromChunk() {
	std::vector<float> vertices;

	int count = 0;
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 256; y++) {
			for (int z = 0; z < 16; z++) {
				if (middleChunk.getBlock(x, y, z) != 0) {
					{//Yfaces
						if (y + 1 >= 256 || middleChunk.getBlock(x, y + 1, z) == 0) {//Top side
							count++;

							float *side = createYFace(x, y + 1, z, false);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
						if (y - 1 < 0 || middleChunk.getBlock(x, y - 1, z) == 0) {//Bottom side
							count++;

							float *side = createYFace(x, y, z, true);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
					}
					{//Xfaces
						if (x + 1 >= 16 || middleChunk.getBlock(x + 1, y, z) == 0) {//Right side
							count++;

							float *side = createXFace(x + 1, y, z, false);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}

						if (x - 1 < 0 || middleChunk.getBlock(x - 1, y, z) == 0) {//Left side
							count++;

							float *side = createXFace(x, y, z, true);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
					}
					{//ZFaces
						if (z + 1 >= 16 || middleChunk.getBlock(x, y, z + 1) == 0) {//Back side
							count++;

							float *side = createZFace(x, y, z + 1, false);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
						if (z - 1 < 0 || middleChunk.getBlock(x, y, z - 1) == 0) {//Front side
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

t_VAO createVAOFromSubChunk(int index) {
	//if (chunk.subChunks[index] == nullptr) {
	//	return t_VAO();
	//}
	std::vector<float> vertices;

	SubChunk &subChunk = *middleChunk.subChunks[index];
	SubChunk &above = middleChunk.subChunks[index + 1] == nullptr ? SubChunk::EMPTY : *middleChunk.subChunks[index + 1];
	SubChunk &below = middleChunk.subChunks[index - 1] == nullptr ? SubChunk::EMPTY : *middleChunk.subChunks[index - 1];

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
						if (x + 1 >= 16 || subChunk.getBlock(x + 1, y, z) == 0) {//Right side
							count++;

							float *side = createXFace(x + 1, y, z, false);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}

						if (x - 1 < 0 || subChunk.getBlock(x - 1, y, z) == 0) {//Left side
							count++;

							float *side = createXFace(x, y, z, true);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
					}
					{//ZFaces
						if (z + 1 >= 16 || subChunk.getBlock(x, y, z + 1) == 0) {//Back side
							count++;

							float *side = createZFace(x, y, z + 1, false);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
						if (z - 1 < 0 || subChunk.getBlock(x, y, z - 1) == 0) {//Front side
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
						if (z + 1 >= 16 || subChunk.getBlock(x, y, z + 1) == 0) {//Back side
							count++;

							float *side = createZFace(x, y, z + 1, false);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
						if (z - 1 < 0 || subChunk.getBlock(x, y, z - 1) == 0) {//Front side
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

void end() {
	glfwTerminate();

	std::cout << "Done";
	std::cin.ignore();
}