#pragma once
#include <glad/glad.h>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <bitset>
#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <btBulletDynamicsCommon.h>

#include "Player.h"
#include "Entity.h"
#include "EntityFoo.h"
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

struct ThreadUnit {
	int id;

	std::thread thread;
	bool ready = false;

public:
	ThreadUnit(int idIn) {
		id = idIn;
	}
};

struct ThreadPool {
	std::mutex mtx;
	std::condition_variable threadLock;
	//Threads can start execution
	bool done = false;//Threads can stop execution

	std::vector<ThreadUnit> threads;

public:
	ThreadPool(int numOfThreads) {
		for (int i = 0; i < numOfThreads; i++) {
			threads.push_back(ThreadUnit(i));
		}
	}

	bool Finished() {
		for (ThreadUnit& unit : threads) {
			if (unit.ready) return false;
		}
		return true;
	}

	void Execute() {
		{//mtx.lock
			std::unique_lock<std::mutex> lck(mtx);
			for (ThreadUnit& unit : threads) {
				unit.ready = true;
			}

			done = false;
			threadLock.notify_all();
		}//mtx.unlock
		while (!Finished());//Wait for threads to finish
	}
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void updateEntities(ThreadPool& pool, ThreadUnit& threadData);
void end();

GLFWwindow* window;
const int windowStartWidth = 854;
const int windowStartHeight = 480;
int wWidth = windowStartWidth;
int wHeight = windowStartHeight;

Player *p_player;
std::vector<Entity*> entities;//Possible TODO: Sort more expensive entites to be at the front to increase multithreaded update efficency (But may require more CPU time than it saves)
PhysicsWorld *p_physicsWorld;
GameRenderer* p_gameRenderer;
World world;

std::mutex entityMutex;
int entityIndex = 0;

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
	entities.push_back(p_player);

	EntityFoo entityFoo;
	entityFoo.transform.position = vec3(0, 90, 0);
	entities.push_back(&entityFoo);
	//EntityFoo entityFoo2;
	//entityFoo2.transform.position = vec3(2, 90, 0);
	//entities.push_back(&entityFoo2);
	//EntityFoo entityFoo3;
	//entityFoo3.transform.position = vec3(4, 90, 0);
	//entities.push_back(&entityFoo3);


	double lastFrame = glfwGetTime();
	float deltaTime;
	float accumulator = 0;
	int ticksThisFrame = 0;
	glfwSwapInterval(1);
	GameRenderer renderer;
	p_gameRenderer = &renderer;

	int tickss = 0;

	ThreadPool pool(2);
	for (ThreadUnit &unit : pool.threads) {
		unit.thread = std::thread(updateEntities, std::ref(pool), std::ref(unit));
	}

	while (!glfwWindowShouldClose(window)) {
		double frameStart = glfwGetTime();
		deltaTime = frameStart - lastFrame;
		lastFrame = frameStart;

		accumulator += deltaTime;

		ticksThisFrame = 0;
		while (accumulator > CrumbleGlobals::FIXED_TIMESTEP) {
			//dynamicsWorld->stepSimulation(CrumbleGlobals::FIXED_TIMESTEP, 10, 1 / 120.f);

			input.processInput();

			//std::cout << tickss++ << " " << std::endl;
			entityIndex = 0;
			pool.Execute();

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

			entityFoo.transform.position = vec3(0, 70, 0);
			//entityFoo2.transform.position = vec3(2, 70, 0);
			//entityFoo3.transform.position = vec3(4, 70, 0);
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

	//threadContinue = false;
	end();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	wWidth = width;
	wHeight = height;
}

void updateEntities(ThreadPool& pool, ThreadUnit& threadData) {
	int ticks = 0;
	Entity* entity = nullptr;

	while (true) {
		{//pool.mtx.lock
			std::unique_lock<std::mutex> lck(pool.mtx);
			pool.threadLock.wait(lck, [&]() {return threadData.ready; });
		}//pool.mtx.unlock


		while (entityIndex < entities.size() && !pool.done) {
			{//entityMutex.lock
				std::lock_guard<std::mutex> lock(entityMutex);
				int i = entityIndex++;

				if (i >= entities.size()) {
					pool.done = true; //We have reached the end of the list
					break;
				}

				//std::cout << threadData.id << " on " << i << std::endl;

				entity = entities[i];
			}//entityMutex.unlock

			entity->UpdateMultiThread();
		}

		pool.done = true;
		threadData.ready = false;
	}
};

void end() {
	glfwTerminate();

	std::cout << "Done";
	std::cin.ignore();
}
