#include "World.h"
#include <iostream>
#include <vector>
#include <limits>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Chunk.h"
#include "FMath.h"

chunkID World::toLong(chunkPos x, chunkPos z) {
	return ((chunkID)x & 4294967295LL) << 32 | (chunkID)z & 4294967295LL;
}

World::World() {
}


World::~World() {
	for (auto pair : chunks) {
		delete pair.second;
	}
}

void World::createChunk(chunkPos x, chunkPos z, Chunk* chunk) {
	chunk->bcPairCache = bcPairCache;
	chunks[toLong(x, z)] = chunk;
}

Chunk* World::getChunk(chunkPos x, chunkPos z) {
	auto chunk = chunks.find(toLong(x, z));
	return chunk == chunks.end() ? nullptr : chunk->second;
}

Chunk& World::getChunkSafe(chunkPos x, chunkPos z) {
	auto chunk = chunks.find(toLong(x, z));
	return chunk == chunks.end() ? Chunk::EMPTY : (*chunk->second);
}

Chunk& World::getChunkSafeBlockPos(int x, int z) {
	auto chunk = chunks.find(toLong(x >> 4, z >> 4));
	return chunk == chunks.end() ? Chunk::EMPTY : (*chunk->second);
}

int World::getBlock(int x, int y, int z) {
	if (y < 0 || y >= 256) {
		return 0;
	}

	auto chunk = chunks.find(toLong(x >> 4, z >> 4));
	return chunk == chunks.end() ? 0 : chunk->second->getBlock(x & 15, y, z & 15);
}

bool World::setBlock(int x, int y, int z, int block) {
	if (y < 0 || y >= 256) {
		return false;
	}

	chunkPos chunkX = x >> 4;
	chunkPos chunkZ = z >> 4;

	auto chunk = chunks.find(toLong(chunkX, chunkZ));

	if (chunk == chunks.end()) {
		createChunk(chunkX, chunkZ);
		chunk = chunks.find(toLong(chunkX, chunkZ));
	}

	collumLoc collumX = x & 15;
	collumLoc collumZ = z & 15;

	(*chunk).second->setBlock(collumX, y, collumZ, block);

	if (collumX == 15) {
		auto chunkItr = chunks.find(toLong(chunkX + 1, chunkZ));
		if (chunkItr != chunks.end()) {
			(*chunkItr).second->updateVAOTest(0, y, collumZ);
		}
	}
	else if (collumX == 0) {
		auto chunkItr = chunks.find(toLong(chunkX - 1, chunkZ));
		if (chunkItr != chunks.end()) {
			(*chunkItr).second->updateVAOTest(15, y, collumZ);
		}
	}

	if (collumZ == 15) {
		auto chunkItr = chunks.find(toLong(chunkX, chunkZ + 1));
		if (chunkItr != chunks.end()) {
			(*chunkItr).second->updateVAOTest(collumX, y, 0);
		}
	}
	else if (collumZ == 0) {
		auto chunkItr = chunks.find(toLong(chunkX, chunkZ - 1));
		if (chunkItr != chunks.end()) {
			(*chunkItr).second->updateVAOTest(collumX, y, 15);
		}
	}
}

std::vector<AABB> World::getOverlappingBlocks(const AABB & collider) {
	std::vector<AABB> worldColliders;

	ivec3 max(glm::ceil(collider.max));
	ivec3 min(glm::floor(collider.min));

	for (int x = min.x; x <= max.x; x++) {
		for (int y = min.y; y <= max.y; y++) {
			for (int z = min.z; z <= max.z; z++) {
				if (y < 256 && y >= 0) {
					if (getBlock(x, y, z) != 0) {
						AABB aabb = AABB::blockAABB + ivec3(x, y, z);//WHY!!!! TODO: FIX THE NEED FOR THIS, fix what?

						if (aabb.overlaps(collider)) {
							worldColliders.push_back(aabb);
						}
					}
				}
			}
		}
	}

	return worldColliders;
}

//TODO: clean this up
//From https://gist.github.com/dogfuntom/cc881c8fc86ad43d55d8
RayTraceResult World::rayTrace(const glm::vec3 & ray_start, const glm::vec3 & dir, float radius) {
	if (dir.x == 0 && dir.y == 0 && dir.z == 0)
		throw "Ray-cast in zero direction!";

	glm::vec3 scan = glm::floor(ray_start);
	glm::ivec3 step = glm::sign(dir);

	glm::vec3 tMax(intbound(ray_start.x, dir.x), intbound(ray_start.y, dir.y), intbound(ray_start.z, dir.z));
	glm::vec3 tDelta(step.x / dir.x, step.y / dir.y, step.z / dir.z);

	radius /= glm::length(dir);

	RayTraceResult result;
	result.world = this;
	glm::ivec3 face;

	while (true) {
		if (getBlock(scan.x, scan.y, scan.z) == 1) {
			result.hasHit = true;
			result.hitPos = scan;
			result.face = face;
			result.distance = glm::distance(glm::floor(ray_start), glm::vec3(result.hitPos));//Flimsy method, should be improved
			break;
		}

		if (tMax.x < tMax.y) {
			if (tMax.x < tMax.z) {
				if (tMax.x > radius)
					break;
				// Update which cube we are now in.
				scan.x += step.x;
				// Adjust tMax.x to the next X-oriented boundary crossing.
				tMax.x += tDelta.x;
				// Record the normal vector of the cube face we entered.
				face.x = -step.x;
				face.y = 0;
				face.z = 0;
			}
			else {
				if (tMax.z > radius)
					break;
				scan.z += step.z;
				tMax.z += tDelta.z;
				face.x = 0;
				face.y = 0;
				face.z = -step.z;
			}
		}
		else {
			if (tMax.y < tMax.z) {
				if (tMax.y > radius)
					break;
				scan.y += step.y;
				tMax.y += tDelta.y;
				face.x = 0;
				face.y = -step.y;
				face.z = 0;
			}
			else {
				// Identical to the second case, repeated for simplicity in
				// the conditionals.
				if (tMax.z > radius)
					break;
				scan.z += step.z;
				tMax.z += tDelta.z;
				face.x = 0;
				face.y = 0;
				face.z = -step.z;
			}
		}
	}

	return result;
}

void SubWorld::UpdateTranslationMatrix() {
	translationMatrix = glm::mat4(1.0f); 
	
	translationMatrix = glm::translate(translationMatrix, centerOfMassOffset);
	translationMatrix = translationMatrix * glm::toMat4(FMath::createQuaternion(-rotation));
	translationMatrix = glm::translate(translationMatrix, -offset);
}

float World::intbound(float s, float ds) {
	bool sIsInt = floorf(s) == s;
	if (ds < 0 && sIsInt)
		return 0;

	if (ds == 0) {
		return FLT_MAX;
	}

	return (ds > 0 ? s == 0.0f ? 1.0f : ceilf(s) - s : s - floorf(s)) / fabs(ds);
}

const Face* Faces::getFace(glm::ivec3 dir) {
	for (const Face& face : horizontal) {
		if (dir == face.vec) {
			return &face;
		}
	}

	return nullptr;
};

