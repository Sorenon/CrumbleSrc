#include "Input.h"

#include <glm/glm.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "globals.h"

Input::Input(GLFWwindow* window) {
	glfwSetWindowUserPointer(window, this);

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}


Input::~Input() {
}

void Input::processInput(GLFWwindow * window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS) {
		p_player->velocity = Vectors::ZERO;
		p_player->transform.position = glm::vec3(0, 64, 0);
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_F3 && action == GLFW_PRESS)
		p_player->noClip = !p_player->noClip;

	else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && p_player->onGround)//TODO replace onGround with a collision check for bhopping
		p_player->velocity.y += 9.0f;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {

	RayTraceResult result = world.rayTrace(p_player->getEyePos(), p_player->transform.getLook());

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		if (result.hit) {
			glm::ivec3 placePos = result.hitPos + result.face;

			if (!(AABB::blockAABB + placePos).overlaps(p_player->getLocalBoundingBox())) {
				world.setBlock(placePos.x, placePos.y, placePos.z, 1);
			}
		}
	} else

		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			if (result.hit) {
				world.setBlock(result.hitPos.x, result.hitPos.y, result.hitPos.z, 0);
			}
		}
}

bool firstMouse = true;
float yaw = 0.0f;
float pitch = 0.0f;
float lastX = 0.0f;
float lastY = 0.0f;

void mouse_callback(GLFWwindow * window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.15f;
	xoffset *= sensitivity;
	yoffset *= -sensitivity;

	glm::vec3 &rotation = p_player->transform.rotation;

	rotation.x += glm::radians(yoffset);
	rotation.y += glm::radians(xoffset);

	if (rotation.x > glm::radians(90.0f))
		rotation.x = glm::radians(90.0f);
	if (rotation.x < -glm::radians(90.0f))
		rotation.x = -glm::radians(90.0f);
}

bool KeyBinding::execute() {
	if (timesPressed > 0) {
		timesPressed--;
		return true;
	}

	return false;
}

bool KeyBinding::isDown() {
	switch (inputType) {
	case Key: return 
	}

	return false;
}
