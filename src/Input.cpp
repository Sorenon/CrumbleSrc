#include "Input.h"

#include <glm/glm.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "globals.h"

Input Input::INSTANCE = Input();

Input::Input() {}

void Input::init(GLFWwindow * windowIn) {
	window = windowIn;

	glfwSetWindowUserPointer(window, this);

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	for (KeyBinding *binding : keybinds) {
		binding->window = window;
	}
}

void Input::processInput() {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}


void Input::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	Input* input = (Input*) glfwGetWindowUserPointer(window);
	if (action == GLFW_PRESS) {
		for (KeyBinding *binding : input->keybinds) {

			if (binding->inputType == InputType::Key && binding->keyID == key) {
				binding->timesPressed++;
				break;
			}
		}
	}
}

void Input::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	Input* input = (Input*)glfwGetWindowUserPointer(window);
	if (action == GLFW_PRESS) {
		for (KeyBinding *binding : input->keybinds) {

			if (binding->inputType == InputType::Mouse && binding->keyID == button) {
				binding->timesPressed++;
				break;
			}
		}
	}
}

void Input::mouse_callback(GLFWwindow * window, double xpos, double ypos) {
	Input* input = (Input*)glfwGetWindowUserPointer(window);

	if (input->firstMouse) {
		input->lastX = xpos;
		input->lastY = ypos;
		input->firstMouse = false;
	}

	float xoffset = xpos - input->lastX;
	float yoffset = input->lastY - ypos; // reversed since y-coordinates go from bottom to top
	input->lastX = xpos;
	input->lastY = ypos;

	input->deltaX += xoffset;
	input->deltaY += yoffset;
}

KeyBinding::KeyBinding(InputType inputTypeIn, int keyIDIn) {
	inputType = inputTypeIn;
	keyID = keyIDIn;
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
	case InputType::Key: return glfwGetKey(window, keyID) == GLFW_PRESS;
	case InputType::Mouse: return glfwGetMouseButton(window, keyID) == GLFW_PRESS;
	case InputType::Scroll: return false;
	}
}

bool KeyBinding::executeOnce() {
	if (timesPressed > 0) {
		timesPressed = 0;
		return true;
	}
	return false;
}

InputAxis::InputAxis(KeyBinding * positiveIn, KeyBinding * negativeIn) {
	positive = positiveIn;
	negative = negativeIn;
}

int InputAxis::getModifier() {
	return positive->isDown() - negative->isDown(); //<3 c++
}