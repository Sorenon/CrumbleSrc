#pragma once

#include <btBulletCollisionCommon.h>

#include "btSimpleBroadphaseCopy.h"

class bcSimpleBroadphase : public btSimpleBroadphaseCopy {
public:
	bcSimpleBroadphase();
	~bcSimpleBroadphase(); 

	virtual void calculateOverlappingPairs(btDispatcher* dispatcher);
	virtual btOverlappingPairCache* getOverlappingPairCache();
	virtual const btOverlappingPairCache* getOverlappingPairCache() const;
};

