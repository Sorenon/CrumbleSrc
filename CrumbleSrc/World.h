#pragma once
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "AABB.h"
#include "Chunk.h"

typedef long long chunkID;
typedef int chunkPos;

typedef struct {
	bool hit = false;
	glm::ivec3 hitPos;
	glm::ivec3 face;

	operator bool() const { return hit; };

}RayTraceResult;

class World {
public:
	static chunkID toLong(chunkPos x, chunkPos z);

public:
	std::unordered_map<chunkID, Chunk*> chunks;

public:
	World();
	~World();

	void createChunk(chunkPos x, chunkPos z);
	Chunk *getChunk(chunkPos x, chunkPos z);
	Chunk &getChunkSafe(chunkPos x, chunkPos z);

	int getBlock(int x, int y, int z);
	bool setBlock(int x, int y, int z, int block);
	std::vector<AABB> getOverlappingBlocks(const AABB &collider);

	RayTraceResult rayTrace(const glm::vec3 &start, const glm::vec3 &dir, float radius = 10.0f);

private:
	float intbound(float s, float ds);
};

