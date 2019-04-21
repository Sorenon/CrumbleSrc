#include "EntityFoo.h"



EntityFoo::EntityFoo() {
	collider = AABB(vec3(-0.25f, 0, -0.25f), vec3(0.25f, 0.5f, 0.25f));
}


EntityFoo::~EntityFoo() {
}

void EntityFoo::UpdateSingleThread() {
}

void EntityFoo::UpdateMultiThread() {
}
