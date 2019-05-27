#pragma once
#include <vector>

#include "globals.h"
#include "World.h"
#include "Rendering/GameRenderer.h"
#include "Faces.h"

typedef struct {
	glm::vec3 position;
	Face facing = Faces::Front;
	AABB2D collider = AABB2D(0, 0, Faces::Front, {0,0,0});

	glm::vec3 exit;

	t_VAO planeVAO;
}Portal;

class Entity;

class Scene
{
public:
	World mainWorld;
	std::vector<SubWorld> subWorlds;
	std::vector<Entity*> entities;//Possible TODO: Sort more expensive entites to be at the front to increase multithreaded update efficency (But may require more CPU time than it saves)

	Portal portal;

public:
	Scene();
	~Scene();

	RayTraceResult RayTraceAllWorlds(float t);
	RayTraceResult RayTraceFromPlayer(float t, World& world);
	RayTraceResult RayTraceFromPlayer(float t, SubWorld& world);
};

