#include "Interface.h"

extern vector<model> models; //Todos los modelos irán en este vector
extern model m;
extern TwBar *menuTW, *modelTW;
extern Camera camera;
extern int width, height, selectedModel, autoRotar, tiempoRotacion;
extern bool activateCamera, toggleTexture, toggleNormalTexture, toggleLightTexture, toggleLight, toggleShadows, toggleDepth, lastToggleDepth, firstMouse, selecting;
extern double xGlobal, yGlobal;
extern bool keys[1024]; //Todas las teclas				
extern float rotacionInicial[4], rotacionPrincipal[4], lightDirection[4], color_material_ambiental[4], color_material_difuso[4], color_material_especular[4], color_luz_ambiental[4], color_luz_difuso[4], color_luz_especular[4];
extern float shinyBlinn, scaleT, ejeX, ejeY, ejeZ, ejeXL, ejeYL, ejeZL, constant, linear, quadratic, inCos, outCos, refractor, bias;
extern GLfloat lastX, lastY;
extern lightning currentLight;
extern glm::mat4 project_mat; //Matriz de Proyección
extern glm::mat4 view_mat; //Matriz de View

void Interface::TwEventMouseButtonGLFW3(GLFWwindow* window, int button, int action, int mods) {
	if (TwEventMouseButtonGLFW(button, action)) return;
	if (action == GLFW_PRESS && !activateCamera) {
		GLint index;
		glReadPixels(xGlobal, height - yGlobal, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);
		bool ISelected = false;
		if (index > 0 && index < 21) { //Validando que no se seleccione el piso ni el stencil del suelo

			ISelected = true;
			//Se absorben las propiedades y se cambia el menú de AntTweakBar
			selectedModel = index;
			TwDefine("Menú visible=false");
			TwDefine("Figura visible=true");
			selecting = true;
			rotacionPrincipal[0] = models[selectedModel].rotacion[0];
			rotacionPrincipal[1] = models[selectedModel].rotacion[1];
			rotacionPrincipal[2] = models[selectedModel].rotacion[2];
			rotacionPrincipal[3] = models[selectedModel].rotacion[3];
			scaleT = models[selectedModel].scaleT;
			bias = models[selectedModel].bias;

			toggleTexture = models[selectedModel].texturing;
			toggleNormalTexture = models[selectedModel].texturingNormal;
			toggleLightTexture = models[selectedModel].texturingLight;
			toggleShadows = models[selectedModel].shadowing;

			shinyBlinn = models[selectedModel].shinyBlinn;

			ejeX = models[selectedModel].ejeX;
			ejeY = models[selectedModel].ejeY;
			ejeZ = models[selectedModel].ejeZ;

			color_material_ambiental[0] = models[selectedModel].color_material_ambiental[0];
			color_material_ambiental[1] = models[selectedModel].color_material_ambiental[1];
			color_material_ambiental[2] = models[selectedModel].color_material_ambiental[2];
			color_material_ambiental[3] = models[selectedModel].color_material_ambiental[3];

			color_material_difuso[0] = models[selectedModel].color_material_difuso[0];
			color_material_difuso[1] = models[selectedModel].color_material_difuso[1];
			color_material_difuso[2] = models[selectedModel].color_material_difuso[2];
			color_material_difuso[3] = models[selectedModel].color_material_difuso[3];

			color_material_especular[0] = models[selectedModel].color_material_especular[0];
			color_material_especular[1] = models[selectedModel].color_material_especular[1];
			color_material_especular[2] = models[selectedModel].color_material_especular[2];
			color_material_especular[3] = models[selectedModel].color_material_especular[3];

		}
		
		if (!ISelected) {
			selecting = false;
			TwDefine("Menú visible=true");
			TwDefine("Figura visible=false");
		}
	}
}

void Interface::cursorPos(GLFWwindow* window, double x, double y) {
	xGlobal = x;
	yGlobal = y;
	if (TwEventMousePosGLFW(x, y)) {
		lastX = x;
		lastY = y;
		return;
	}
	if (firstMouse) {
		lastX = x;
		lastY = y;
		firstMouse = false;
	}
	GLfloat xoffset = x - lastX;
	GLfloat yoffset = lastY - y;
	lastX = x;
	lastY = y;
	if (activateCamera) camera.ProcessMouseMovement(xoffset, yoffset);
}

void Interface::TwEventMousePosGLFW3(GLFWwindow* window, double x, double y) {
	TwMouseMotion(int(x), int(y));
}

void Interface::cursorScroll(GLFWwindow* window, double xoffset, double yoffset) {
	if (TwEventMouseWheelGLFW(yoffset)) return;
	//Para escalar el objeto principal cuando es seleccionado
	if (yoffset == 1 && selecting) {
		scaleT += 0.01;
	}
	if (yoffset == -1 && selecting) {
		scaleT -= 0.01;
	}
}

void Interface::TwEventKeyGLFW3(GLFWwindow* window, int key, int scancode, int action, int mods) {
	TwEventKeyGLFW(key, action);
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)	glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS) keys[key] = true;
		else if (action == GLFW_RELEASE) keys[key] = false;

		if (key == GLFW_KEY_T && (action == GLFW_PRESS)) {
			activateCamera = !activateCamera;			

			if (activateCamera) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				if (selecting) {
					TwDefine("Figura visible=false");
				}
				else {
					TwDefine("Menú visible=false");
				}
				toggleDepth = false;
			}
			else {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				if (selecting) {
					TwDefine("Figura visible=true");
				}
				else {
					TwDefine("Menú visible=true");
				}
				toggleDepth = lastToggleDepth;
			}
		}
	}
}

void Interface::TwEventCharGLFW3(GLFWwindow* window, int codepoint) {
	TwEventCharGLFW(codepoint, GLFW_PRESS);
}

void Interface::TwWindowSizeGLFW3(GLFWwindow* window, int width, int height) {
	TwWindowSize(width, height);
}


//Función de salir
void TW_CALL exit(void *clientData) {
	exit(1);
}

//Función de activar el AutoRotate
void TW_CALL SetAutoRotateCB(const void *value, void *clientData) {
	autoRotar = *(const int *)value;
	if (autoRotar != 0) {
		tiempoRotacion = m.getTimeMs();
		rotacionInicial[0] = rotacionPrincipal[0];
		rotacionInicial[1] = rotacionPrincipal[1];
		rotacionInicial[2] = rotacionPrincipal[2];
		rotacionInicial[3] = rotacionPrincipal[3];
	}
}

//Función de AutoRotate
void TW_CALL GetAutoRotateCB(void *value, void *clientData) {
	(void)clientData;
	*(int *)value = autoRotar;
}

//Activar o desactivar texturas
void TW_CALL setTexture(const void *value, void *clientData) {
	toggleTexture = *(const int *)value;
}

void TW_CALL getTexture(void *value, void *clientData) {
	(void)clientData;
	*(int *)value = toggleTexture;
}

//Activar o desactivar texturas normales
void TW_CALL setTextureNormal(const void *value, void *clientData) {
	toggleNormalTexture = *(const int *)value;
}

void TW_CALL getTextureNormal(void *value, void *clientData) {
	(void)clientData;
	*(int *)value = toggleNormalTexture;
}

//Activar o desactivar texturas de luz
void TW_CALL setTextureLight(const void *value, void *clientData) {
	toggleLightTexture = *(const int *)value;
}

void TW_CALL getTextureLight(void *value, void *clientData) {
	(void)clientData;
	*(int *)value = toggleLightTexture;
}

//Activar sombras
void TW_CALL setShadows(const void *value, void *clientData) {
	toggleShadows = *(const int *)value;
}

void TW_CALL getShadows(void *value, void *clientData) {
	(void)clientData;
	*(int *)value = toggleShadows;
}

//Activar o desactivar el depth map
void TW_CALL setDepth(const void *value, void *clientData) {
	toggleDepth = *(const int *)value;
	lastToggleDepth = toggleDepth;
}

void TW_CALL getDepth(void *value, void *clientData) {
	(void)clientData;
	*(int *)value = toggleDepth;
}

//Activar o desactivar la luz
void TW_CALL setLight(const void *value, void *clientData) {
	toggleLight = *(const int *)value;
}

void TW_CALL getLight(void *value, void *clientData) {
	(void)clientData;
	*(int *)value = toggleLight;
}

void Interface::initAntTweakBar() {

	menuTW = TwNewBar("Menú");
	TwDefine("Menú size='270 80' position='20 20' color='128 0 0' label='F.C.G. Tarea #2 - Jose M. Alvarez'");
	TwAddButton(menuTW, "exit", exit, NULL, " label='Salir' key=Esc");

	modelTW = TwNewBar("Figura");
	TwWindowSize(200, 400);
	TwDefine("Figura visible=false size='270 520' position='20 20' color='128 0 0' label='Objeto'");

	TwAddVarRW(modelTW, "BrilloBlinn", TW_TYPE_FLOAT, &shinyBlinn, "min=1.0 max=400.0 step=1.0 label='Brillo' group='Blinn-Phong'");

	TwAddVarCB(modelTW, "toggleTextures", TW_TYPE_BOOL32, setTexture, getTexture, NULL, " label='Textura'");
	TwAddVarCB(modelTW, "toggleNormalTextures", TW_TYPE_BOOL32, setTextureNormal, getTextureNormal, NULL, " label='Textura Normal'");
	TwAddVarCB(modelTW, "toggleLightTextures", TW_TYPE_BOOL32, setTextureLight, getTextureLight, NULL, " label='Textura de Luz'");
	TwAddVarCB(modelTW, "toggleShadows", TW_TYPE_BOOL32, setShadows, getShadows, NULL, " label='Activar' group='Sombras'");
	TwAddVarRW(modelTW, "bias", TW_TYPE_FLOAT, &bias, "min=0.001 step=0.001 max=0.5 label='Bias' group='Sombras'");

	TwAddVarRW(modelTW, "refractor", TW_TYPE_FLOAT, &refractor, "min=1.0 max=5.0 step=0.01 label='Escalar' label='Factor' group='Refracción'");
	TwAddVarRW(modelTW, "scale", TW_TYPE_FLOAT, &scaleT, "min=0.01 step=0.01 label='Escalar' group='Transformaciones'");
	TwAddVarRW(modelTW, "ejeX", TW_TYPE_FLOAT, &ejeX, "step=0.01 label='Traslación x' group='Transformaciones'");
	TwAddVarRW(modelTW, "ejeY", TW_TYPE_FLOAT, &ejeY, "step=0.01 label='Traslación y' group='Transformaciones'");
	TwAddVarRW(modelTW, "ejeZ", TW_TYPE_FLOAT, &ejeZ, "step=0.01 label='Traslación z' group='Transformaciones'");
	TwAddVarRW(modelTW, "rotation", TW_TYPE_QUAT4F, &rotacionPrincipal, " label='Rotación' opened=true group='Transformaciones'");

	TwAddVarRW(modelTW, "Color AmbientalM", TW_TYPE_COLOR3F, &color_material_ambiental, "label='Color Ambiental' group='Material'");
	TwAddVarRW(modelTW, "Color DifusoM", TW_TYPE_COLOR3F, &color_material_difuso, "label='Color Difuso' group='Material'");
	TwAddVarRW(modelTW, "Color EspecularM", TW_TYPE_COLOR3F, &color_material_especular, "label='Color Especular' group='Material'");

	TwAddVarCB(modelTW, "toggleLight", TW_TYPE_BOOL32, setLight, getLight, NULL, " label='Prender/Apagar' group='Luz'");
	TwAddVarCB(modelTW, "toggleDepthMap", TW_TYPE_BOOL32, setDepth, getDepth, NULL, " label='Depth Map' group='Luz'");
	{
		TwEnumVal styleLight[3] = { { Directional, "Direccional" },{ Point, "Puntual" },{ Spot, "Reflector" } };
		TwType light = TwDefineEnum("LightType", styleLight, 3);
		TwAddVarRW(modelTW, "Tipo de luz", light, &currentLight, "group='Luz'");
	}

	TwAddVarRW(modelTW, "constantP", TW_TYPE_FLOAT, &constant, "min=0.001 max=1.0 step=0.001 label='Constante' group='Puntual'");
	TwAddVarRW(modelTW, "linearP", TW_TYPE_FLOAT, &linear, "min=0.001 max=1.0 step=0.001 label='Lineal' group='Puntual'");
	TwAddVarRW(modelTW, "quadraticP", TW_TYPE_FLOAT, &quadratic, "min=0.001 max=1.8 step=0.001 label='Cuadrática' group='Puntual'");

	TwAddVarRW(modelTW, "inCos", TW_TYPE_FLOAT, &inCos, "min=0.001 max=1.0 step=0.001 label='Coseno interno' group='Reflector'");
	TwAddVarRW(modelTW, "outCos", TW_TYPE_FLOAT, &outCos, "min=0.750 max=1.0 step=0.001 label='Coseno externo' group='Reflector'");
	TwAddVarRW(modelTW, "constantR", TW_TYPE_FLOAT, &constant, "min=0.001 max=1.0 step=0.001 label='Constante' group='Reflector'");
	TwAddVarRW(modelTW, "linearR", TW_TYPE_FLOAT, &linear, "min=0.001 max=1.0 step=0.001 label='Lineal' group='Reflector'");
	TwAddVarRW(modelTW, "quadraticR", TW_TYPE_FLOAT, &quadratic, "min=0.001 max=1.8 step=0.001 label='Cuadrática' group='Reflector'");
	TwAddVarRW(modelTW, "LightDir", TW_TYPE_DIR3F, &lightDirection, " label='Dirección' opened=true group='Reflector'");

	TwAddVarRW(modelTW, "ejeXL", TW_TYPE_FLOAT, &ejeXL, "step=0.01 label='x' min=-1.79 max=1.75 group='Trasladar luz' group='Luz'");
	TwAddVarRW(modelTW, "ejeYL", TW_TYPE_FLOAT, &ejeYL, "step=0.01 label='y' group='Trasladar luz' group='Luz'");
	TwAddVarRW(modelTW, "ejeZL", TW_TYPE_FLOAT, &ejeZL, "step=0.01 label='z' min=-1.82 max=1.82 group='Trasladar luz' group='Luz'");

	TwAddVarRW(modelTW, "Color AmbientalL", TW_TYPE_COLOR3F, &color_luz_ambiental, "label='Color Ambiental' group='Luz'");
	TwAddVarRW(modelTW, "Color DifusoL", TW_TYPE_COLOR3F, &color_luz_difuso, "label='Color Difuso' group='Luz'");
	TwAddVarRW(modelTW, "Color EspecularL", TW_TYPE_COLOR3F, &color_luz_especular, "label='Color Especular' group='Luz'");
	TwAddButton(modelTW, "exitF", exit, NULL, " label='Salir' key=Esc");

}