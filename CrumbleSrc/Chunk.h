#pragma once

#include "glad/glad.h"

typedef int collumLoc;
typedef int cubeLoc;

typedef struct {
	GLuint id = 0;
	GLuint VBO = 0;
	int vertices = 0;
}t_VAO;

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
	static Chunk EMPTY;

public:
	SubChunk *subChunks[16] = {};
	bool needsUpdate = true;
	t_VAO subChunkVAOs[16];

public:
	Chunk();
	Chunk(int layers);
	~Chunk();

	SubChunk &getSubChunkSafe(int i);

	int getBlock(collumLoc x, collumLoc y, collumLoc z);
	bool setBlock(collumLoc x, collumLoc y, collumLoc z, int block);
};

