#include "Scene.h"

#include "glm/glm.hpp"

#include "globals.h"
#include "FMath.h"
#include "StandardEntityComponents.h"

Scene::Scene()
{
}


Scene::~Scene()
{
}

RayTraceResult Scene::RayTraceAllWorlds(float t) {
	std::vector<RayTraceResult> results(1 + subWorlds.size());

	results.push_back(RayTraceFromPlayer(t, mainWorld));
	for (int i = 0; i < subWorlds.size(); i++) {
		SubWorld& world = subWorlds[i];
		results[i + 1] = RayTraceFromPlayer(t, world);
	}

	RayTraceResult chosen;
	for (RayTraceResult result : results) {
		if (result.hasHit) {
			if (chosen.hasHit) {
				if (result.distance < chosen.distance) {//TODO: improve the method used for calculating distance
					chosen = result;
				}
			}
			else {
				chosen = result;
			}
		}
	}

	return chosen;
}

RayTraceResult Scene::RayTraceFromPlayer(float t, World& world) {
	RayTraceResult result;

	auto& trans = registry.get<components::transform>(localplayer);
	auto& rb = registry.get<components::kinematic_ridgedbody>(localplayer);

	return world.rayTrace(trans.getInterpPos(t) + glm::vec3(0, rb.eyeHeight, 0), FMath::getNormal(trans.rotation));
}

RayTraceResult Scene::RayTraceFromPlayer(float t, SubWorld& world) {
	RayTraceResult result;

	auto& trans = registry.get<components::transform>(localplayer);
	auto& rb = registry.get<components::kinematic_ridgedbody>(localplayer);

	glm::vec3 eyePos = glm::vec3(world.translationMatrix * glm::vec4(trans.getInterpPos(t) + glm::vec3(0, rb.eyeHeight, 0), 1));

	glm::quat playerLook = FMath::createQuaternion(trans.rotation);//Use quat to avoid gimbal lock
	playerLook = playerLook * glm::quat(world.rotation);

	glm::vec3 rayDir = Vectors::FORWARD * playerLook;

	return world.rayTrace(eyePos, rayDir);
}
