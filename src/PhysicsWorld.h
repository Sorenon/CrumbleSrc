#pragma once

#include <btBulletDynamicsCommon.h>


class PhysicsWorld {
public:
	btBroadphaseInterface*                  overlappingPairCache;
	btDefaultCollisionConfiguration*        collisionConfiguration;
	btCollisionDispatcher*                  dispatcher;
	btSequentialImpulseConstraintSolver*    solver;
	btDiscreteDynamicsWorld*                dynamicsWorld;
	//bcDebugDrawer* debugDraw;

	btAlignedObjectArray<btCollisionShape*> collisionShapes;
	btRigidBody* rbCube;

public:
	PhysicsWorld();
	~PhysicsWorld();
};

