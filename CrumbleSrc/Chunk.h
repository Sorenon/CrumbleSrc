#pragma once

typedef int collumLoc;
typedef int relitiveLoc;

class SubChunk {
public:
	int blocks[16][16][16] = {0};
public:
	SubChunk();

	int getBlock(relitiveLoc x, relitiveLoc y, relitiveLoc z);
};

class Chunk {
public:
	SubChunk *subChunks[16] = {};

public:
	Chunk();
	~Chunk();

	int getBlock(collumLoc x, collumLoc y, collumLoc z);
};

