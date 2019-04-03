#include "Chunk.h"
#include <algorithm>
#include <iterator>

Chunk::Chunk() {
	for (int i = 0; i < 4; i++) {
		subChunks[i] = new SubChunk();
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

SubChunk::SubChunk() {
	std::fill(std::begin(blocks[0][0]), std::end(blocks[15][15]), 1);
}

int SubChunk::getBlock(relitiveLoc x, relitiveLoc y, relitiveLoc z) {
	return blocks[x][y][z];
}
