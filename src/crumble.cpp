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
#include "Pathfinder.h"
#include "Scene.h"
#include "Portal.h"
#include "StandardEntityComponents.h"

typedef struct _ThreadUnit {
	int id;

	std::thread thread;
	bool readyOrExecuting = false;//Can start execution or is executing

public:
	_ThreadUnit(int idIn) {
		id = idIn;
	}
} ThreadUnit;

typedef struct _ThreadPool {
	std::mutex mtx;
	std::condition_variable threadLock;
	bool done = false;//Threads can stop execution

	std::vector<ThreadUnit> threads;

public:
	_ThreadPool(int numOfThreads) {
		for (int i = 0; i < numOfThreads; i++) {
			threads.push_back(ThreadUnit(i));
		}
	}

	bool Finished() {
		for (ThreadUnit& unit : threads) {
			if (unit.readyOrExecuting) return false;
		}
		return true;
	}

	void Execute() {
		{//mtx.lock
			std::unique_lock<std::mutex> lck(mtx);
			for (ThreadUnit& unit : threads) {
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

entt::registry::entity_type player;
PhysicsWorld* p_physicsWorld;
GameRenderer* p_gameRenderer;
Pathfinder* p_pathfinder;
Scene scene;
entt::registry registry;

std::mutex entityMutex;
int entityIndex = 0;

int main(int argc, char* argv[]) {
	PhysicsWorld physicsWorld;
	p_physicsWorld = &physicsWorld;

	for (int x = -1; x <= 1; x++) {
		for (int z = -1; z <= 1; z++) {
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

	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	FT_Face face;
	if (FT_New_Face(ft, "C:/Windows/Fonts/arial.ttf", 0, &face))
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

	FT_Set_Pixel_Sizes(face, 0, 48);

	if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
		std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;

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

	//Player player;
	//p_player = &player;
	//player.transform.rotation.x = glm::radians(180.0f);

	//scene.entities.push_back(p_player);

	player = registry.create();
	{
		components::transform trans;
		trans.position = glm::vec3(0, 90, 0);
		trans.rotation.x = glm::radians(180.0f);

		components::kinematic_ridgedbody rb;
		rb.collider = AABB(vec3(-0.4, 0, -0.4), vec3(0.4, 1.9, 0.4));
		rb.eyeHeight = 1.8f;
		rb.gravity = 28.0f;

		registry.assign<components::transform>(player, trans);
		registry.assign<components::kinematic_ridgedbody>(player, rb);
		registry.assign<components::player_movement>(player, components::player_movement());
	}

	//EntityFoo entityFoo;
	//entityFoo.transform.position = vec3(0, 90, 0);
	//scene.entities.push_back(&entityFoo);

	//auto entityFoo = registry.create();
	//{
	//	components::transform trans;
	//	trans.position = glm::vec3(0, 90, 0);

	//	components::kinematic_ridgedbody rb;
	//	rb.collider = AABB(vec3(-0.49f, 0, -0.49f), vec3(0.49f, 1, 0.49f));
	//	rb.eyeHeight = 0.5f;

	//	registry.assign<components::transform>(entityFoo, trans);
	//	registry.assign<components::kinematic_ridgedbody>(entityFoo, rb);
	//	registry.assign<components::renderable>(entityFoo, components::renderable());
	//}


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

	while (!glfwWindowShouldClose(window)) {
		double frameStart = glfwGetTime();
		deltaTime = frameStart - lastFrame;
		lastFrame = frameStart;

		accumulator += deltaTime;

		ticksThisFrame = 0;
		while (accumulator > CrumbleGlobals::FIXED_TIMESTEP) {
			//physicsWorld.dynamicsWorld->stepSimulation(CrumbleGlobals::FIXED_TIMESTEP, 1, CrumbleGlobals::FIXED_TIMESTEP);

			input.processInput();

			auto view2 = registry.view<components::transform, components::kinematic_ridgedbody, components::player_movement>();
			for (auto entity : view2) {
				auto& trans = view2.get<components::transform>(entity);
				auto& rb = view2.get<components::kinematic_ridgedbody>(entity);
				auto& pm = view2.get<components::player_movement>(entity);

				vec3 forward = FMath::getForward(trans.rotation.y);
				vec3 right = glm::cross(forward, Vectors::UP);
				vec3 wishVel;

				Input& input = Input::INSTANCE;

				if (input.kbJump.executeOnce()) {
					if (rb.onGround) {
						rb.velocity.y += 9.0f;
					}
				}

				if (input.kbNoClip.executeOnce()) {
					rb.noClip = !rb.noClip;

					if (rb.noClip) {
						rb.gravity = 0.0f;
						rb.drag = 1.0f;
					}
					else {
						rb.gravity = 28.0f;
						rb.drag = 0.98f;
					}
				}

				const float moveSpeed = rb.noClip ? pm.noClipSpeed : pm.walkSpeed;

				wishVel += forward * (input.axForward.getModifier() * moveSpeed);
				wishVel += right * (input.axRight.getModifier() * moveSpeed);
				wishVel += glm::vec3(0.0f, input.axUp.getModifier() * moveSpeed, 0.0f);

				if (rb.noClip) {
					rb.velocity = wishVel;
				}
				else {
					if (rb.onGround) {
						components::player_movement::ApplyFriction(rb.velocity, 10.0f);
						components::player_movement::Walk(rb.velocity, wishVel, 15.0f);
					}
					else {
						components::player_movement::ApplyFriction(rb.velocity, 1.5f);
						components::player_movement::Walk(rb.velocity, wishVel, 3.0f);
					}
				}
			}

			auto view = registry.view<components::transform, components::kinematic_ridgedbody>();
			for (auto entity : view) {
				auto& trans = view.get<components::transform>(entity);
				auto& rb = view.get<components::kinematic_ridgedbody>(entity);

				rb.velocity.y -= rb.gravity * CrumbleGlobals::FIXED_TIMESTEP;
				rb.velocity *= rb.drag;

				glm::vec3 move = rb.velocity * CrumbleGlobals::FIXED_TIMESTEP;	//How far the entity expects to move 

				std::cout << glm::to_string(move) << std::endl;

				if (rb.noClip) {
					trans.prevPosition = trans.position;
					trans.position += move;
				}
				else {
					AABB entityCol = rb.collider + trans.position;
					AABB clippedCol = entityCol;

					std::vector<AABB> worldColliders = scene.mainWorld.getOverlappingBlocks(entityCol.expandByVelocity(move)); //Find all blocks (as AABBs) the entity may collide with

					{//Collide along y axis
						const float moveY = move.y;

						entityCol = rb.collider + trans.position;

						for (AABB aabb : worldColliders) {
							aabb.clipY(entityCol, move.y);
						}

						if (moveY != move.y) {
							rb.velocity.y = 0;

							if (moveY < 0.0f) {
								rb.onGround = true;
							}
							else {
								rb.onGround = false;
							}
						}
						else {
							rb.onGround = false;
						}

						trans.prevPosition.y = trans.position.y;
						trans.position.y += move.y;
					}

					{//Collide along x axis
						const float x = move.x;

						entityCol = rb.collider + trans.position;

						for (AABB aabb : worldColliders) {
							aabb.clipX(entityCol, move.x);
						}

						if (x != move.x) {
							rb.velocity.x = 0;
						}

						trans.prevPosition.x = trans.position.x;
						trans.position.x += move.x;
					}

					{//Collide along z axis
						const float moveZ = move.z;

						entityCol = rb.collider + trans.position;

						for (AABB aabb : worldColliders) {
							aabb.clipZ(entityCol, move.z);
						}

						if (moveZ != move.z) {
							rb.velocity.z = 0;
						}

						trans.prevPosition.z = trans.position.z;
						trans.position.z += move.z;
					}
				}

			}

			accumulator -= CrumbleGlobals::FIXED_TIMESTEP;

			ticksThisFrame++;
		}

		physicsWorld.dynamicsWorld->stepSimulation(1 / 120.f, 1, 1 / 120.f);
		physicsWorld.dynamicsWorld->stepSimulation(1 / 120.f, 1, 1 / 120.f);
		//scene.portal.rotation += glm::vec3(0, glm::radians(1.0f), 0);

		subWorld.rotation += glm::vec3(0, 0, glm::radians(1.0f));
		subWorld.UpdateTranslationMatrix();

		{
			float sensitivity = 0.15f;

			auto& trans = registry.get<components::transform>(player);
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

		if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS) {//Reset physics
			btTransform trans;
			trans.setIdentity();
			//trans.setOrigin(FMath::convertVector(player.transform.position + glm::vec3(0, 0.5f, 0)));
			trans.setOrigin(FMath::convertVector(glm::vec3(4, 70.5f, 4)));

			float pitch = glm::radians(45.0f);
			float yaw = 0.0f;
			float roll = 0.0f;
			trans.setRotation(btQuaternion(yaw, pitch, roll));

			glm::quat quat = FMath::createQuaternion(glm::vec3(pitch, yaw, roll));

			physicsWorld.rbCube->activate();
			physicsWorld.rbCube->setWorldTransform(trans);

			physicsWorld.rbCube->setLinearVelocity(btVector3(0, 0, 0));
			physicsWorld.rbCube->setAngularVelocity(btVector3(0, 0, 0));

			//entityFoo.transform.position = player.transform.position;
		}

		if (input.kbDoThing.executeOnce()) {//Debug key
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
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

void interactWithWorlds(Input& input, float t) {
	while (input.kbPlace.execute()) {
		RayTraceResult& result = scene.RayTraceAllWorlds(t);
		if (result.hasHit) {

			glm::ivec3 placePos = result.hitPos + result.face;
			AABB playerCol = registry.get<components::kinematic_ridgedbody>(player).collider + registry.get<components::transform>(player).position;

			if ((AABB::blockAABB + placePos).overlaps(playerCol)) { //TODO: make this work with subWorlds
				//if (player.onGround && !(AABB::blockAABB + placePos).overlaps(playerCol + Vectors::UP) && world.getOverlappingBlocks(playerCol.expandByVelocity(glm::vec3(0, 1, 0))).empty()) {
				//	world.setBlock(placePos.x, placePos.y, placePos.z, 1);
				//	player.transform.position.y += 0.25f;
				//	player.velocity.y += 7.8f;
				//}
			}
			else {
				result.world->setBlock(placePos.x, placePos.y, placePos.z, 1);
			}
		}
	}

	while (input.kbAttack.execute()) {
		RayTraceResult& result = scene.RayTraceAllWorlds(t);
		if (result.hasHit) {
			result.world->setBlock(result.hitPos.x, result.hitPos.y, result.hitPos.z, 0);
		}
	}
}

void end() {
	glfwTerminate();

	std::cout << "Done";
	std::cin.ignore();
}
