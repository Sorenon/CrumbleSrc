#include "bcSimpleBroadphase.h"
#include <iostream>

#include "btBulletDynamicsCommon.h"
#include <glm/glm.hpp>

#include "btSimpleBroadphaseCopy.h"
#include "globals.h"
#include "FMath.h"
#include "AABB.h"
#include "Chunk.h"

bcSimpleBroadphase::bcSimpleBroadphase() : btSimpleBroadphaseCopy() {
	bcPairCache.defaultCache = m_pairCache;
	world.bcPairCache = &bcPairCache;
}

void bcSimpleBroadphase::calculateOverlappingPairs(btDispatcher * dispatcher) {
	btSimpleBroadphaseCopy::calculateOverlappingPairs(dispatcher);

	std::cout << "beg" << bcPairCache.worldCollisions.size();
	{//Remove all uncolliding pairs
		std::list<btBroadphasePair>::iterator it = bcPairCache.worldCollisions.begin();
		auto end = bcPairCache.worldCollisions.end();
		while (it != end) {
			btBroadphasePair& pair = *it;

			//std::unordered_set<btCollisionObject*> &toRemove = bcPairCache.toRemove;
			//if (toRemove.find((btCollisionObject*)pair.m_pProxy0->m_clientObject) != toRemove.end()) { //proxy0 will allways be the blockCollider so no need to check which is which
			//	deletePair(pair, dispatcher);
			//	it = bcPairCache.worldCollisions.erase(it);
			//	continue;
			//}

			AABB aabb1((btCollisionObject*)pair.m_pProxy0->m_clientObject);
			AABB aabb2((btCollisionObject*)pair.m_pProxy1->m_clientObject);
			if (!aabb1.overlaps(aabb2)) { //TODO: check if this comparison is too different to the doWorldCollisions one
				deletePair(pair, dispatcher);
				it = bcPairCache.worldCollisions.erase(it);
				continue;

			}

			++it;
		}
	}
	bcPairCache.toRemove.clear();
	std::cout << " end" << bcPairCache.worldCollisions.size() << std::endl;

	////Delete all removed blockColliders
	//for (btCollisionObject* obj : bcPairCache.toRemove) {
	//	deleteBlock(obj);
	//}

	//Collide rigedbodies with the world
	for (int i = collisionWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
		btCollisionObject* obj = collisionWorld->getCollisionObjectArray()[i];

		if (!obj->isStaticObject() && obj->isActive()) {
			doWorldCollisions(obj);
		}
	}

	//Delete all unused blocks
	for (auto &it : world.chunks) {
		Chunk *chunk = it.second;

		std::unordered_map<glm::ivec3, btCollisionObject*, HashFunc_ivec3, HashFunc_ivec3>::iterator it2 = chunk->storage.begin();
		auto end = chunk->storage.end();

		while (it2 != end) {
			auto pair = *it2;

			ColBlockData* cBD = (ColBlockData*)pair.second->getUserPointer();
			if (cBD->colliding.empty()) {
				deleteBlock(pair.second);
				it2 = chunk->storage.erase(it2);
			}
			else {
				++it2;
			}
		}
	}
}

btOverlappingPairCache* bcSimpleBroadphase::getOverlappingPairCache() {
	return &bcPairCache;
}

const btOverlappingPairCache* bcSimpleBroadphase::getOverlappingPairCache() const {
	return &bcPairCache;
}

void bcSimpleBroadphase::doWorldCollisions(btCollisionObject* obj) {
	if ((obj->getBroadphaseHandle()->m_collisionFilterMask & btBroadphaseProxy::CollisionFilterGroups::StaticFilter) != 0) {

		for (AABB &aabb : world.getOverlappingBlocks(AABB(obj))) {
			glm::ivec3 pos = aabb.min;

			Chunk &chunk = world.getChunkSafeBlockPos(pos.x, pos.z);
			auto it = chunk.storage.find(glm::ivec3(pos.x % 16, pos.y, pos.z % 16));
			btCollisionObject* blockCollider;
			if (it == chunk.storage.end()) {//BlockCollider doesn't exist yet 
				blockCollider = makeBlock(pos);
				chunk.storage[pos] = blockCollider;
			} else {
				blockCollider = (*it).second;
				ColBlockData* cBD = (ColBlockData*)blockCollider->getUserPointer();
				if (cBD->colliding.find(obj) != cBD->colliding.end()) {//Already colliding with this block
					continue;
				}
			}
			ColBlockData* cBD = (ColBlockData*)blockCollider->getUserPointer();
			cBD->colliding.insert(obj);

			bcPairCache.worldCollisions.push_back(btBroadphasePair(*blockCollider->getBroadphaseHandle(), *obj->getBroadphaseHandle()));
		}
	}
}

void bcSimpleBroadphase::deletePair(btBroadphasePair & pair, btDispatcher * dispatcher) {
	ColBlockData* cBD = (ColBlockData*)((btCollisionObject*)pair.m_pProxy0->m_clientObject)->getUserPointer(); //proxy0 will allways be the blockCollider so no need to check which is which
	cBD->colliding.erase(cBD->colliding.find((btCollisionObject*)pair.m_pProxy1->m_clientObject));

	if (pair.m_algorithm != nullptr) {
		pair.m_algorithm->~btCollisionAlgorithm();
		dispatcher->freeCollisionAlgorithm(pair.m_algorithm);
		pair.m_algorithm = nullptr;
	}
}

btCollisionObject* bcSimpleBroadphase::makeBlock(glm::ivec3 pos) {
	btTransform trans;
	trans.setIdentity();
	trans.setOrigin(FMath::convertVector(glm::vec3(pos) + glm::vec3(0.5f, 0.5f, 0.5f)));
	btDefaultMotionState* motionstate = new btDefaultMotionState(trans);

	btRigidBody::btRigidBodyConstructionInfo rbInfo(0, motionstate, blockShape, btVector3(0, 0, 0));
	rbInfo.m_restitution = 0.1f;
	rbInfo.m_friction = 0.91f;
	btRigidBody* obj = new btRigidBody(rbInfo);
	obj->setUserPointer(new ColBlockData);

	short collisionFilterGroup = btBroadphaseProxy::CollisionFilterGroups::StaticFilter;
	short collisionFilterMask = (short)(btBroadphaseProxy::CollisionFilterGroups::AllFilter ^ btBroadphaseProxy::CollisionFilterGroups::StaticFilter);

	btVector3 min;
	btVector3 max;
	blockShape->getAabb(trans, max, min);

	btSimpleBroadphaseCopyProxy* proxy = new btSimpleBroadphaseCopyProxy(min, max, 1337, obj, collisionFilterGroup, collisionFilterMask);
	proxy->m_uniqueId = INT_MIN;//Do this to prevent btBroadphasePair reordering itself

	obj->setBroadphaseHandle(proxy);
	return obj;
}

void bcSimpleBroadphase::deleteBlock(btCollisionObject * blockCollider) {
	delete blockCollider->getUserPointer();
	delete blockCollider->getBroadphaseHandle();
	delete ((btRigidBody*)blockCollider)->getMotionState();
	delete blockCollider;
}