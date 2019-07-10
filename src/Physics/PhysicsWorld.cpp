#include "Physics/PhysicsWorld.h"

#include <cmath>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/NarrowPhaseCollision/btPolyhedralContactClipping.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Physics/bcSimpleBroadphase.h"
#include "globals.h"
#include "Scene.h"
#include "Portal.h"
#include "FMath.h"
#include "Plane.h"

PhysicsWorld::PhysicsWorld()
{
	m_pairCache = new bcSimpleBroadphase();
	//overlappingPairCache = new btDbvtBroadphase();
	m_collisionConfiguration = new btDefaultCollisionConfiguration();
	m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
	m_solver = new btSequentialImpulseConstraintSolver;

	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_pairCache, m_solver, m_collisionConfiguration);
	m_dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
	m_dynamicsWorld->setInternalTickCallback(preTickStatic, 0, true);

	m_debugDrawer = new bcDebugDrawer();
	m_dynamicsWorld->setDebugDrawer(m_debugDrawer);
	m_debugDrawer->setDebugMode(m_debugDrawer->DBG_DrawWireframe);

	{
		((bcSimpleBroadphase*)m_pairCache)->m_collisionWorld = m_dynamicsWorld;
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
		m_cachedCollisionShapes.push_back(boxCollisionShape);

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
		m_rbCube = new btRigidBody(rigidBodyCI);
		//rbCube->setActivationState(DISABLE_DEACTIVATION);

		m_dynamicsWorld->addRigidBody(m_rbCube);

		//m_rbCube->setAngularFactor(0);
		m_rbCube->setLinearFactor({ 0, 0, 0 });
	}
}

PhysicsWorld::~PhysicsWorld()
{
	for (int iCollObj = m_dynamicsWorld->getNumCollisionObjects() - 1; iCollObj >= 0; iCollObj--)
	{
		btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[iCollObj];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body != nullptr && body->getMotionState())
		{
			delete body->getMotionState();
		}
		m_dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	for (btCollisionShape*& shape : m_cachedCollisionShapes)
	{
		delete shape;
		shape = nullptr;
	}

	for (btCollisionShape*& shape : m_tmpCollisionShapes)
	{
		delete shape;
		shape = nullptr;
	}

	delete m_dynamicsWorld;
	delete m_solver;
	delete m_pairCache;
	delete m_dispatcher;
	delete m_collisionConfiguration;
}

void PhysicsWorld::preTickStatic(btDynamicsWorld* world, btScalar timeStep)
{
	p_physicsWorld->preTick(world, timeStep);
}

void PhysicsWorld::preTick(btDynamicsWorld* world, btScalar timeStep)
{
	using namespace FMath;

	for (btCollisionShape* shape : m_tmpCollisionShapes)
	{
		delete shape;
	}
	m_tmpCollisionShapes.clear();

	//if (true)
	//	return;

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

		Portal& portal = scene.portals[0];
		btVector3 rbPos = m_rbCube->getWorldTransform().getOrigin();
		btVector3 portalPos = btglm_conversion::convertVector(portal.getPosition());

		btVector3 relitivePos = portalPos - rbPos;

		glm::quat rotation = btglm_conversion::convertQuaternion(m_rbCube->getWorldTransform().getRotation());
		//rotation.y = z;
		//rotation.z = y;

		const float x = rotation.x;
		const float y = rotation.y;
		const float z = rotation.z;

		//rotation.x = -x;
		//rotation.y = -y;
		//rotation.z = -z;

		glm::vec3 normal = glm::inverse(rotation) * glm::vec3(portal.getFacing().normalVector);

		float distance = 0.0f;

		//Plane plane = Plane(glm::vec3(0, 0, 0), portal.getFacing().angle * glm::inverse(btglm_conversion::convertQuaternion(m_rbCube->getWorldTransform().getRotation())));

		for (const btVector3* side : sides)//Each side is cut individually for a more simple final mesh (TODO: find if this is true)
		{
			btVertexArray inputVerticies;
			btVertexArray outputVerticies;

			for (int i = 0; i < 4; i++)
			{
				inputVerticies.push_back(side[i] - btVector3(0.5f, 0.5f, 0.5f));
			}

			//btPolyhedralContactClipping::clipFace(inputVerticies, outputVerticies, btglm_conversion::convertVector(plane.getNormal()), plane.getOffset()/* + 0.04f*/);
			btPolyhedralContactClipping::clipFace(inputVerticies, outputVerticies, btglm_conversion::convertVector(normal), distance);

			for (int i = 0; i < outputVerticies.size(); i++)
			{
				btVector3& newVertex = outputVerticies[i];

				for (int i2 = 0; i2 < finalVerticies.size(); i2++)
				{//Only add new verticies
					btVector3& existingVertex = finalVerticies[i2];

					if (newVertex == existingVertex)
					{
						goto skip;
					}
				}

				finalVerticies.push_back(newVertex);
			skip:;
			}
		}
	}

	if (finalVerticies.size() != 0)
	{
		btConvexHullShape* cutBoxShape = new btConvexHullShape();
		//boxCollisionShape->setMargin(0.0f);//btBoxShape doesnt have an outward margin
		cutBoxShape->setMargin(0.02f);

		for (int i = 0; i < finalVerticies.size(); i++)
		{
			cutBoxShape->addPoint(finalVerticies[i], i == finalVerticies.size() - 1);
		}

		m_rbCube->setCollisionShape(cutBoxShape);
		m_tmpCollisionShapes.push_back(cutBoxShape);
	}
	else
	{
		m_rbCube->setCollisionShape(&m_emptyShape);
	}
}
