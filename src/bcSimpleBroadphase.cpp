#include "bcSimpleBroadphase.h"

#include <btBulletCollisionCommon.h>

#include "btSimpleBroadphaseCopy.h"
#include "globals.h"

bcSimpleBroadphase::bcSimpleBroadphase() {
}


bcSimpleBroadphase::~bcSimpleBroadphase() {
}

void bcSimpleBroadphase::calculateOverlappingPairs(btDispatcher * dispatcher) {
	btSimpleBroadphaseCopy::calculateOverlappingPairs(dispatcher);
}

btOverlappingPairCache* bcSimpleBroadphase::getOverlappingPairCache() {
	return btSimpleBroadphaseCopy::getOverlappingPairCache();
}

const btOverlappingPairCache* bcSimpleBroadphase::getOverlappingPairCache() const {
	return btSimpleBroadphaseCopy::getOverlappingPairCache();
}
