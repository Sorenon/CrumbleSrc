#include "AABB.h"
#include <math.h>
#include <iostream>
#include <btBulletCollisionCommon.h>

#include "FMath.h"
#include "globals.h"
#include "Entity.h"
#include "Scene.h"

AABB AABB::blockAABB = AABB(vec3(0, 0, 0), vec3(1, 1, 1));

AABB::AABB(float x1, float y1, float z1, float x2, float y2, float z2) {
	min.x = std::fmin(x1, x2);
	min.y = std::fmin(y1, y2);
	min.z = std::fmin(z1, z2);

	max.x = std::fmax(x1, x2);
	max.y = std::fmax(y1, y2);
	max.z = std::fmax(z1, z2);
}

AABB::AABB(vec3 min, vec3 max) {
	this->min = min;
	this->max = max;
}

AABB::AABB(btCollisionObject* collObj) {
	btVector3 min;
	btVector3 max;
	collObj->getCollisionShape()->getAabb(collObj->getWorldTransform(), min, max);

	this->min = FMath::convertVector(min);
	this->max = FMath::convertVector(max);
}

AABB AABB::operator+(const vec3& vec) {
	return AABB(min + vec, max + vec);
}

bool AABB::intersectsX(const AABB & other) {
	return other.max.x > min.x&& other.min.x < max.x;

	//return FMath::greaterTorE(other.max.x, min.x) && FMath::lessThan(other.min.x, max.x);
}

bool AABB::intersectsY(const AABB & other) {
	return other.max.y > min.y&& other.min.y < max.y;

	//return FMath::greaterTorE(other.max.y, min.y) && FMath::lessThan(other.min.y, max.y);
}

bool AABB::intersectsZ(const AABB & other) {
	return other.max.z > min.z&& other.min.z < max.z;

	//return FMath::greaterTorE(other.max.z, min.z) && FMath::lessThan(other.min.z, max.z);
}

void AABB::clipY(const AABB & other, float& move) {
	if (intersectsX(other) && intersectsZ(other)) {
		if (move > 0.0f && FMath::greaterTorE(min.y, other.max.y)) {		//Is other rising and bellow AABB?
			if (other.max.y + move > min.y) {								//Would it rise above or inside?
				move = min.y - other.max.y;									//If so limit how far it moves
			}
		}
		else if (move < 0.0f && FMath::greaterTorE(other.min.y, max.y)) {	//Is other decending and above AABB?
			if (other.min.y + move < max.y) {								//Would it move bellow or inside?
				move = max.y - other.min.y;									//If so limit how far it moves
			}
		}
	}
}

void AABB::clipX(const AABB & other, float& move) {
	if (intersectsY(other) && intersectsZ(other)) {
		if (move > 0.0f && FMath::greaterTorE(min.x, other.max.x)) {
			if (other.max.x + move > min.x) {
				move = min.x - other.max.x;
			}
		}
		else if (move < 0.0f && FMath::greaterTorE(other.min.x, max.x)) {
			if (other.min.x + move < max.x) {
				move = max.x - other.min.x;
			}
		}

		//if (move > 0.0f && FMath::greaterTorE(min.x, other.max.x)) {
		//	if (FMath::greaterTorE(other.max.x + move, min.x)) {
		//		move = min.x - other.max.x;
		//	}
		//} else if (move < 0.0f && FMath::greaterTorE(other.min.x, max.x)) {
		//	if (FMath::lessThan(other.min.x + move, max.x)) {
		//		move = max.x - other.min.x;
		//	}
		//}
	}
}

void AABB::clipZ(const AABB & other, float& move) {
	if (intersectsY(other) && intersectsX(other)) {
		if (move > 0.0f && FMath::greaterTorE(min.z, other.max.z)) {
			if (other.max.z + move > min.z) {
				move = min.z - other.max.z;
			}
		}
		else if (move < 0.0f && FMath::greaterTorE(other.min.z, max.z)) {
			if (other.min.z + move < max.z) {
				move = max.z - other.min.z;
			}
		}

		//if (move > 0.0f && FMath::greaterTorE(min.z, other.max.z)) {
		//	if (FMath::greaterTorE(other.max.z + move, min.z)) {
		//		move = min.z - other.max.z;
		//	}
		//} else if (move < 0.0f && FMath::greaterTorE(other.min.z, max.z)) {
		//	if (FMath::lessThan(other.min.z + move, max.z)) {
		//		move = max.z - other.min.z;
		//	}
		//}
	}
}

bool AABB::overlaps(const AABB & other) {
	return (min.x < other.max.x && max.x > other.min.x) &&
		(min.y < other.max.y && max.y > other.min.y) &&
		(min.z < other.max.z && max.z > other.min.z);
}

AABB AABB::expandByVelocity(vec3 velocity) {
	vec3 fMax = max;
	vec3 fMin = min;
	if (velocity.x < 0) {
		fMin.x += velocity.x;
	}
	else {
		fMax.x += velocity.x;
	}

	if (velocity.y < 0) {
		fMin.y += velocity.y;
	}
	else {
		fMax.y += velocity.y;
	}

	if (velocity.z < 0) {
		fMin.z += velocity.z;
	}
	else {
		fMax.z += velocity.z;
	}

	return AABB(fMin, fMax);
}

AABB2D::AABB2D(float widthIn, float heightIn, Face facingIn, vec3 posIn)
{
	width = widthIn;
	height = heightIn;
	facing = facingIn;
	squarePos = posIn;

	vec3 vec1;
	if (facing.normalVector.y == 0) {
		vec1 = posIn + glm::cross(glm::vec3(facing.normalVector), Vectors::UP) * widthIn + glm::vec3(0, height, 0);
	}
	else {
		if (facing == Faces::Down) {//Im too lazy to implement a proper method for this yet
			vec1 = posIn + glm::vec3(width, 0, -height);
		}
		else {
			vec1 = posIn + glm::vec3(width, 0, height);
		}
	}

	min.x = std::fmin(vec1.x, posIn.x);
	min.y = std::fmin(vec1.y, posIn.y);
	min.z = std::fmin(vec1.z, posIn.z);

	max.x = std::fmax(vec1.x, posIn.x);
	max.y = std::fmax(vec1.y, posIn.y);
	max.z = std::fmax(vec1.z, posIn.z);
}

AABB2D AABB2D::operator+(const vec3 & vec)
{
	return AABB2D(width, height, facing, squarePos + vec);
}

bool AABB2D::intersectsX(const AABB & other) {
	return other.max.x > min.x&& other.min.x < max.x;

	//return FMath::greaterTorE(other.max.x, min.x) && FMath::lessThan(other.min.x, max.x);
}

bool AABB2D::intersectsY(const AABB & other) {
	return other.max.y > min.y&& other.min.y < max.y;

	//return FMath::greaterTorE(other.max.y, min.y) && FMath::lessThan(other.min.y, max.y);
}

bool AABB2D::intersectsZ(const AABB & other) {
	return other.max.z > min.z&& other.min.z < max.z;

	//return FMath::greaterTorE(other.max.z, min.z) && FMath::lessThan(other.min.z, max.z);
}

bool AABB2D::intersectsYportal(const AABB & other) { 
	return FMath::greaterTorE(other.max.y, min.y) && FMath::lessThanOrE(other.min.y, max.y);
}

void AABB2D::clipY(const AABB & other, float& move) {
	if (intersectsX(other) && intersectsZ(other)) {
		if (move > 0.0f && FMath::greaterTorE(min.y, other.max.y)) {		//Is other rising and bellow AABB?
			if (other.max.y + move > min.y) {								//Would it rise above or inside?
				move = min.y - other.max.y;									//If so limit how far it moves
			}
		}
		else if (move < 0.0f && FMath::greaterTorE(other.min.y, max.y)) {	//Is other decending and above AABB?
			if (other.min.y + move < max.y) {								//Would it move bellow or inside?
				move = max.y - other.min.y;									//If so limit how far it moves
			}
		}
	}
}

void AABB2D::clipX(const AABB & other, float& move) {
	if (facing == Faces::Right || facing == Faces::Left) {
		if (intersectsY(other) && intersectsZ(other)) {
			if (move > 0.0f && FMath::greaterTorE(min.x, other.max.x)) {
				if (other.max.x + move > min.x) {
					move = min.x - other.max.x;
				}
			}
			else if (move < 0.0f && FMath::greaterTorE(other.min.x, max.x)) {
				if (other.min.x + move < max.x) {
					move = max.x - other.min.x;
				}
			}

			//if (move > 0.0f && FMath::greaterTorE(min.x, other.max.x)) {
			//	if (FMath::greaterTorE(other.max.x + move, min.x)) {
			//		move = min.x - other.max.x;
			//	}
			//} else if (move < 0.0f && FMath::greaterTorE(other.min.x, max.x)) {
			//	if (FMath::lessThan(other.min.x + move, max.x)) {
			//		move = max.x - other.min.x;
			//	}
			//}
		}
	}
}

void AABB2D::clipZ(const AABB & other, float& move) {
	if (facing == Faces::Front) {
		if (intersectsY(other) && intersectsX(other)) {
			if (move < 0.0f && FMath::greaterTorE(other.min.z, max.z)) {
				if (other.min.z + move < max.z) {
					move = max.z - other.min.z;
				}
			}
		}
	}
	else if (facing == Faces::Behind) {
		if (intersectsY(other) && intersectsX(other)) {
			if (move > 0.0f && FMath::greaterTorE(min.z, other.max.z)) {
				if (other.max.z + move > min.z) {
					move = min.z - other.max.z;
				}
			}
		}
	}
}

void AABB2D::portalY(const AABB & other, vec3 & moveVec, Entity * entity, vec3 portalExit) {
	const float yPos = squarePos.y;
	const float move = moveVec.y;
	const float entityY = entity->getEyePos().y;

	Transform& transform = entity->transform;
	bool teleport = false;

	if (facing == Faces::Down && move < 0.0f) {
		if (intersectsX(other) && intersectsZ(other)) {
			if (FMath::greaterTorE(entityY, yPos)) {
				if (entityY + move < yPos) {
					teleport = true;
				}
			}
		}
	}
	else if (facing == Faces::Up && move > 0.0f) {
		if (intersectsX(other) && intersectsZ(other)) {
			if (FMath::lessThanOrE(transform.position.y, yPos)) {
				if (transform.position.y + move > yPos) {
					teleport = true;
				}
			}
		}
	}

	if (teleport) {
		const vec3 differenceFromPortal = transform.position - squarePos;

		transform.position.x = differenceFromPortal.x + portalExit.x;
		transform.position.y = differenceFromPortal.y + portalExit.y;
		transform.position.z = differenceFromPortal.z + portalExit.z;
	}
}


void AABB2D::portalZ(const AABB & other, vec3 & moveVec, Entity * entity, vec3 portalExit) {
	const float zPos = squarePos.z;
	const float move = moveVec.z;
	Transform& transform = entity->transform;
	bool teleport = false;

	if (facing == Faces::Front && move < 0.0f) {
		if (intersectsY(other) && intersectsX(other)) {
			if (FMath::greaterTorE(transform.position.z, zPos)) {
				if (transform.position.z + move < zPos) {
					teleport = true;
				}
			}
		}
	}
	else if (facing == Faces::Behind && move > 0.0f) {
		if (intersectsY(other) && intersectsX(other)) {
			if (FMath::lessThanOrE(transform.position.z, zPos)) {
				if (transform.position.z + move > zPos) {
					teleport = true;
				}
			}
		}
	}

	if (teleport) {
		const vec3 differenceFromPortal = transform.position - squarePos;

		transform.position.z = differenceFromPortal.z + portalExit.z;
		transform.prevPosition.z = transform.position.z;

		transform.position.x = differenceFromPortal.x + portalExit.x;
		transform.prevPosition.x = transform.position.x - moveVec.x;


		transform.position.y = differenceFromPortal.y + portalExit.y;
		transform.prevPosition.y = transform.position.y - moveVec.y;

	}
}
