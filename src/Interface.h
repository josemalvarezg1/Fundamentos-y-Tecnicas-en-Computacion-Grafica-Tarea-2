#pragma once
#include "Main.h"
#include "Model.h"

class Interface {
	public:
		static void TwEventMouseButtonGLFW3(GLFWwindow* window, int button, int action, int mods);
		static void cursorPos(GLFWwindow* window, double x, double y);
		static void TwEventMousePosGLFW3(GLFWwindow* window, double x, double y);
		static void cursorScroll(GLFWwindow* window, double xoffset, double yoffset);
		static void TwEventKeyGLFW3(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void TwEventCharGLFW3(GLFWwindow* window, int codepoint);
		static void TwWindowSizeGLFW3(GLFWwindow* window, int width, int height);
		static void initAntTweakBar();
};