#pragma once

typedef int collumLoc;
typedef int cubeLoc;

class SubChunk {
public:
	int blocks[16][16][16] = {0};
	bool needsUpdate = true;
public:
	inline int getBlock(cubeLoc x, cubeLoc y, cubeLoc z);
	inline bool setBlock(cubeLoc x, cubeLoc y, cubeLoc z, int block);
};

class Chunk {
public:
	static SubChunk EMPTY;

public:
	SubChunk *subChunks[16] = {};
	bool needsUpdate = true;

public:
	Chunk();
	~Chunk();

	int getBlock(collumLoc x, collumLoc y, collumLoc z);
	bool setBlock(collumLoc x, collumLoc y, collumLoc z, int block);
};

