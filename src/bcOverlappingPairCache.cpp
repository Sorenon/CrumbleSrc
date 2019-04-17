#include "bcOverlappingPairCache.h"
#include <list>

#include <btBulletCollisionCommon.h>


void bcOverlappingPairCache::cleanProxyFromPairs(btBroadphaseProxy * proxy, btDispatcher * dispatcher) {
	defaultCache->cleanProxyFromPairs(proxy, dispatcher);
}

void bcOverlappingPairCache::removeOverlappingPairsContainingProxy(btBroadphaseProxy * proxy0, btDispatcher * dispatcher) {
	return defaultCache->removeOverlappingPairsContainingProxy(proxy0, dispatcher);
	//TODO
}

void bcOverlappingPairCache::processAllOverlappingPairs(btOverlapCallback * overlapCallback, btDispatcher * dispatcher) {
	defaultCache->processAllOverlappingPairs(overlapCallback, dispatcher);

	for (std::list<btBroadphasePair>::iterator iterator = worldCollisions.begin(), end = worldCollisions.end(); iterator != end; ++iterator) {
		/*if ()) {
			worldCollisions.erase(iterator);
		}*/
		overlapCallback->processOverlap((*iterator));
	}
}