#include "bcSimpleBroadphase.h"
#include <iostream>

#include <btBulletCollisionCommon.h>
#include <glm/glm.hpp>

#include "btSimpleBroadphaseCopy.h"
#include "globals.h"
#include "FMath.h"
#include "AABB.h"

bcSimpleBroadphase::bcSimpleBroadphase() : btSimpleBroadphaseCopy() {
	bcPairCache.defaultCache = m_pairCache;
}

void bcSimpleBroadphase::calculateOverlappingPairs(btDispatcher * dispatcher) {
	btSimpleBroadphaseCopy::calculateOverlappingPairs(dispatcher);

	//for (std::list<btBroadphasePair>::const_iterator it = bcPairCache.worldCollisions.begin(), end = bcPairCache.worldCollisions.end(); it != end; ++it) {
	//	const btBroadphasePair& pair = *it;
	//}

	//for (int i = collisionWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
	//	btCollisionObject* obj = collisionWorld->getCollisionObjectArray()[i];

	//	if (!obj->isStaticObject() && obj->isActive()) {
	//		doWorldCollisions(obj);
	//	}
	//}
}

btOverlappingPairCache* bcSimpleBroadphase::getOverlappingPairCache() {
	return &bcPairCache;
}

const btOverlappingPairCache* bcSimpleBroadphase::getOverlappingPairCache() const {
	return &bcPairCache;
}

void bcSimpleBroadphase::doWorldCollisions(btCollisionObject* obj) {
	btBroadphaseProxy* proxy = obj->getBroadphaseHandle();

	if ((proxy->m_collisionFilterMask & btBroadphaseProxy::CollisionFilterGroups::StaticFilter) != 0) {
		btVector3 min;
		btVector3 max;

		obj->getCollisionShape()->getAabb(obj->getWorldTransform(), min, max);

		AABB colAABB(FMath::convertVector(min), FMath::convertVector(max));

		for (AABB &aabb : world.getOverlappingBlocks(colAABB)) {
			glm::ivec3 pos = aabb.min;

			btCollisionObject* block = chunk[pos.x][pos.y][pos.z];
			if (block == nullptr) {
				block = makeBlock(pos);
				chunk[pos.x][pos.y][pos.z] = block;
			}

			m_pairCache->addOverlappingPair(obj->getBroadphaseHandle(), block->getBroadphaseHandle());
		}
	}
}

btCollisionObject* bcSimpleBroadphase::makeBlock(glm::ivec3 pos) {
	btCollisionObject* obj = new btCollisionObject;
	btTransform trans;
	trans.setOrigin(FMath::convertVector(pos));
	obj->setWorldTransform(trans);
	obj->setCollisionShape(blockShape);
	obj->setRestitution(0.1f);
	obj->setUserPointer(new ColBlockData);

	short collisionFilterGroup = btBroadphaseProxy::CollisionFilterGroups::StaticFilter;
	short collisionFilterMask = (short)(btBroadphaseProxy::CollisionFilterGroups::AllFilter ^ btBroadphaseProxy::CollisionFilterGroups::StaticFilter);

	btVector3 min;
	btVector3 max;
	blockShape->getAabb(trans, max, min);

	btSimpleBroadphaseCopyProxy* proxy = new btSimpleBroadphaseCopyProxy(min, max, 1337, obj, collisionFilterGroup, collisionFilterMask);
	proxy->m_uniqueId = ++m_numHandles;

	obj->setBroadphaseHandle(proxy);
	return obj;
}