#pragma once
#include <unordered_map>

#include "Chunk.h"

typedef long long chunkID;

class World {
public:
	static chunkID toLong(int x, int z);

public:
	std::unordered_map<chunkID, Chunk*> world;

public:
	World();
	~World();

	void createChunk(int x, int z);
	Chunk *getChunk(int x, int z);
};

