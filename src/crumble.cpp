#pragma once
#include <glad/glad.h>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <bitset>
#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <boost/process.hpp>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/NarrowPhaseCollision/btPolyhedralContactClipping.h>
#include <entt/entity/registry.hpp>
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

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

//tmp functions to store broken code
void initNonNetworked();
void tickNonNetworked();

bool initOpenGL();

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
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
entt::registry registry;

ISteamNetworkingSockets* initGameNetworkSockets()
{
	SteamDatagramErrMsg errMsg;
	if (!GameNetworkingSockets_Init(nullptr, errMsg))
	{
		std::cout << "GameNetworkingSockets_Init failed.   " << errMsg;
		std::cin.ignore();
		exit(-1);
	}

	return SteamNetworkingSockets();
}

class NetworkingServer : private ISteamNetworkingSocketsCallbacks
{
private:
	ISteamNetworkingSockets* m_networkInterface = nullptr;
	HSteamListenSocket m_serverListenSocket = k_HSteamListenSocket_Invalid;

	struct Client_t
	{
		std::string m_name;
	};

	std::map<HSteamNetConnection, Client_t> m_clients;

public:
	void Run(ISteamNetworkingSockets* networkInterface)
	{
		SteamNetworkingIPAddr listenAddress;
		listenAddress.Clear();
		listenAddress.SetIPv6LocalHost(27020);

		m_networkInterface = networkInterface;
		m_serverListenSocket = networkInterface->CreateListenSocketIP(listenAddress);

		if (m_serverListenSocket == k_HSteamListenSocket_Invalid)
		{
			std::cout << "SERVER FAILED TO INIT";
			std::cin.ignore();
			exit(-1);
		}

		while (true)
		{
			m_networkInterface->RunCallbacks(this);

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

private:
	// Inherited via ISteamNetworkingSocketsCallbacks
	virtual void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* callbackInfo) override
	{
		switch (callbackInfo->m_info.m_eState)
		{
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		{
			throw "probo";
		}
		case k_ESteamNetworkingConnectionState_Connecting:
			assert(m_clients.find(callbackInfo->m_hConn) == m_clients.end());

			std::cout << "SERVER::Connection request from " << callbackInfo->m_info.m_szConnectionDescription << std::endl;

			if (m_networkInterface->AcceptConnection(callbackInfo->m_hConn) != k_EResultOK)
			{
				m_networkInterface->CloseConnection(callbackInfo->m_hConn, 0, nullptr, false);
				std::cout << "SERVER::connection closed before it connected" << std::endl;
				break;
			}

			m_clients[callbackInfo->m_hConn];

			std::cout << "SERVER::neat  " << callbackInfo->m_hConn << std::endl;
			break;
		default:
			break;
		}
	}
};

class NetworkingClient : private ISteamNetworkingSocketsCallbacks
{
private:
	ISteamNetworkingSockets* m_networkInterface = nullptr;
	HSteamNetConnection m_clientConnection = k_HSteamNetConnection_Invalid;

public:
	void Run(ISteamNetworkingSockets* networkInterface)
	{
		SteamNetworkingIPAddr serverAddress;
		serverAddress.Clear();
		serverAddress.SetIPv6LocalHost(27020);

		m_networkInterface = networkInterface;
		m_clientConnection = networkInterface->ConnectByIPAddress(serverAddress);

		if (m_clientConnection == k_HSteamNetConnection_Invalid)
		{
			std::cout << "CLIENT FAILED TO INIT";
			std::cin.ignore();
			exit(-1);
		}

		std::cout << "client init with id:" << m_clientConnection << std::endl;

		while (true)
		{
			m_networkInterface->RunCallbacks(this);

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

private:
	// Inherited via ISteamNetworkingSocketsCallbacks
	virtual void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* callbackInfo) override
	{
		assert(callbackInfo->m_hConn == m_clientConnection);

		switch (callbackInfo->m_info.m_eState)
		{
		case k_ESteamNetworkingConnectionState_Connected:
			std::cout << "Connected to server OK" << std::endl;
			break;
		default:
			break;
		}
	}
};

int main(int argc, const char* argv[])
{
	bool isServer = false;

	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "--server") == 0)
		{
			isServer = true;
		}
	}

	ISteamNetworkingSockets* networkInterface = initGameNetworkSockets();

	if (!isServer)//Client
	{
		boost::process::child serverProccess("crumble.exe --server");

		std::cout << "hello from client" << std::endl;

		NetworkingClient client;
		client.Run(networkInterface);

		GameNetworkingSockets_Kill();

		serverProccess.join();

		std::cout << "done";
		std::cin.ignore();

		return 0;
	}
	else//Server
	{
		std::cout << "hello from server" << std::endl;

		NetworkingServer server;
		server.Run(networkInterface);

		GameNetworkingSockets_Kill();

		return 0;
	}

	/*
	##################################################################################################################
	##################################################################################################################
	##################################################################################################################
	*/

	if (!initOpenGL())
	{
		return -1;
	}

	Input& input = Input::INSTANCE;
	input.init(window);

	for (int x = -1; x <= 1; x++)
	{
		for (int z = -1; z <= 1; z++)
		{
			scene.mainWorld.createChunk(x, z, new Chunk(4));
		}
	}

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

	while (!glfwWindowShouldClose(window))
	{
		double frameStart = glfwGetTime();
		deltaTime = frameStart - lastFrame;
		lastFrame = frameStart;

		accumulator += deltaTime;

		ticksThisFrame = 0;
		while (accumulator > CrumbleGlobals::FIXED_TIMESTEP)
		{
			input.processInput();

			standard_entity_systems::do_player_movement(registry);
			standard_entity_systems::move_kinematic_ridgedbodies(registry);

			accumulator -= CrumbleGlobals::FIXED_TIMESTEP;

			ticksThisFrame++;
		}

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

		if (input.kbDoThing.executeOnce())
		{
			renderer.renderPortalDebugOutline = !renderer.renderPortalDebugOutline;
		}

		float t = accumulator / CrumbleGlobals::FIXED_TIMESTEP;
		renderer.doRender(t);
		interactWithWorlds(input, t);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	end();
	return 0;
}

bool initOpenGL()
{
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
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	return true;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	wWidth = width;
	wHeight = height;
}

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

/*
////////////////////////////////
## IGNORE EVERYTHING BELOW ME ##
////////////////////////////////
*/

void initNonNetworked()
{
	PhysicsWorld physicsWorld;
	p_physicsWorld = &physicsWorld;

	scene.mainWorld.setBlock(0, 62, 0, 1);
	scene.subWorlds.push_back(SubWorld());
	SubWorld& subWorld = scene.subWorlds[0];
	subWorld.offset = glm::vec3(5, 66, 5);
	subWorld.centerOfMassOffset = glm::vec3(0.5f, 0.5f, 0.5f);
	subWorld.setBlock(0, 0, 0, 1);
	subWorld.setBlock(0, 2, 0, 1);
	subWorld.rotation = glm::vec3(0, 0, glm::radians(-45.0f));
	subWorld.UpdateTranslationMatrix();

	scene.portals.push_back(Portal(glm::vec3(0, 64, -6), glm::vec2(3, 3), Faces::Down, glm::vec3(0, 80, -6)));
	//scene.portals.push_back(Portal(glm::vec3(0, 80, -6), glm::vec2(3, 3), Faces::Up, glm::vec3(0, 64, -6)));
}

void tickNonNetworked()
{
	p_physicsWorld->m_dynamicsWorld->stepSimulation(1 / 120.f, 1, 1 / 120.f);
	p_physicsWorld->m_dynamicsWorld->stepSimulation(1 / 120.f, 1, 1 / 120.f);

	//subWorld.rotation += glm::vec3(0, 0, glm::radians(1.0f));
	//subWorld.UpdateTranslationMatrix();

	if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS)
	{//Reset physics
		btTransform trans;
		trans.setIdentity();
		//trans.setOrigin(FMath::convertVector(player.transform.position + glm::vec3(0, 0.5f, 0)));
		trans.setOrigin(btglmConvert::Vector(glm::vec3(0, 70, -6)));

		//constexpr float pitch = glm::radians(45.0f);
		//constexpr float yaw = glm::radians(32.0f);					
		//constexpr float roll = glm::radians(5.0f); 
		//trans.setRotation(btQuaternion(yaw, pitch, roll));

		p_physicsWorld->m_rbCube->activate();
		p_physicsWorld->m_rbCube->setWorldTransform(trans);

		p_physicsWorld->m_rbCube->setLinearVelocity(btVector3(0, 0, 0));
		p_physicsWorld->m_rbCube->setAngularVelocity(btVector3(0, 0, 0));

		//entityFoo.transform.position = player.transform.position;
	}
}

