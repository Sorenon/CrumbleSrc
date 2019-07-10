#pragma once
#include <unordered_map>

#include <glm/glm.hpp>
#include "glad/glad.h"

class btCollisionObject;//Faster than including the bullet headers
class bcOverlappingPairCache;

typedef int collumLoc;
typedef int cubeLoc;

struct HashFunc_ivec3
{
	size_t operator()(const glm::ivec3& k)const
	{
		return (k.y + k.z * 31) * 31 + k.x;
	}

	bool operator()(const glm::ivec3& a, const glm::ivec3& b)const
	{
		return a == b;
	}
};

typedef struct
{
	GLuint id = 0;
	GLuint VBO = 0;
	int count = 0;
}t_VAO;

class SubChunk
{
public:
	static SubChunk EMPTY;

public:
	int blocks[16][16][16] = { 0 };
	bool needsUpdate = true;
public:
	int getBlock(collumLoc x, cubeLoc y, collumLoc z);
	inline bool setBlock(collumLoc x, cubeLoc y, collumLoc z, int block);
};

class Chunk
{
public:
	static Chunk EMPTY;

public:
	SubChunk* subChunks[16] = {};
	bool needsUpdate = true;
	t_VAO subChunkVAOs[16];
	std::unordered_map<glm::ivec3, btCollisionObject*, HashFunc_ivec3, HashFunc_ivec3> storage;
	bcOverlappingPairCache* bcPairCache = nullptr;

public:
	Chunk();
	Chunk(int layers);
	~Chunk();

	SubChunk& getSubChunkSafe(int i);

	int getBlock(collumLoc x, collumLoc y, collumLoc z);
	bool setBlock(collumLoc x, collumLoc y, collumLoc z, int block);

	void updateVAOTest(collumLoc x, collumLoc y, collumLoc z);
};

