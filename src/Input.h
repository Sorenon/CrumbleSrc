#pragma once
#include <GLFW/glfw3.h>

class KeyBinding {
public:
	int id;
	InputType inputType;
	GLFWwindow* window;

	int timesPressed = 0;

	bool execute();
	bool isDown();
};

enum InputType {
	Key,
	Mouse,
	Scroll
};

class Input {
public:
	Input(GLFWwindow* window);
	~Input();

	void processInput(GLFWwindow *window);
	//void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	//void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
};

