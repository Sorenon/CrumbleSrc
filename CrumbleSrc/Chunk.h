#pragma once

typedef int collumLoc;
typedef int cubeLoc;

class SubChunk {
public:
	static SubChunk EMPTY;

public:
	int blocks[16][16][16] = {0};
	bool needsUpdate = true;
public:
	inline int getBlock(cubeLoc x, cubeLoc y, cubeLoc z);
	inline bool setBlock(cubeLoc x, cubeLoc y, cubeLoc z, int block);
};

class Chunk {
public:
	SubChunk *subChunks[16] = {};
	bool needsUpdate = true;

public:
	Chunk();
	~Chunk();

	SubChunk &getSubChunk(int i);

	int getBlock(collumLoc x, collumLoc y, collumLoc z);
	bool setBlock(collumLoc x, collumLoc y, collumLoc z, int block);
};

