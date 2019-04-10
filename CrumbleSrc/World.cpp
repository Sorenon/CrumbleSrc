#include "World.h"
#include <iostream>
#include <vector>
#include <limits>

#include "Chunk.h"

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

void World::createChunk(chunkPos x, chunkPos z) {
	chunks[toLong(x, z)] = new Chunk(4);
}

Chunk * World::getChunk(chunkPos x, chunkPos z) {
	auto chunk = chunks.find(toLong(x, z));
	return chunk == chunks.end() ? nullptr : (*chunk).second;
}

Chunk & World::getChunkSafe(chunkPos x, chunkPos z) {
	auto chunk = chunks.find(toLong(x, z));
	return chunk == chunks.end() ? Chunk::EMPTY : (*(*chunk).second);
}

int World::getBlock(int x, int y, int z) {
	if (y < 0 || y >= 256) {
		return 0;
	}

	auto chunk = chunks.find(toLong(x >> 4, z >> 4));
	return chunk == chunks.end() ? 0 : (*chunk).second->getBlock(x & 15, y, z & 15);
}

bool World::setBlock(int x, int y, int z, int block) {
	if (y < 0 || y >= 256) {
		return false;
	}


	auto chunk = chunks.find(toLong(x >> 4, z >> 4));
	return chunk == chunks.end() ? false : (*chunk).second->setBlock(x & 15, y, z & 15, block);
}

std::vector<AABB> World::getOverlappingBlocks(const AABB &collider) {
	std::vector<AABB> worldColliders;

	ivec3 max(glm::ceil(collider.max));
	ivec3 min(glm::floor(collider.min));

	AABB blockAABB(vec3(0, 0, 0), vec3(1, 1, 1));

	for (int x = min.x; x <= max.x; x++) {
		for (int y = min.y; y <= max.y; y++) {
			for (int z = min.z; z <= max.z; z++) {
				if (y < 256 && y >= 0) {
					if (getBlock(x, y, z) != 0) {
						worldColliders.push_back(blockAABB + vec3(x, y, z));
					}
				}
			}
		}
	}

	return worldColliders;
}

RayTraceResult World::rayTrace(const glm::vec3 &startPos, const glm::vec3 &endPos) {
	std::vector<glm::ivec3> blocks;

	glm::ivec3 scan = glm::floor(startPos);
	glm::ivec3 end = glm::floor(endPos);
	glm::vec3 ray = endPos - startPos;

	glm::ivec3 step(glm::sign(ray));
	glm::vec3 nextBoundary(scan.x + step.x, scan.y + step.y, scan.z + step.z);

	float fMax = std::numeric_limits<float>::max();

	glm::vec3 tMax(ray.x != 0 ? (nextBoundary.x - startPos.x) / ray.x : fMax, ray.y != 0 ? (nextBoundary.y - startPos.y) / ray.y : fMax, ray.z != 0 ? (nextBoundary.z - startPos.z) / ray.z : fMax);
	glm::vec3 tDelta(ray.x != 0 ? 1 / (ray.x * step.x) : fMax, ray.y != 0 ? 1 / (ray.y * step.y) : fMax, ray.z != 0 ? 1 / (ray.z * step.z) : fMax);

	glm::ivec3 diff;
	bool negRay = false;
	if (scan.x != end.x && ray.x < 0) { diff.x--; negRay = true; };
	if (scan.y != end.y && ray.y < 0) { diff.y--; negRay = true; };
	if (scan.z != end.z && ray.z < 0) { diff.z--; negRay = true; };

	//blocks.push_back(scan);
	if (negRay) {
		scan += diff;
	//	blocks.push_back(scan);
	}

	glm::ivec3 old = scan;
	RayTraceResult result;

	while (scan != end) {
		if (tMax.x < tMax.y) {
			if (tMax.x < tMax.z) {
				scan.x += step.x;
				tMax.x += tDelta.x;
			} else {
				scan.z += step.z;
				tMax.z += tDelta.z;
			}
		} else {
			if (tMax.y < tMax.z) {
				scan.y += step.y;
				tMax.y += tDelta.y;
			} else {
				scan.z += step.z;
				tMax.z += tDelta.z;
			}
		}

		if (result.hit == false && getBlock(scan.x, scan.y, scan.z) != 0) {
			result.hitPos = scan;
			result.hit = true;
		}
		blocks.push_back(scan);
	}
	result.all = blocks;
	return result;
}
