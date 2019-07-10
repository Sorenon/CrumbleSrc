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
#include <BulletCollision/NarrowPhaseCollision/btPolyhedralContactClipping.h>
#include <entt/entity/registry.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H  

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
#include "Pathfinder.h"
#include "Scene.h"
#include "Portal.h"
#include "StandardEntityComponents.h"
#include "StandardEntitySystems.h"

typedef struct _ThreadUnit
{
	int id;

	std::thread thread;
	bool readyOrExecuting = false;//Can start execution or is executing

public:
	_ThreadUnit(int idIn)
	{
		id = idIn;
	}
} ThreadUnit;

typedef struct _ThreadPool
{
	std::mutex mtx;
	std::condition_variable threadLock;
	bool done = false;//Threads can stop execution

	std::vector<ThreadUnit> threads;

public:
	_ThreadPool(int numOfThreads)
	{
		for (int i = 0; i < numOfThreads; i++)
		{
			threads.push_back(ThreadUnit(i));
		}
	}

	bool Finished()
	{
		for (ThreadUnit& unit : threads)
		{
			if (unit.readyOrExecuting) return false;
		}
		return true;
	}

	void Execute()
	{
		{//mtx.lock
			std::unique_lock<std::mutex> lck(mtx);
			for (ThreadUnit& unit : threads)
			{
				unit.readyOrExecuting = true;
			}

			done = false;
			threadLock.notify_all();
		}//mtx.unlock
		while (!Finished());//Wait for threads to finish
	}
}ThreadPool;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void updateEntities(ThreadPool& pool, ThreadUnit& threadData);
void interactWithWorlds(Input& input, float t);
void end();

GLFWwindow* window;
const int windowStartWidth = 854;
const int windowStartHeight = 480;
int wWidth = windowStartWidth;
int wHeight = windowStartHeight;

entt::registry::entity_type localplayer;
PhysicsWorld* p_physicsWorld;
GameRenderer* p_gameRenderer;
Pathfinder* p_pathfinder;
Scene scene;
entt::registry registry;//Question: use one register for all entities or use one register per entity

std::mutex entityMutex;
int entityIndex = 0;

int main(int argc, char* argv[])
{

	//glm::quat glmquat = FMath::createQuaternion(glm::vec3(0, glm::radians(54.0f), glm::radians(76.0f)));
	//btQuaternion btquat = btQuaternion(glm::radians(54.0f), 0, glm::radians(76.0f));

	//std::cout << glmquat.x << std::endl;
	//std::cout << glmquat.y << std::endl;
	//std::cout << glmquat.z << std::endl;
	//std::cout << glmquat.w << std::endl;

	//std::cout << btquat.getX() << std::endl;
	//std::cout << btquat.getY() << std::endl;
	//std::cout << btquat.getZ() << std::endl;
	//std::cout << btquat.getW() << std::endl;

	//btQuaternion glmquat2 = bullet_glm_conversion::convertQuaternion(glmquat);

	//std::cout << glmquat2.getX() << std::endl;
	//std::cout << glmquat2.getY() << std::endl;
	//std::cout << glmquat2.getZ() << std::endl;
	//std::cout << glmquat2.getW() << std::endl;


	//std::cin.ignore();

	//if (true) return 0;

	PhysicsWorld physicsWorld;
	p_physicsWorld = &physicsWorld;
	//physicsWorld.rbCube->getWorldTransform().setRotation(btQuaternion(0, glm::radians(90.0f), 0));

	//glm::quat glmquat = glm::quat(glm::vec3(Faces::Down.angle, 0.0f));
	//std::cout << "down" << std::endl;
	//std::cout << glmquat.x << std::endl;
	//std::cout << glmquat.y << std::endl;
	//std::cout << glmquat.z << std::endl;
	//std::cout << glmquat.w << std::endl << std::endl;

	//btQuaternion btquat = physicsWorld.rbCube->getWorldTransform().getRotation();
	//glm::quat btquat2 = bullet_glm_conversion::convertQuaternion(btquat);

	//std::cout << "rbCube" << std::endl;
	//std::cout << btquat2.x << std::endl;
	//std::cout << btquat2.y << std::endl;
	//std::cout << btquat2.z << std::endl;
	//std::cout << btquat2.w << std::endl << std::endl;

	//glm::quat glmquat2 = glmquat * btquat2;
	//std::cout << "glmquat * btquat2" << std::endl;
	//std::cout << glmquat2.x << std::endl;
	//std::cout << glmquat2.y << std::endl;
	//std::cout << glmquat2.z << std::endl;
	//std::cout << glmquat2.w << std::endl << std::endl;

	//glm::quat glmquat3 = btquat2 * glmquat;
	//std::cout << "btquat2 * glmquat" << std::endl;
	//std::cout << glmquat3.x << std::endl;
	//std::cout << glmquat3.y << std::endl;
	//std::cout << glmquat3.z << std::endl;
	//std::cout << glmquat3.w << std::endl << std::endl;

	//std::cin.ignore();

	//if (true) return 0;

	for (int x = -1; x <= 1; x++)
	{
		for (int z = -1; z <= 1; z++)
		{
			scene.mainWorld.createChunk(x, z, new Chunk(4));
		}
	}

	//subWorld.setBlock(5, 66, 5, 1);
	scene.mainWorld.setBlock(0, 62, 0, 1);

	scene.subWorlds.push_back(SubWorld());
	SubWorld& subWorld = scene.subWorlds[0];

	subWorld.offset = glm::vec3(5, 66, 5);
	subWorld.centerOfMassOffset = glm::vec3(0.5f, 0.5f, 0.5f);
	subWorld.setBlock(0, 0, 0, 1);
	subWorld.setBlock(0, 2, 0, 1);

	subWorld.rotation = glm::vec3(0, 0, glm::radians(-45.0f));
	subWorld.UpdateTranslationMatrix();

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(windowStartWidth, windowStartHeight, "Crumble", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window\n";
		std::cin.ignore();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	Input& input = Input::INSTANCE;
	input.init(window);

	localplayer = registry.create();
	{
		components::transform trans;
		trans.position = glm::vec3(0, 90, 0);
		trans.rotation.x = glm::radians(180.0f);
		trans.scene = &scene;

		components::kinematic_ridgedbody rb;
		rb.collider = AABB(vec3(-0.4, 0, -0.4), vec3(0.4, 1.9, 0.4));
		rb.eyeHeight = 1.8f;
		rb.gravity = 28.0f;

		registry.assign<components::transform>(localplayer, trans);
		registry.assign<components::kinematic_ridgedbody>(localplayer, rb);
		registry.assign<components::player_movement>(localplayer, components::player_movement());
	}

	auto entityFoo = registry.create();
	{
		components::transform trans;
		trans.position = glm::vec3(0, 90, 0);
		trans.scene = &scene;

		components::kinematic_ridgedbody rb;
		rb.collider = AABB(vec3(-0.49f, 0, -0.49f), vec3(0.49f, 1, 0.49f));
		rb.eyeHeight = 0.5f;

		registry.assign<components::transform>(entityFoo, trans);
		registry.assign<components::kinematic_ridgedbody>(entityFoo, rb);
		registry.assign<components::renderable>(entityFoo, components::renderable());
	}


	double lastFrame = glfwGetTime();
	float deltaTime;
	float accumulator = 0;
	int ticksThisFrame = 0;
	glfwSwapInterval(1);
	GameRenderer renderer;
	p_gameRenderer = &renderer;

	scene.portals.push_back(Portal(glm::vec3(0, 64, -6), glm::vec2(3, 3), Faces::Down, glm::vec3(0, 80, -6)));
	//scene.portals.push_back(Portal(glm::vec3(0, 80, -6), glm::vec2(3, 3), Faces::Up, glm::vec3(0, 64, -6)));

	Pathfinder pathfinder;
	p_pathfinder = &pathfinder;

	scene.mainWorld.setBlock(2, 64, 3, 1);

	//pathfinder.FindPath({ 0, 64, 0 }, { 3, 64, 3 }, 5);

	while (!glfwWindowShouldClose(window))
	{
		double frameStart = glfwGetTime();
		deltaTime = frameStart - lastFrame;
		lastFrame = frameStart;

		accumulator += deltaTime;

		ticksThisFrame = 0;
		while (accumulator > CrumbleGlobals::FIXED_TIMESTEP)
		{
			//physicsWorld.dynamicsWorld->stepSimulation(CrumbleGlobals::FIXED_TIMESTEP, 1, CrumbleGlobals::FIXED_TIMESTEP);

			input.processInput();

			standard_entity_systems::do_player_movement(registry);
			standard_entity_systems::move_kinematic_ridgedbodies(registry);

			accumulator -= CrumbleGlobals::FIXED_TIMESTEP;

			ticksThisFrame++;
		}

		physicsWorld.m_dynamicsWorld->stepSimulation(1 / 120.f, 1, 1 / 120.f);
		physicsWorld.m_dynamicsWorld->stepSimulation(1 / 120.f, 1, 1 / 120.f);
		//scene.portal.rotation += glm::vec3(0, glm::radians(1.0f), 0);

		subWorld.rotation += glm::vec3(0, 0, glm::radians(1.0f));
		subWorld.UpdateTranslationMatrix();

		{
			float sensitivity = 0.15f;

			auto& trans = registry.get<components::transform>(localplayer);
			glm::vec3& rotation = trans.rotation;

			rotation.x += glm::radians(input.deltaY * -sensitivity);
			rotation.y += glm::radians(input.deltaX * sensitivity);

			if (rotation.x > glm::radians(90.0f))
				rotation.x = glm::radians(90.0f);
			if (rotation.x < -glm::radians(90.0f))
				rotation.x = -glm::radians(90.0f);

			input.deltaX = 0;
			input.deltaY = 0;

			trans.prevRotation = rotation;//for the player entity trans.prevRotation == trans.rotation 
		}

		if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS)
		{//Reset physics
			btTransform trans;
			trans.setIdentity();
			//trans.setOrigin(FMath::convertVector(player.transform.position + glm::vec3(0, 0.5f, 0)));
			trans.setOrigin(btglm_conversion::convertVector(glm::vec3(4, 70.5f, 4)));

			constexpr float pitch = glm::radians(45.0f);
			constexpr float yaw = 0.0f;					//Yaw works as roll
			constexpr float roll = 0.0f; //Roll works as yaw
			trans.setRotation(btQuaternion(yaw, pitch, roll));

			glm::quat quat = FMath::createQuaternion(glm::vec3(pitch, yaw, roll));

			physicsWorld.m_rbCube->activate();
			physicsWorld.m_rbCube->setWorldTransform(trans);

			physicsWorld.m_rbCube->setLinearVelocity(btVector3(0, 0, 0));
			physicsWorld.m_rbCube->setAngularVelocity(btVector3(0, 0, 0));

			//entityFoo.transform.position = player.transform.position;
		}

		if (input.kbDoThing.executeOnce())
		{//Debug key
//entityFoo.transform.position = vec3(0.5f, 64, 0.5f);
//entityFoo.destination = glm::floor(player.transform.position);

			renderer.renderPortalDebugOutline = !renderer.renderPortalDebugOutline;
		}
		//entityFoo.destination = glm::floor(player.transform.position);

		float t = accumulator / CrumbleGlobals::FIXED_TIMESTEP;
		renderer.doRender(t);
		interactWithWorlds(input, t);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//threadContinue = false;
	end();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	wWidth = width;
	wHeight = height;
}

//void updateEntities(ThreadPool& pool, ThreadUnit& threadData) {
//	int ticks = 0;
//	Entity* entity = nullptr;
//
//	while (true) {
//		{//pool.mtx.lock
//			std::unique_lock<std::mutex> lck(pool.mtx);
//			pool.threadLock.wait(lck, [&]() {return threadData.readyOrExecuting; });
//		}//pool.mtx.unlock
//
//
//		while (entityIndex < scene.entities.size() && !pool.done) {
//			{//entityMutex.lock
//				std::lock_guard<std::mutex> lock(entityMutex);
//				int i = entityIndex++;
//
//				if (i >= scene.entities.size()) {
//					pool.done = true; //We have reached the end of the list
//					break;
//				}
//
//				//std::cout << threadData.id << " on " << i << std::endl;
//
//				entity = scene.entities[i];
//			}//entityMutex.unlock
//
//			entity->UpdateMultiThread();
//		}
//
//		pool.done = true;
//		threadData.readyOrExecuting = false;
//	}
//}

void interactWithWorlds(Input& input, float t)
{
	while (input.kbPlace.execute())
	{
		RayTraceResult& result = scene.RayTraceAllWorlds(t);
		if (result.hasHit)
		{

			glm::ivec3 placePos = result.hitPos + result.face;
			AABB playerCol = registry.get<components::kinematic_ridgedbody>(localplayer).collider + registry.get<components::transform>(localplayer).position;

			if ((AABB::blockAABB + placePos).overlaps(playerCol))
			{ //TODO: make this work with subWorlds
//if (player.onGround && !(AABB::blockAABB + placePos).overlaps(playerCol + Vectors::UP) && world.getOverlappingBlocks(playerCol.expandByVelocity(glm::vec3(0, 1, 0))).empty()) {
//	world.setBlock(placePos.x, placePos.y, placePos.z, 1);
//	player.transform.position.y += 0.25f;
//	player.velocity.y += 7.8f;
//}
			}
			else
			{
				result.world->setBlock(placePos.x, placePos.y, placePos.z, 1);
			}
		}
	}

	while (input.kbAttack.execute())
	{
		RayTraceResult& result = scene.RayTraceAllWorlds(t);
		if (result.hasHit)
		{
			result.world->setBlock(result.hitPos.x, result.hitPos.y, result.hitPos.z, 0);
		}
	}
}

void end()
{
	glfwTerminate();

	std::cout << "Done";
	std::cin.ignore();
}
