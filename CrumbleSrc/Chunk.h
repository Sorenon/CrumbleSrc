#pragma once

typedef int collumLoc;
typedef int relitiveLoc;

class SubChunk {
public:
	int blocks[16][16][16] = {0};
public:
	inline int getBlock(relitiveLoc x, relitiveLoc y, relitiveLoc z);
	inline void setBlock(relitiveLoc x, relitiveLoc y, relitiveLoc z, int block);
};

class Chunk {
public:
	SubChunk *subChunks[16] = {};

public:
	Chunk();
	~Chunk();

	int getBlock(collumLoc x, collumLoc y, collumLoc z);
	void setBlock(collumLoc x, collumLoc y, collumLoc z, int block);
};

