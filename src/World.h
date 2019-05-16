#pragma once
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "AABB.h"
#include "Chunk.h"

class bcOverlappingPairCache;

typedef long long chunkID;
typedef int chunkPos;

typedef struct {
	glm::ivec3 vec;
	float angle;
} Face;

namespace Faces {
	const Face Front = { { 0, 0, -1 } , 0 };
	const Face Behind = { { 0, 0, 1 } , 180 };
	const Face Right = { { 1, 0, 0 } , 270 };
	const Face Left = { { -1, 0, 0 } , 90 };

	//const Face horizontal[] = { Front, Behind, Right, Left };
	const Face horizontal[] = { Front, Left, Behind, Right };

	const Face* getFace(glm::ivec3 dir);
};

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
	bcOverlappingPairCache* bcPairCache;

	glm::vec3 offset;
	glm::vec3 rotation;
	glm::vec3 centerOfMassOffset;

public:
	World();
	~World();

	void createChunk(chunkPos x, chunkPos z, Chunk* chunk = new Chunk());

	Chunk* getChunk(chunkPos x, chunkPos z);
	Chunk& getChunkSafe(chunkPos x, chunkPos z);

	Chunk& getChunkSafeBlockPos(int x, int z);

	int getBlock(int x, int y, int z);
	bool setBlock(int x, int y, int z, int block);
	std::vector<AABB> getOverlappingBlocks(const AABB& collider);

	RayTraceResult rayTrace(const glm::vec3& start, const glm::vec3& dir, float radius = 10.0f);

private:
	float intbound(float s, float ds);
};