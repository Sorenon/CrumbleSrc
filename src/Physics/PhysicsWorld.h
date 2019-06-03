#pragma once
#include <vector>

#include <btBulletDynamicsCommon.h>

struct bcDebugDrawer : btIDebugDraw {
public:
	int mode = 0;
	std::vector<float> vertices;
	int count = 0;

	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
		float *point = makePoint(from.getX(), from.getY(), from.getZ(), color.getX(), color.getY(), color.getZ());
		vertices.insert(vertices.end(), point, &point[5]);
		delete[] point;

		point = makePoint(to.getX(), to.getY(), to.getZ(), color.getX(), color.getY(), color.getZ());
		vertices.insert(vertices.end(), point, &point[5]);
		delete[] point;

		count += 2;
	}

	virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {}

	virtual void reportErrorWarning(const char* warningString) {}

	virtual void draw3dText(const btVector3& location, const char* textString) {}

	virtual void setDebugMode(int debugMode) {
		mode = debugMode;
	}

	virtual int getDebugMode() const {
		return mode;
	}

	float * makePoint(float x, float y, float z, float r, float g, float b) {
		//return new float[6]{x, y, z, r, g, b};
		return new float[5]{ x, y, z, 0, 0 };
	}
};

class PhysicsWorld {
public:
	btBroadphaseInterface*                  overlappingPairCache;
	btDefaultCollisionConfiguration*        collisionConfiguration;
	btCollisionDispatcher*                  dispatcher;
	btSequentialImpulseConstraintSolver*    solver;
	btDiscreteDynamicsWorld*                dynamicsWorld;
	bcDebugDrawer* debugDraw;

	btRigidBody* rbCube;

	std::vector<btCollisionShape*> collisionShapes;
	std::vector<btCollisionShape*> tmpCollisionShapes;//Deleted at the begining of each physics tick
	btEmptyShape emptyShape = btEmptyShape();

public:
	PhysicsWorld();
	~PhysicsWorld();

	static void preTickStatic(btDynamicsWorld* world, btScalar timeStep);
	void preTick(btDynamicsWorld* world, btScalar timeStep);
};

