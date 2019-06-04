#include "PhysicsWorld.h"

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/NarrowPhaseCollision/btPolyhedralContactClipping.h>

#include "bcSimpleBroadphase.h"
#include "../globals.h"
#include "../Scene.h"
#include "../Portal.h"
#include "../FMath.h"

PhysicsWorld::PhysicsWorld() {
	overlappingPairCache = new bcSimpleBroadphase();
	//overlappingPairCache = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
	dynamicsWorld->setInternalTickCallback(preTickStatic, 0, true);

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
		btCollisionShape* boxCollisionShape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
		collisionShapes.push_back(boxCollisionShape);

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

		rbCube->setAngularFactor(0);
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

	for (btCollisionShape*& shape : collisionShapes) {
		shape = nullptr;
		delete shape;
	}

	for (btCollisionShape*& shape : tmpCollisionShapes) {
		shape = nullptr;
		delete shape;
	}

	delete dynamicsWorld;
	delete solver;
	delete overlappingPairCache;
	delete dispatcher;
	delete collisionConfiguration;
}

void PhysicsWorld::preTickStatic(btDynamicsWorld* world, btScalar timeStep) {
	p_physicsWorld->preTick(world, timeStep);
}

void PhysicsWorld::preTick(btDynamicsWorld* world, btScalar timeStep) {
	for (btCollisionShape* shape : tmpCollisionShapes) {
		delete shape;
	}
	tmpCollisionShapes.clear();

	//btCompoundShape* combinedShape = new btCompoundShape();

	btVertexArray finalVerticies;
	{
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
			btVertexArray inputVerticies;
			btVertexArray outputVerticies;

			for (int i = 0; i < 4; i++) {
				inputVerticies.push_back(side[i] - btVector3(0.5f, 0.5f, 0.5f));
			}

			Portal& portal = scene.portals[0];
			btVector3 rbPos = rbCube->getWorldTransform().getOrigin();
			btVector3 portalPos = FMath::convertVector(portal.getPosition());
			
			btVector3 relitivePos = portalPos - rbPos;

			btPolyhedralContactClipping::clipFace(inputVerticies, outputVerticies, btVector3(0, -1, 0), relitivePos.getY() + 0.04f);//Each side is cut individually for a more simple final mesh

			for (int i = 0; i < outputVerticies.size(); i++) {
				btVector3& newVertex = outputVerticies[i];

				for (int i2 = 0; i2 < finalVerticies.size(); i2++) {//Only add new verticies
					btVector3& existingVertex = finalVerticies[i2];

					if (newVertex == existingVertex) {
						goto skip;
					}
				}

				finalVerticies.push_back(newVertex);
			skip:;
			}
		}
	}

	if (finalVerticies.size() != 0) {
		btConvexHullShape* cutBoxShape = new btConvexHullShape();
		//boxCollisionShape->setMargin(0.0f);//btBoxShape doesnt have an outward margin
		cutBoxShape->setMargin(0.02f);

		for (int i = 0; i < finalVerticies.size(); i++) {
			cutBoxShape->addPoint(finalVerticies[i], i == finalVerticies.size() - 1);
		}

		rbCube->setCollisionShape(cutBoxShape);
		tmpCollisionShapes.push_back(cutBoxShape);
	}
	else {
		rbCube->setCollisionShape(&emptyShape);
	}
}
