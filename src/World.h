#pragma once
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "AABB.h"
#include "Chunk.h"

class bcOverlappingPairCache;

typedef long long chunkID;
typedef int chunkPos;

typedef struct _Face {
	glm::ivec3 normalVector;
	float angle;
} Face;

namespace Faces {
	const Face Front = { { 0, 0, -1 } ,	glm::radians(0.0f) };
	const Face Behind = { { 0, 0, 1 } ,	glm::radians(180.0f) };
	const Face Right = { { 1, 0, 0 } ,	glm::radians(270.0f) };
	const Face Left = { { -1, 0, 0 } ,	glm::radians(90.0f) };

	//const Face horizontal[] = { Front, Behind, Right, Left };
	const Face horizontal[] = { Front, Left, Behind, Right };

	const Face* getFace(glm::ivec3 dir);
};

class World;

typedef struct _RayTraceResult {
	bool hasHit = false;
	glm::ivec3 hitPos;
	glm::ivec3 face;
	float distance = 0.0f;
	World* world = nullptr;

	operator bool() const { return hasHit; };

}RayTraceResult;

//Unlike in Minecraft, the World class only stores the block grid and not entites etc. Those are all stored in the Scene class
class World {
public:
	static chunkID toLong(chunkPos x, chunkPos z);

public:
	std::unordered_map<chunkID, Chunk*> chunks;
	bcOverlappingPairCache* bcPairCache;
	bool isSubWorld = false;

public:
	World();
	~World();

	void createChunk(chunkPos x, chunkPos z, Chunk* chunk = new Chunk());

	Chunk* getChunk(chunkPos x, chunkPos z);
	Chunk& getChunkSafe(chunkPos x, chunkPos z);

	Chunk& getChunkSafeBlockPos(int x, int z);

	int getBlock(int x, int y, int z);
	bool setBlock(int x, int y, int z, int block);
	std::vector<AABB> getOverlappingBlocks(const AABB& collider);

	RayTraceResult rayTrace(const glm::vec3& start, const glm::vec3& dir, float radius = 10.0f);

private:
	float intbound(float s, float ds);
};

class SubWorld : public World {
public:
	glm::vec3 offset;	//How far off 0,0,0 this world's center of mass is translated
	glm::vec3 rotation;
	glm::vec3 centerOfMassOffset;
	glm::mat4 translationMatrix = glm::mat4(1.0f); //Used to translate global positions into relitive positions of this world (*= global->relitive, /= relitive->global)

public:
	SubWorld() : World() {
		isSubWorld = true;
	};

	void UpdateTranslationMatrix();
};