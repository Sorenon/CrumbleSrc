#include "PhysicsWorld.h"

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/NarrowPhaseCollision/btPolyhedralContactClipping.h>

#include "bcSimpleBroadphase.h"

PhysicsWorld::PhysicsWorld() {
	overlappingPairCache = new bcSimpleBroadphase();
	//overlappingPairCache = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));

	debugDraw = new bcDebugDrawer();
	dynamicsWorld->setDebugDrawer(debugDraw);
	debugDraw->setDebugMode(debugDraw->DBG_DrawWireframe);

	{
		((bcSimpleBroadphase*)overlappingPairCache)->collisionWorld = dynamicsWorld;
	}

	//{
	//	//btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
	//	btCollisionShape* groundShape = new btBoxShape(btVector3(20, 1, 20));

	//	btTransform startTransform;
	//	startTransform.setIdentity();

	//	btScalar mass(0.f);

	//	//startTransform.setOrigin(btVector3(0, 63.5f, 0));
	//	startTransform.setOrigin(btVector3(0, 63, 0));

	//	btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);

	//	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, groundShape, btVector3(0, 0, 0));
	//	btRigidBody* rb = new btRigidBody(rbInfo);

	//	dynamicsWorld->addRigidBody(rb);
	//}

	{
		//btCollisionShape* boxCollisionShape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
		btVertexArray outputVectors;

		{
			btVertexArray inputVectors;

			const btVector3 front[] = {
				btVector3(0.0f, 0.0f, 0.0f),
				btVector3(1.0f, 0.0f, 0.0f),
				btVector3(1.0f, 1.0f, 0.0f),
				btVector3(0.0f, 1.0f, 0.0f),
			};

			const btVector3 left[] = {
				btVector3(0.0f, 0.0f, 0.0f),
				btVector3(0.0f, 0.0f, 1.0f),
				btVector3(0.0f, 1.0f, 1.0f),
				btVector3(0.0f, 1.0f, 0.0f),
			};

			const btVector3 bottom[] = {
				btVector3(0.0f, 0.0f, 0.0f),
				btVector3(0.0f, 0.0f, 1.0f),
				btVector3(1.0f, 0.0f, 1.0f),
				btVector3(1.0f, 0.0f, 0.0f),
			};

			const btVector3 back[] = {
				btVector3(0.0f, 0.0f, 1.0f),
				btVector3(1.0f, 0.0f, 1.0f),
				btVector3(1.0f, 1.0f, 1.0f),
				btVector3(0.0f, 1.0f, 1.0f),
			};

			const btVector3 right[] = {
				btVector3(1.0f, 0.0f, 0.0f),
				btVector3(1.0f, 0.0f, 1.0f),
				btVector3(1.0f, 1.0f, 1.0f),
				btVector3(1.0f, 1.0f, 0.0f),
			};

			const btVector3 top[] = {
				btVector3(0.0f, 1.0f, 0.0f),
				btVector3(0.0f, 1.0f, 1.0f),
				btVector3(1.0f, 1.0f, 1.0f),
				btVector3(1.0f, 1.0f, 0.0f),
			};

			const btVector3* sides[] = {
				front,
				left,
				bottom,
				back,
				right,
				top,
			};

			for (const btVector3* side : sides) {
				for (int i = 0; i < 4; i++) {
					inputVectors.push_back(side[i] - btVector3(0.5f, 0.5f, 0.5f));
				}
			}

			btPolyhedralContactClipping::clipFace(inputVectors, outputVectors, btVector3(0, -1, 0), 0.0f);//TODO: Call once for each side
		}

		btConvexHullShape* boxCollisionShape = new btConvexHullShape();

		for (int i = 0; i < outputVectors.size(); i++) {
			btVector3& vec = outputVectors[i];
			boxCollisionShape->addPoint(vec);
		}

		btTransform trans;
		trans.setIdentity();
		trans.setOrigin(btVector3(5, 70, 5));
		btDefaultMotionState* motionstate = new btDefaultMotionState(trans);

		int mass = 1;
		btVector3 localInertia;
		boxCollisionShape->calculateLocalInertia(mass, localInertia);

		btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionstate, boxCollisionShape, localInertia);
		rigidBodyCI.m_restitution = 0.2f;
		rigidBodyCI.m_friction = 0.91f;
		rbCube = new btRigidBody(rigidBodyCI);
		//rbCube->setActivationState(DISABLE_DEACTIVATION);

		dynamicsWorld->addRigidBody(rbCube);

		//rbCube->setAngularFactor(0);
		//rbCube->setLinearFactor({ 0, 0, 0 });
	}
}


PhysicsWorld::~PhysicsWorld() {
	for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState()) {
			delete body->getMotionState();
		}
		dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	for (int j = 0; j < collisionShapes.size(); j++) {
		btCollisionShape* shape = collisionShapes[j];
		collisionShapes[j] = 0;
		delete shape;
	}

	delete dynamicsWorld;
	delete solver;
	delete overlappingPairCache;
	delete dispatcher;
	delete collisionConfiguration;
}
