#include "bcOverlappingPairCache.h"
#include <list>

#include <btBulletCollisionCommon.h>


void bcOverlappingPairCache::cleanProxyFromPairs(btBroadphaseProxy* proxy, btDispatcher* dispatcher)
{
	m_defaultCache->cleanProxyFromPairs(proxy, dispatcher);//TODO
}

void bcOverlappingPairCache::removeOverlappingPairsContainingProxy(btBroadphaseProxy* proxy0, btDispatcher* dispatcher)
{
	return m_defaultCache->removeOverlappingPairsContainingProxy(proxy0, dispatcher);//TODO
}

void bcOverlappingPairCache::processAllOverlappingPairs(btOverlapCallback* overlapCallback, btDispatcher* dispatcher)
{
	m_defaultCache->processAllOverlappingPairs(overlapCallback, dispatcher);

	for (std::list<btBroadphasePair>::iterator iterator = m_blockWorldCollisions.begin(), end = m_blockWorldCollisions.end(); iterator != end; ++iterator)
	{
		if (overlapCallback->processOverlap((*iterator)))
		{
			m_blockWorldCollisions.erase(iterator);
		}
	}
}