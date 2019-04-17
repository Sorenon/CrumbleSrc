#pragma once
#include <glm/glm.hpp>
#include <btBulletCollisionCommon.h>

using namespace glm;

class AABB {
public:
	static AABB blockAABB;

public:
	vec3 min;
	vec3 max;

	AABB(float x1, float y1, float z1, float x2, float y2, float z2);
	AABB(vec3 min, vec3 max);
	AABB(btCollisionObject* collObj);

	AABB operator+(const vec3 &vec);

	bool intersectsX(const AABB &other);
	bool intersectsY(const AABB &other);
	bool intersectsZ(const AABB &other);

	void clipX(const AABB &other, float &move);
	void clipY(const AABB &other, float &move);
	void clipZ(const AABB &other, float &move);

	bool overlaps(const AABB &other);

	AABB expandByVelocity(vec3 velocity);
};