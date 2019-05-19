#pragma once
#include <vector>

#include "globals.h"
#include "World.h"

class Entity;

class Scene
{
public:
	World mainWorld;
	std::vector<World> subWorlds;
	std::vector<Entity*> entities;//Possible TODO: Sort more expensive entites to be at the front to increase multithreaded update efficency (But may require more CPU time than it saves)

public:
	Scene();
	~Scene();

	RayTraceResult RayTraceAllWorlds(float t);
	RayTraceResult RayTraceFromPlayer(float t, World& world);
};

