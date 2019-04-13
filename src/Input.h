#pragma once
#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <vector>

enum InputType {
	Key,
	Mouse,
	Scroll
};

class KeyBinding {
public:
	int keyID;
	InputType inputType;
	GLFWwindow* window;

	int timesPressed = 0;

public:
	KeyBinding(InputType inputType, int keyID);

	bool execute();
	bool isDown();
};

class InputAxis {
public:
	KeyBinding* positive;
	KeyBinding* negative;

public:
	InputAxis(KeyBinding* positive, KeyBinding* negative);

	int getModifier();
};

class Input {
public:
	bool firstMouse = true;
	float lastX = 0.0f;
	float lastY = 0.0f;
	float deltaX = 0.0f;
	float deltaY = 0.0f;

	static Input INSTANCE;
	GLFWwindow* window;

public:
	KeyBinding kbForward = KeyBinding(InputType::Key, GLFW_KEY_W);
	KeyBinding kbBackward = KeyBinding(InputType::Key, GLFW_KEY_S);
	KeyBinding kbLeft  = KeyBinding(InputType::Key, GLFW_KEY_A);
	KeyBinding kbRight = KeyBinding(InputType::Key, GLFW_KEY_D);
	KeyBinding kbJump = KeyBinding(InputType::Key, GLFW_KEY_SPACE);
	KeyBinding kbCrouch = KeyBinding(InputType::Key, GLFW_KEY_LEFT_SHIFT);

	KeyBinding kbAttack = KeyBinding(InputType::Mouse, GLFW_MOUSE_BUTTON_LEFT);
	KeyBinding kbPlace = KeyBinding(InputType::Mouse, GLFW_MOUSE_BUTTON_RIGHT);

	std::vector<KeyBinding*> keybinds = {&kbForward, &kbBackward, &kbLeft, &kbRight, &kbJump, &kbCrouch, &kbAttack, &kbPlace};

	InputAxis axForward = InputAxis(&kbForward, &kbBackward);
	InputAxis axRight = InputAxis(&kbRight, &kbLeft);
	InputAxis axUp = InputAxis(&kbJump, &kbCrouch);

public:
	void init(GLFWwindow* window);

	void processInput();
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

private:
	Input();
};

