#pragma once
#include <unordered_map>
#include <unordered_set>
#include <iostream>

#include <btBulletCollisionCommon.h>
#include <glm/glm.hpp>

#include "btSimpleBroadphaseCopy.h"
#include "bcOverlappingPairCache.h"

struct ColBlockData {
	std::unordered_set<btCollisionObject*> colliding;
};

class bcSimpleBroadphase : public btSimpleBroadphaseCopy {
public:
	btCollisionWorld* collisionWorld;

	bcOverlappingPairCache bcPairCache;

	btBoxShape* blockShape = new btBoxShape(btVector3(btScalar(0.5f), btScalar(0.5f), btScalar(0.5f)));
public:
	bcSimpleBroadphase();

	virtual void calculateOverlappingPairs(btDispatcher* dispatcher);
	virtual btOverlappingPairCache* getOverlappingPairCache();
	virtual const btOverlappingPairCache* getOverlappingPairCache() const;

	btCollisionObject* makeBlock(glm::ivec3 pos);
	void deleteBlock(btCollisionObject* blockCollider);

	void doWorldCollisions(btCollisionObject* obj);
	void deletePair(btBroadphasePair& pair, btDispatcher * dispatcher);
};

