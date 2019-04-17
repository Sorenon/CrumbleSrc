#pragma once
#include <list>
#include <vector>
#include <unordered_set>

#include <btBulletCollisionCommon.h>

class bcOverlappingPairCache : public btOverlappingPairCache {
public:
	btOverlappingPairCache* defaultCache;
	std::list<btBroadphasePair> worldCollisions;//I chose list over vector because: Lots of iterating and sudden removal+addition
	std::unordered_set<btCollisionObject*> toRemove;//BlockColliders to delete before the next collision step

public:
	//Most of these functions just pass the method to the default btOverlappingPairCache 

	virtual btBroadphasePair* getOverlappingPairArrayPtr() {
		return defaultCache->getOverlappingPairArrayPtr();
	}

	virtual const btBroadphasePair* getOverlappingPairArrayPtr() const {
		return defaultCache->getOverlappingPairArrayPtr();
	}

	virtual btBroadphasePairArray& getOverlappingPairArray() {
		return defaultCache->getOverlappingPairArray();
	}
	
	virtual void cleanOverlappingPair(btBroadphasePair& pair, btDispatcher* dispatcher) {
		defaultCache->cleanOverlappingPair(pair, dispatcher);
	}

	virtual void setOverlapFilterCallback(btOverlapFilterCallback* callback) {
		defaultCache->setOverlapFilterCallback(callback);
	}

	virtual void setInternalGhostPairCallback(btOverlappingPairCallback* ghostPairCallback) {
		defaultCache->setInternalGhostPairCallback(ghostPairCallback);
	}

	virtual void sortOverlappingPairs(btDispatcher* dispatcher) {
		throw "later";
		defaultCache->sortOverlappingPairs(dispatcher);
	}

	virtual int getNumOverlappingPairs() const {
		return defaultCache->getNumOverlappingPairs();
	}

	virtual btBroadphasePair* findPair(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) {
		throw "later";
		defaultCache->findPair(proxy0, proxy1);
	}

	virtual bool hasDeferredRemoval() {
		return false;
	}

	virtual btBroadphasePair* addOverlappingPair(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) {
		return defaultCache->addOverlappingPair(proxy0, proxy1);
	}

	virtual void* removeOverlappingPair(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1, btDispatcher* dispatcher) {
		return defaultCache->removeOverlappingPair(proxy0, proxy1, dispatcher);
	}

	//Overrided functions
	virtual void cleanProxyFromPairs(btBroadphaseProxy* proxy, btDispatcher* dispatcher);

	virtual void removeOverlappingPairsContainingProxy(btBroadphaseProxy* proxy0, btDispatcher* dispatcher);

	virtual void processAllOverlappingPairs(btOverlapCallback* overlapCallback, btDispatcher* dispatcher);

	//Custom functions
	void removeBlockCollider(btCollisionObject* blockCollider);
};

