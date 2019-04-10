#pragma once
#include <unordered_map>
#include <vector>

#include "AABB.h"
#include "Chunk.h"

typedef long long chunkID;
typedef int chunkPos;

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
	std::vector<AABB> getOverlappingBlocks(AABB collider);
};

