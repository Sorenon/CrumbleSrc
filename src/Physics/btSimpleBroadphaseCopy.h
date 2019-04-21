/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#pragma once
//I copied this file from the bullet sources and made multiple functions virtual

#include <btBulletCollisionCommon.h>

struct btSimpleBroadphaseCopyProxy : public btBroadphaseProxy
{
	int m_nextFree;

	//	int			m_handleId;

	btSimpleBroadphaseCopyProxy(){};

	btSimpleBroadphaseCopyProxy(const btVector3& minpt, const btVector3& maxpt, int shapeType, void* userPtr, int collisionFilterGroup, int collisionFilterMask)
		: btBroadphaseProxy(minpt, maxpt, userPtr, collisionFilterGroup, collisionFilterMask)
	{
		(void)shapeType;
	}

	SIMD_FORCE_INLINE void SetNextFree(int next) { m_nextFree = next; }
	SIMD_FORCE_INLINE int GetNextFree() const { return m_nextFree; }
};

///The SimpleBroadphase is just a unit-test for btAxisSweep3, bt32BitAxisSweep3, or btDbvtBroadphase, so use those classes instead.
///It is a brute force aabb culling broadphase based on O(n^2) aabb checks
class btSimpleBroadphaseCopy : public btBroadphaseInterface
{
protected:
	int m_numHandles;  // number of active handles
	int m_maxHandles;  // max number of handles
	int m_LastHandleIndex;

	btSimpleBroadphaseCopyProxy* m_pHandles;  // handles pool

	void* m_pHandlesRawPtr;
	int m_firstFreeHandle;  // free handles list

	int allocHandle()
	{
		btAssert(m_numHandles < m_maxHandles);
		int freeHandle = m_firstFreeHandle;
		m_firstFreeHandle = m_pHandles[freeHandle].GetNextFree();
		m_numHandles++;
		if (freeHandle > m_LastHandleIndex)
		{
			m_LastHandleIndex = freeHandle;
		}
		return freeHandle;
	}

	void freeHandle(btSimpleBroadphaseCopyProxy* proxy)
	{
		int handle = int(proxy - m_pHandles);
		btAssert(handle >= 0 && handle < m_maxHandles);
		if (handle == m_LastHandleIndex)
		{
			m_LastHandleIndex--;
		}
		proxy->SetNextFree(m_firstFreeHandle);
		m_firstFreeHandle = handle;

		proxy->m_clientObject = 0;

		m_numHandles--;
	}

	btOverlappingPairCache* m_pairCache;
	bool m_ownsPairCache;

	int m_invalidPair;

	inline btSimpleBroadphaseCopyProxy* getSimpleProxyFromProxy(btBroadphaseProxy* proxy)
	{
		btSimpleBroadphaseCopyProxy* proxy0 = static_cast<btSimpleBroadphaseCopyProxy*>(proxy);
		return proxy0;
	}

	inline const btSimpleBroadphaseCopyProxy* getSimpleProxyFromProxy(btBroadphaseProxy* proxy) const
	{
		const btSimpleBroadphaseCopyProxy* proxy0 = static_cast<const btSimpleBroadphaseCopyProxy*>(proxy);
		return proxy0;
	}

	///reset broadphase internal structures, to ensure determinism/reproducability
	virtual void resetPool(btDispatcher* dispatcher);

	void validate();

protected:
public:
	btSimpleBroadphaseCopy(int maxProxies = 16384, btOverlappingPairCache* overlappingPairCache = 0);
	virtual ~btSimpleBroadphaseCopy();

	static bool aabbOverlap(btSimpleBroadphaseCopyProxy* proxy0, btSimpleBroadphaseCopyProxy* proxy1);

	virtual btBroadphaseProxy* createProxy(const btVector3& aabbMin, const btVector3& aabbMax, int shapeType, void* userPtr, int collisionFilterGroup, int collisionFilterMask, btDispatcher* dispatcher);

	virtual void calculateOverlappingPairs(btDispatcher* dispatcher);

	virtual void destroyProxy(btBroadphaseProxy* proxy, btDispatcher* dispatcher);
	virtual void setAabb(btBroadphaseProxy* proxy, const btVector3& aabbMin, const btVector3& aabbMax, btDispatcher* dispatcher);
	virtual void getAabb(btBroadphaseProxy* proxy, btVector3& aabbMin, btVector3& aabbMax) const;

	virtual void rayTest(const btVector3& rayFrom, const btVector3& rayTo, btBroadphaseRayCallback& rayCallback, const btVector3& aabbMin = btVector3(0, 0, 0), const btVector3& aabbMax = btVector3(0, 0, 0));
	virtual void aabbTest(const btVector3& aabbMin, const btVector3& aabbMax, btBroadphaseAabbCallback& callback);

	virtual btOverlappingPairCache* getOverlappingPairCache()
	{
		return m_pairCache;
	}
	virtual const btOverlappingPairCache* getOverlappingPairCache() const
	{
		return m_pairCache;
	}

	bool testAabbOverlap(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1);

	///getAabb returns the axis aligned bounding box in the 'global' coordinate frame
	///will add some transform later
	virtual void getBroadphaseAabb(btVector3& aabbMin, btVector3& aabbMax) const
	{
		aabbMin.setValue(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
		aabbMax.setValue(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
	}

	virtual void printStats()
	{
		//		printf("btSimpleBroadphaseCopy.h\n");
		//		printf("numHandles = %d, maxHandles = %d\n",m_numHandles,m_maxHandles);
	}
};