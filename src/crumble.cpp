#pragma once
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
//#include <stb_image.h>
#include "btBulletDynamicsCommon.h"

#include "Player.h"
#include "Chunk.h"
#include "Rendering/ShaderProgram.h"
#include "Rendering/ShaderConstructor.h"
#include "FileUtil.h"
#include "AABB.h"
#include "Transform.h"
#include "globals.h"
#include "World.h"
#include "Input.h"
#include "FMath.h"
#include "Rendering/GameRenderer.h"
#include "Physics/PhysicsWorld.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void end();

GLFWwindow* window;
const int windowStartWidth = 854;
const int windowStartHeight = 480;
int wWidth = windowStartWidth;
int wHeight = windowStartHeight;

Player *p_player;
PhysicsWorld *p_physicsWorld;
GameRenderer* p_gameRenderer;
World world;

int main(int argc, char* argv[]) {
	PhysicsWorld physicsWorld;
	p_physicsWorld = &physicsWorld;

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
	GameRenderer renderer;
	p_gameRenderer = &renderer;

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

		physicsWorld.dynamicsWorld->stepSimulation(1 / 60.f, 10, 1 / 120.f);

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

			physicsWorld.rbCube->activate();
			physicsWorld.rbCube->setWorldTransform(trans);

			physicsWorld.rbCube->setLinearVelocity(btVector3(0, 0, 0));
			physicsWorld.rbCube->setAngularVelocity(btVector3(0, 0, 0));
		}

		float t = accumulator / CrumbleGlobals::FIXED_TIMESTEP;

		renderer.doRender(t);

		RayTraceResult result;
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	wWidth = width;
	wHeight = height;
}

void end() {
	glfwTerminate();

	std::cout << "Done";
	std::cin.ignore();
}
