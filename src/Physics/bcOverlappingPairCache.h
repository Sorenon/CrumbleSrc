#pragma once
#include <list>
#include <vector>
#include <unordered_set>

#include <btBulletCollisionCommon.h>

class bcOverlappingPairCache : public btOverlappingPairCache
{
public:
	btOverlappingPairCache* m_defaultCache;
	std::list<btBroadphasePair> m_blockWorldCollisions;//I chose list over vector because: Lots of iterating and sudden removal+addition
	std::unordered_set<btCollisionObject*> m_unusedBlockColliders;//BlockColliders to delete before the next collision step

public:
	//Most of these functions just pass the method to the default btOverlappingPairCache 

	virtual btBroadphasePair* getOverlappingPairArrayPtr()
	{
		return m_defaultCache->getOverlappingPairArrayPtr();
	}

	virtual const btBroadphasePair* getOverlappingPairArrayPtr() const
	{
		return m_defaultCache->getOverlappingPairArrayPtr();
	}

	virtual btBroadphasePairArray& getOverlappingPairArray()
	{
		return m_defaultCache->getOverlappingPairArray();
	}

	virtual void cleanOverlappingPair(btBroadphasePair& pair, btDispatcher* dispatcher)
	{
		m_defaultCache->cleanOverlappingPair(pair, dispatcher);
	}

	virtual void setOverlapFilterCallback(btOverlapFilterCallback* callback)
	{
		m_defaultCache->setOverlapFilterCallback(callback);
	}

	virtual void setInternalGhostPairCallback(btOverlappingPairCallback* ghostPairCallback)
	{
		m_defaultCache->setInternalGhostPairCallback(ghostPairCallback);
	}

	virtual void sortOverlappingPairs(btDispatcher* dispatcher)
	{
		throw "later";
		m_defaultCache->sortOverlappingPairs(dispatcher);
	}

	virtual int getNumOverlappingPairs() const
	{
		return m_defaultCache->getNumOverlappingPairs();
	}

	virtual btBroadphasePair* findPair(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1)
	{
		throw "later";
		m_defaultCache->findPair(proxy0, proxy1);
	}

	virtual bool hasDeferredRemoval()
	{
		return false;
	}

	virtual btBroadphasePair* addOverlappingPair(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1)
	{
		return m_defaultCache->addOverlappingPair(proxy0, proxy1);
	}

	virtual void* removeOverlappingPair(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1, btDispatcher* dispatcher)
	{
		return m_defaultCache->removeOverlappingPair(proxy0, proxy1, dispatcher);
	}

	//Overrided functions
	virtual void cleanProxyFromPairs(btBroadphaseProxy* proxy, btDispatcher* dispatcher);

	virtual void removeOverlappingPairsContainingProxy(btBroadphaseProxy* proxy0, btDispatcher* dispatcher);

	virtual void processAllOverlappingPairs(btOverlapCallback* overlapCallback, btDispatcher* dispatcher);
};

