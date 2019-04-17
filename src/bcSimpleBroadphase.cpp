#include "bcSimpleBroadphase.h"
#include <iostream>

#include "btBulletDynamicsCommon.h"
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

	//if (block == nullptr) {
	//	block = makeBlock({5, 65, 5});
	//	for (int i = collisionWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
	//		btCollisionObject* obj = collisionWorld->getCollisionObjectArray()[i];

	//		if (!obj->isStaticObject() && obj->isActive()) {
	//			bcPairCache.worldCollisions.push_back(btBroadphasePair(*obj->getBroadphaseHandle(), *block->getBroadphaseHandle()));
	//		}
	//	}
	//}

	for (int i = collisionWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
		btCollisionObject* obj = collisionWorld->getCollisionObjectArray()[i];

		if (!obj->isStaticObject() && obj->isActive()) {
			doWorldCollisions(obj);
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
			else {
				ColBlockData* cBD = (ColBlockData*)block->getUserPointer();
				if (cBD->colliding.find(obj) != cBD->colliding.end()) {//Allready colliding with this block
					continue;
				}
			}
			ColBlockData* cBD = (ColBlockData*)block->getUserPointer();
			cBD->colliding.insert(obj);

			bcPairCache.worldCollisions.push_back(btBroadphasePair(*obj->getBroadphaseHandle(), *block->getBroadphaseHandle()));
		}
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

	//btCollisionObject* obj = new btCollisionObject;
	//btTransform trans;
	//trans.setOrigin(FMath::convertVector(glm::vec3(pos) + glm::vec3(0.5f, 0.5f, 0.5f)));
	//obj->setWorldTransform(trans);
	//obj->setCollisionShape(blockShape);
	//obj->setRestitution(0.1f);
	//obj->setUserPointer(new ColBlockData);

	short collisionFilterGroup = btBroadphaseProxy::CollisionFilterGroups::StaticFilter;
	short collisionFilterMask = (short)(btBroadphaseProxy::CollisionFilterGroups::AllFilter ^ btBroadphaseProxy::CollisionFilterGroups::StaticFilter);

	btVector3 min;
	btVector3 max;
	blockShape->getAabb(trans, max, min);

	btSimpleBroadphaseCopyProxy* proxy = new btSimpleBroadphaseCopyProxy(min, max, 1337, obj, collisionFilterGroup, collisionFilterMask);

	obj->setBroadphaseHandle(proxy);
	return obj;
}