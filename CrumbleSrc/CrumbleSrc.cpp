#pragma once
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>

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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
GLuint generateTexture(const char* path);

GLuint createVAO();
void end();

typedef struct {
	glm::vec3 rotation;
	glm::vec3 position = glm::vec3(0.0, 10.0, 0.0);
	glm::vec3 velocity;

	AABB collider = AABB(glm::vec3(-0.4, 0, -0.4), glm::vec3(0.4, 1.8, 0.4));
} Player;

GLFWwindow* window;
const int windowStartWidth = 854;
const int windowStartHeight = 480;
int wWidth = windowStartWidth;
int wHeight = windowStartHeight;

const float FIXED_TIMESTEP = 1.0f / 30.0f;

Player player;
AABB world = AABB(glm::vec3(-5, -1, -5), glm::vec3(0, 0, 0));

bool firstMouse = true;
float yaw = 0.0f;
float pitch = 0.0f;
float lastX = 0.0f;
float lastY = 0.0f;

int main(int argc, char* argv[]) {
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

	GLuint VAO = createVAO();

	GLuint texture = generateTexture("C:/Users/Sorenon/Crumble/src/main/resources/grass.png");

	glEnable(GL_DEPTH_TEST);

	double lastFrame = glfwGetTime();
	float deltaTime;
	float accumulator = 0;

	while (!glfwWindowShouldClose(window)) {
		double frameStart = glfwGetTime();
		deltaTime = frameStart - lastFrame;
		lastFrame = frameStart;

		accumulator += deltaTime;

		while (accumulator > FIXED_TIMESTEP) {
			processInput(window);

			//player.update(window);
			player.velocity.y -= 18 * FIXED_TIMESTEP;			//Apply gravity

			glm::vec3 move = player.velocity * FIXED_TIMESTEP;	//How far the player expects to move 
			AABB playerCol = player.collider + player.position;

			const float y = move.y;

			world.clipY(playerCol, move.y);

			if (y != move.y) {
				player.velocity.y = 0;
			}

			player.position += move;

			accumulator -= FIXED_TIMESTEP;
		}

		float t = accumulator / FIXED_TIMESTEP;

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		program.activate();
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
		glUniformMatrix4fv(projID, 1, GL_FALSE, glm::value_ptr(projection));

		{
			glm::mat4 view = glm::mat4(1.0f);
			view = glm::rotate(view, glm::radians(player.rotation.x), glm::vec3(1, 0, 0));
			view = glm::rotate(view, glm::radians(player.rotation.y), glm::vec3(0, 1, 0));
			view = glm::translate(view, -player.position - glm::vec3(0,1.8,0));

			glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));
		}

		glBindVertexArray(VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		for (int x = 0; x < 5; x++) {
			for (int z = 0; z < 5; z++) {
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(-x-0.5f, -0.5f, -z-0.5f));
				glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_TRIANGLES, 0, 36);
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

	glm::vec3 &rotation = player.rotation;

	rotation.x += yoffset;
	rotation.y += xoffset;

	if (rotation.x > 89.0f)
		rotation.x = 89.0f;
	if (rotation.x < -89.0f)
		rotation.x = -89.0f;
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

void end() {
	glfwTerminate();

	std::cout << "Done";
	std::cin.ignore();
}