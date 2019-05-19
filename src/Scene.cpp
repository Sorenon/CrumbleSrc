#include "Scene.h"

#include "glm/glm.hpp"

#include "globals.h"
#include "FMath.h"
#include "Player.h"

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

	return world.rayTrace(p_player->getEyePos(t), p_player->transform.getLook(t));
}

RayTraceResult Scene::RayTraceFromPlayer(float t, SubWorld& world) {
	RayTraceResult result;

	glm::vec3 eyePos = glm::vec3(world.translationMatrix * glm::vec4(p_player->getEyePos(t), 1));

	glm::quat playerLook = FMath::createQuaternion(p_player->transform.getInterpRot(t));//Use quat to avoid gimbal lock
	playerLook = playerLook * glm::quat(world.rotation);

	glm::vec3 rayDir = Vectors::FORWARD * playerLook;

	return world.rayTrace(eyePos, rayDir);
}
