#pragma once
#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

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
	KeyBinding(int keyID);

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
	static Input INSTANCE;
	GLFWwindow* window;

public:
	KeyBinding kbForaward = KeyBinding(GLFW_KEY_W);
	KeyBinding kbBackward = KeyBinding(GLFW_KEY_S);
	KeyBinding kbLeft  = KeyBinding(GLFW_KEY_A);
	KeyBinding kbRight = KeyBinding(GLFW_KEY_D);

	InputAxis axForward = InputAxis(&kbForaward, &kbBackward);
	InputAxis axRight = InputAxis(&kbRight, &kbLeft);

public:
	void init(GLFWwindow* window);

	void processInput();
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

private:
	Input();
};

