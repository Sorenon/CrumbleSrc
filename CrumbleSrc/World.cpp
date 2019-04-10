#include "World.h"
#include <iostream>

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
	auto chunk = chunks.find(toLong(x >> 4, z >> 4));
	return chunk == chunks.end() ? 0 : (*chunk).second->getBlock(x & 15, y, z & 15);
}

std::vector<AABB> World::getOverlappingBlocks(AABB collider) {
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
