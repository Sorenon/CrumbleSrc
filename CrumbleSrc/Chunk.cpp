#include "Chunk.h"
#include <algorithm>
#include <iterator>

Chunk::Chunk() {
	for (int i = 0; i < 4; i++) {
		subChunks[i] = new SubChunk();

		int(*blocks)[16][16][16] = &subChunks[i]->blocks;

		std::fill(std::begin((*blocks)[0][0]), std::end((*blocks)[15][15]), 1);
	}
}

Chunk::~Chunk() {
	for (SubChunk *subChunk : subChunks) {
		if (subChunk != nullptr) {
			delete subChunk;
		}
	}
}

int Chunk::getBlock(collumLoc x, collumLoc y, collumLoc z) {
	SubChunk *subChunk = subChunks[y >> 4];

	if (subChunk == nullptr) {
		return 0;
	}
	else {
		return subChunk->getBlock(x, y % 16, z);
	}
}

void Chunk::setBlock(collumLoc x, collumLoc y, collumLoc z, int block) {
	SubChunk *subChunk = subChunks[y >> 4];

	if (subChunk == nullptr) {
		subChunk = new SubChunk;
		subChunks[y >> 4] = subChunk;
	}

	subChunk->setBlock(x, y % 16, z, block);
}

inline int SubChunk::getBlock(relitiveLoc x, relitiveLoc y, relitiveLoc z) {
	return blocks[x][y][z];
}

inline void SubChunk::setBlock(relitiveLoc x, relitiveLoc y, relitiveLoc z, int block) {
	blocks[x][y][z] = block;
}
