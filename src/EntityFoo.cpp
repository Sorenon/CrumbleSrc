#include "EntityFoo.h"

#include "globals.h"

EntityFoo::EntityFoo() {
	//collider = AABB(vec3(-0.25f, 0, -0.25f), vec3(0.25f, 0.5f, 0.25f));
	collider = AABB(vec3(-0.5f, 0, -0.5f), vec3(0.5f, 1, 0.5f));
}


EntityFoo::~EntityFoo() {
}

void EntityFoo::UpdateSingleThread() {

}

void EntityFoo::UpdateMultiThread() {
	transform.step();

	velocity.y -= 28 * CrumbleGlobals::FIXED_TIMESTEP;			//Apply gravity

	Move();
}
