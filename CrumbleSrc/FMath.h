#pragma once
#include <float.h>

namespace FMath {
	bool equal(float a, float b, float epsilon = FLT_EPSILON);

	/*
	Are two floats greater than or equal to eachother
	*/
	bool greaterTorE(float larger, float smaller, float epsilon = FLT_EPSILON);

	bool lessThan(float smaller, float larger, float epsilon = FLT_EPSILON);
};

