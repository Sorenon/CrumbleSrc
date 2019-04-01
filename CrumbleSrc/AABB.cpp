#include "AABB.h"
#include <math.h>
#include <iostream>

#include "FMath.h"

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

AABB AABB::operator+(const vec3 & vec) {
	return AABB(min + vec, max + vec);
}

void AABB::clipY(const AABB & other, float & move) {
	if (move < 0.0f && FMath::greaterThan(other.min.y, this->max.y)) {	//If falling and above AABB
		if (other.min.y + move < max.y) {		//Would it move bellow or inside?
			move = max.y - other.min.y;			//If so limit how far it moves
		}
	}
}
