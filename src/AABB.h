#pragma once
#include <glm/glm.hpp>
#include <btBulletCollisionCommon.h>

#include "Faces.h"

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

class Entity;

class AABB2D {
private:
	float width;
	float height;
	Face facing;
	vec3 squarePos;

	vec3 min;
	vec3 max;
public:
	AABB2D(float width, float height, Face facing, vec3 pos);

	AABB2D operator+(const vec3& vec);

	//Does other overlap me
	bool intersectsX(const AABB& other);
	bool intersectsY(const AABB& other);
	bool intersectsZ(const AABB& other);

	//Allows intersection detection with portals even when it is inline with walls
	bool intersectsEpsilonX(const AABB& other);
	bool intersectsEpsilonY(const AABB& other);
	bool intersectsEpsilonZ(const AABB& other);

	//Is other fully contained within me
	bool surroundsX(const AABB& other);
	bool surroundsY(const AABB& other);
	bool surroundsZ(const AABB& other);

	void clipX(const AABB& other, float& move);
	void clipY(const AABB& other, float& move);
	void clipZ(const AABB& other, float& move);

	void portalY(const AABB& other, vec3& moveVec, Entity* entity, vec3 portalExit);
	void portalZ(const AABB& other, vec3& moveVec, Entity* entity, vec3 portalExit);
};