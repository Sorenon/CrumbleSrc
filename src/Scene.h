#pragma once
#include <vector>

#include "globals.h"
#include "World.h"
#include "Faces.h"
#include "Portal.h"

class Entity;

class Scene
{
public:
	World mainWorld;
	std::vector<SubWorld> subWorlds;
	std::vector<Portal> portals;
public:
	Scene();
	~Scene();

	RayTraceResult RayTraceAllWorlds(float t);
	RayTraceResult RayTraceFromPlayer(float t, World& world);
	RayTraceResult RayTraceFromPlayer(float t, SubWorld& world);
};

