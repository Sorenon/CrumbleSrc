#include "World.h"

#include "Chunk.h"


chunkID World::toLong(int x, int z) {
	return ((chunkID)x & 4294967295LL) << 32 | (chunkID) z & 4294967295LL;
}

World::World() {
}


World::~World() {
	for (auto pair : world) {
		delete pair.second;
	}
}

void World::createChunk(int x, int z) {
	world[toLong(x, z)] = new Chunk();
}

Chunk * World::getChunk(int x, int z) {
	auto chunk = world.find(toLong(x, z));
	return chunk == world.end() ? nullptr : (*chunk).second;
}
