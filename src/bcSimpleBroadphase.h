#pragma once
#include <unordered_map>
#include <unordered_set>
#include <iostream>

#include <btBulletCollisionCommon.h>
#include <glm/glm.hpp>

#include "btSimpleBroadphaseCopy.h"
#include "bcOverlappingPairCache.h"

struct HashFunc_ivec3 {
	size_t operator()(const glm::ivec3& k)const {
		return (k.y + k.z * 31) * 31 + k.x;
	}

	bool operator()(const glm::ivec3& a, const glm::ivec3& b)const {
		return a == b;
	}
};

struct ColBlockData {
	std::unordered_set<btCollisionObject*> colliding;
};

class bcSimpleBroadphase : public btSimpleBroadphaseCopy {
public:
	btCollisionWorld* collisionWorld;

	bcOverlappingPairCache bcPairCache;
	//btSphereShape blockShape = btSphereShape(btScalar(0.5f));
	//btBoxShape blockShape = btBoxShape(btVector3(1.f, 1.f, 1.f));
	btBoxShape* blockShape = new btBoxShape(btVector3(btScalar(0.5f), btScalar(0.5f), btScalar(0.5f)));


	//std::unordered_map<glm::ivec3, btCollisionObject*, HashFunc_ivec3, HashFunc_ivec3> storage;
	btCollisionObject *chunk[16][256][16] = { 0 };

public:
	bcSimpleBroadphase();

	virtual void calculateOverlappingPairs(btDispatcher* dispatcher);
	virtual btOverlappingPairCache* getOverlappingPairCache();
	virtual const btOverlappingPairCache* getOverlappingPairCache() const;

	btCollisionObject* makeBlock(glm::ivec3 pos);
	void doWorldCollisions(btCollisionObject* obj);
};

