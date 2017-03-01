#include "Main.h"
#include "Model.h"
#include "Interface.h"
#include "Shadow.h"
#include "Render.h"
using namespace std;

int width = 1024, height = 768, selectedModel = 0, imageWidth, imageHeight, imageFormat, id, autoRotar = 0, tiempoRotacion = 0;
GLuint texture, frame = 0, skyboxVAO, skyboxVBO;;
vector<const GLchar*> faces;
bool activateCamera = false, toggleTexture = true, toggleNormalTexture = true, toggleLightTexture = true, toggleLight = true, toggleShadows = true, toggleDepth = true, lastToggleDepth = true, firstMouse = true, selecting = false;
double xGlobal, yGlobal;
bool keys[1024]; //Todas las teclas				
Camera camera(glm::vec3(0.0f, 0.55f, 1.1f));
GLfloat lastX = 400, lastY = 300, deltaTime = 0.0f, lastFrame = 0.0f, currenttime = 0.0f, timebase = 0.0f;
model m;
Interface i;
TwBar *menuTW;
TwBar *modelTW;

//Variables globales a usar en AntTweakBar
float color_material_ambiental[] = { 0.0f, 0.0f, 0.0f, 0.0 };
float color_material_difuso[] = { 1.0f, 1.0f, 0.0f, 0.0 };
float color_material_especular[] = { 1.0f, 1.0f, 1.0f, 0.0 };
float color_luz_ambiental[] = { 0.0f, 0.0f, 0.0f, 0.0 };
float color_luz_difuso[] = { 1.0f, 1.0f, 1.0f, 0.0 };
float color_luz_especular[] = { 1.0f, 1.0f, 1.0f, 0.0 };
float shinyBlinn = 128.0, scaleT = 1.00, ejeX = 0.0, ejeY = 0.0, ejeZ = 0.0, ejeXL = 0.23, ejeYL = 1.18, ejeZL = 0.00, constant = 1.0, linear = 0.09, quadratic = 0.032, inCos = 0.91, outCos = 0.750, refractor = 1.03, bias = 0.05, cosPlus = 1.0, prevOut = 0.750;

lightning currentLight = Point;
float rotacion[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float rotacionPrincipal[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float rotacionInicial[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDirection[] = { -1.31, -0.12, 1.10 };

CGLSLProgram glslProgram; //Programa de shaders
CGLSLProgram glslSkyboxProgram; //Programa de shaders del skybox
CGLSLProgram glslDepthProgram;	//Programa de shaders del depth map
CGLSLProgram glslShadowProgram;	//Programa de shaders de las sombras

GLuint depthMap, depthMapFBO, quadVAO = 0, quadVBO;

glm::mat4 project_mat; //Matriz de Proyección
glm::mat4 view_mat; //Matriz de View
glm::vec3 eye(0.0f, 0.0f, 2.0f); // Ojo

vector<model> models; //Todos los modelos irán en este vector

GLuint texture1, texture2, texture2_light, texture3, texture4, texture4_normal, texture5, texture5_normal, texture6, texture7, texture8, texture9, texture10, texture10_normal, texture11, texture12, texture13, texture14, texture15, texture16, cubemapTexture;

//Cubo de Skybox
GLfloat skyboxVertices[] = {
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f
};

//Para separar un string
vector<string> split(const string &s, char delim) {
	stringstream ss(s);
	string item;
	vector<string> tokens;
	while (getline(ss, item, delim)) {
		tokens.push_back(item);
	}
	return tokens;
}

void loadTextures() {
	const char* file = "Texturas/floor.jpg";
	texture1 = m.LoadTexture(file, true);

	file = "Texturas/master.jpg";
	texture2 = m.LoadTexture(file, true);

	file = "Texturas/lightmap.png";
	texture2_light = m.LoadTexture(file, true);

	file = "Texturas/mickey.jpg";
	texture3 = m.LoadTexture(file, true);

	file = "Texturas/flash.jpg";
	texture4 = m.LoadTexture(file, true);

	file = "Texturas/flash_normal.tga";
	texture4_normal = m.LoadTexture(file, true);

	file = "Texturas/arrow.jpg";
	texture5 = m.LoadTexture(file, true);

	file = "Texturas/arrow_normal.tga";
	texture5_normal = m.LoadTexture(file, true);

	file = "Texturas/joker.jpg";
	texture6 = m.LoadTexture(file, true);

	file = "Texturas/spiderman.jpg";
	texture7 = m.LoadTexture(file, true);

	file = "Texturas/silver.jpg";
	texture8 = m.LoadTexture(file, true);

	file = "Texturas/ironman.jpg";
	texture9 = m.LoadTexture(file, true);

	file = "Texturas/brick.jpg";
	texture10 = m.LoadTexture(file, true);

	file = "Texturas/brick_normal.jpg";
	texture10_normal = m.LoadTexture(file, true);

	file = "Texturas/mona.jpg";
	texture11 = m.LoadTexture(file, true);

	file = "Texturas/mario.jpg";
	texture12 = m.LoadTexture(file, true);

	file = "Texturas/luigi.jpg";
	texture13 = m.LoadTexture(file, true);

	file = "Texturas/car.jpg";
	texture14 = m.LoadTexture(file, true);

	file = "Texturas/plant.jpg";
	texture15 = m.LoadTexture(file, true);

	file = "Texturas/transparent.jpg";
	texture16 = m.LoadTexture(file, true);
}

GLuint loadCubemap(vector<const GLchar*> faces) {
	ilInit();
	ILuint imageID;
	GLuint textureID;
	glGenTextures(1, &textureID);
	ILboolean success;
	ILenum error;
	ilGenImages(1, &imageID);
	ilBindImage(imageID);
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

	for (GLuint i = 0; i < faces.size(); i++) {
		success = ilLoadImage(faces[i]);
		if (success)
		{
			success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
			if (!success) {
				error = ilGetError();
				cout << "Image conversion fails" << endl;
			}

			glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				ilGetInteger(IL_IMAGE_FORMAT),
				ilGetInteger(IL_IMAGE_WIDTH),
				ilGetInteger(IL_IMAGE_HEIGHT),
				0,
				ilGetInteger(IL_IMAGE_FORMAT),
				GL_UNSIGNED_BYTE,
				ilGetData()
			);
		}
	}

	ilDeleteImages(1, &imageID);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;

}

//Inicializa los valores de skybox
void initSkybox() {
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);

	faces.push_back("Texturas/left.jpg");
	faces.push_back("Texturas/right.jpg");
	faces.push_back("Texturas/top.jpg");
	faces.push_back("Texturas/bottom.jpg");
	faces.push_back("Texturas/back.jpg");
	faces.push_back("Texturas/front.jpg");
	cubemapTexture = loadCubemap(faces);
}

void reshape(GLFWwindow* window, int w, int h) {

	w = max(w, 1);
	h = max(h, 1);
	width = w;
	height = h;	
	glViewport(0, 0, w, h);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	project_mat = glm::perspective(45.0f, (float)w / (float)h, 0.1f, 1000.0f);
	gluPerspective(45.0f, (float)w / (float)h, 0.1f, 1000.0f);

	glm::vec3 eye(0.0f, 0.0f, 2.0f);
	glm::vec3 norm(0.0f, 0.0f, 0.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	view_mat = glm::lookAt(eye, norm, up);
	gluLookAt(eye[0], eye[1], eye[2], norm[0], norm[1], norm[2], up[0], up[1], up[2]);

	glMatrixMode(GL_MODELVIEW);
	glClearColor(1.0, 0.0, 0.0, 0);
	glColor3f(1.0, 1.0, 1.0);
	TwWindowSize(w, h);

}

//Funcion que mueve la cámara con WASD y la deshabilita con T
void moverme() {

	if (keys[GLFW_KEY_W]) camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S]) camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A]) camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D]) camera.ProcessKeyboard(RIGHT, deltaTime);

}

void drawSkybox() {

	glslSkyboxProgram.enable();
	glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
	glm::mat4 projection = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glslSkyboxProgram.getLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glslSkyboxProgram.getLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniform1i(glslSkyboxProgram.getLocation("refracting"), false);
	// Cubo del Skybox
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glslSkyboxProgram.getLocation("skybox"), 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);
	glslSkyboxProgram.disable();

}

int main(int argc, char* argv[]) {

	if (!glfwInit()) {
		std::cerr << "Error inicializando glfw...\n";
		return 0;
	}

	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(1024, 768, "F.C.G. Tarea #2 - Jose Manuel Alvarez - CI 25038805", nullptr, nullptr);
	if (!window) {
		std::cerr << "Error creando ventana...\n";
		glfwTerminate();
		return 0;
	}

	glfwSetFramebufferSizeCallback(window, reshape);
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	TwInit(TW_OPENGL, NULL);
	m.initGlew();
	i.initAntTweakBar();
	glfwSetWindowSizeCallback(window, i.TwWindowSizeGLFW3);
	glfwSetMouseButtonCallback(window, (GLFWmousebuttonfun)i.TwEventMouseButtonGLFW3);
	glfwSetCursorPosCallback(window, i.cursorPos);
	glfwSetScrollCallback(window, i.cursorScroll);
	glfwSetKeyCallback(window, (GLFWkeyfun)i.TwEventKeyGLFW3);
	glfwSetCharCallback(window, (GLFWcharfun)i.TwEventCharGLFW3);
	reshape(window, 1024, 768);
	glewInit();
	initSkybox();
	//Todos los objetos
	m.read_obj("Modelos/obj/floor.obj");
	m.read_obj("Modelos/obj/cube.obj");
	m.read_obj("Modelos/obj/brawler.obj");
	m.read_obj("Modelos/obj/flash.obj");
	m.read_obj("Modelos/obj/arrow.obj");
	m.read_obj("Modelos/obj/joker.obj");
	m.read_obj("Modelos/obj/spiderman.obj");
	m.read_obj("Modelos/obj/silver.obj");
	m.read_obj("Modelos/obj/ironman.obj");
	m.read_obj("Modelos/obj/wall.obj");
	m.read_obj("Modelos/obj/wall.obj");
	m.read_obj("Modelos/obj/wall.obj");
	m.read_obj("Modelos/obj/wall.obj");
	m.read_obj("Modelos/obj/floor.obj");
	m.read_obj("Modelos/obj/mario.obj");
	m.read_obj("Modelos/obj/luigi.obj");
	m.read_obj("Modelos/obj/car.obj");
	m.read_obj("Modelos/obj/plant.obj");
	m.read_obj("Modelos/obj/plant.obj");
	m.read_obj("Modelos/obj/plant.obj");
	m.read_obj("Modelos/obj/sphere.obj");
	loadTextures();
	initShadow();

	while (!glfwWindowShouldClose(window)) {
		//Este es el Main loop
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		currenttime = currentFrame;
		lastFrame = currentFrame;

		if (prevOut != outCos) {
			cosPlus += outCos - prevOut;
			prevOut = outCos;
		}

		//Dibujo el skybox
		drawSkybox();
		if (currentLight == Directional || currentLight == Spot) drawShadows(true);
		glViewport(0, 0, width, height);
		glClear(GL_DEPTH_BUFFER_BIT);		
		glLoadIdentity();
		glDepthMask(GL_FALSE);		
		glClear(GL_STENCIL_BUFFER_BIT);

		//Dibujo la escena
		glslProgram.disable();

		draw(false); //Modelos originales sin sombra puntual

		//Sólo Point
		if (currentLight == Point) {

			glDisable(GL_STENCIL_TEST);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(-1.0, -3.0);
			draw(true); //Sombra puntual
			glDisable(GL_POLYGON_OFFSET_FILL);

		}		

		if ((toggleDepth) && (currentLight == Directional || currentLight == Spot)) draw_depth_map();
		TwDraw();
		moverme();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	TwTerminate();
	glfwTerminate();
	return 0;
}