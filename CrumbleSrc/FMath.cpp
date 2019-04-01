#include "FMath.h"
#include <algorithm>

bool FMath::equal(float a, float b, float epsilon) {
	return (fabs(a - b) <= epsilon * std::max(fabs(a), fabs(b)));
}

bool FMath::greaterTorE(float larger, float smaller, float epsilon) {
	return (larger > smaller || equal(smaller, larger, epsilon));
}

bool FMath::lessThan(float smaller, float larger, float epsilon) {
	return !greaterTorE(smaller, larger, epsilon);
}
