#include "Chunk.h"
#include <algorithm>
#include <iterator>

SubChunk SubChunk::EMPTY;
Chunk Chunk::EMPTY;

Chunk::Chunk() {

}

Chunk::Chunk(int layers) {
	for (int i = 0; i < layers; i++) {
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

SubChunk& Chunk::getSubChunkSafe(int i) {
	return subChunks[i] == nullptr ? SubChunk::EMPTY : *subChunks[i];
}

int Chunk::getBlock(collumLoc x, collumLoc y, collumLoc z) {
	SubChunk *subChunk = subChunks[y >> 4];

	if (subChunk == nullptr) {
		return 0;
	} else {
		return subChunk->getBlock(x, y % 16, z);
	}
}

bool Chunk::setBlock(collumLoc x, collumLoc y, collumLoc z, int block) {
	int subChunkY = y >> 4;

	SubChunk *subChunk = subChunks[subChunkY];

	if (subChunk == nullptr) {
		subChunk = new SubChunk;
		subChunks[subChunkY] = subChunk;
	}

	cubeLoc relY = y % 16;
	if (subChunk->setBlock(x, relY, z, block)) {
		needsUpdate = true;

		if (relY == 15 && y != 255) {
			if (subChunks[subChunkY + 1] != nullptr) {
				SubChunk &above = *subChunks[(subChunkY) + 1];

				if (above.getBlock(x, 0, z) != 0) {
					above.needsUpdate = true;
				}
			}
		} else if (relY == 0 && y != 0) {
			if (subChunks[subChunkY - 1] != nullptr) {
				SubChunk &below = *subChunks[(subChunkY) - 1];

				if (below.getBlock(x, 15, z) != 0) {
					below.needsUpdate = true;
				}
			}
		}

		return true;
	}

	return false;
}

void Chunk::updateVAOTest(collumLoc x, collumLoc y, collumLoc z) {
	SubChunk *subChunk = subChunks[y >> 4];

	if (subChunk != nullptr && subChunk->getBlock(x, y % 16, z) != 0) {
		needsUpdate = true;
		subChunk->needsUpdate = true;
	}
}

int SubChunk::getBlock(collumLoc x, cubeLoc y, collumLoc z) {
	return blocks[x][y][z];
}

inline bool SubChunk::setBlock(collumLoc x, cubeLoc y, collumLoc z, int block) {
	if (blocks[x][y][z] == block) {
		return false;
	} else {
		blocks[x][y][z] = block;
		needsUpdate = true;
		return true;
	}
}
