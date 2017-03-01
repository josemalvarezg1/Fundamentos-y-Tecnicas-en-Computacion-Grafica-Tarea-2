#include "Shadow.h"

extern vector<model> models; //Todos los modelos irán en este vector
extern model m;
extern TwBar *menuTW, *modelTW;
extern Camera camera;
extern int width, height, selectedModel, autoRotar, tiempoRotacion;
extern bool activateCamera, toggleTexture, toggleNormalTexture, toggleLightTexture, toggleLight, toggleShadows, toggleDepth, lastToggleDepth, firstMouse, selecting;
extern double xGlobal, yGlobal;
extern bool keys[1024]; //Todas las teclas				
extern float rotacionInicial[4], rotacionPrincipal[4], lightDirection[4], color_material_ambiental[4], color_material_difuso[4], color_material_especular[4], color_luz_ambiental[4], color_luz_difuso[4], color_luz_especular[4];
extern float shinyBlinn, scaleT, ejeX, ejeY, ejeZ, ejeXL, ejeYL, ejeZL, constant, linear, quadratic, inCos, outCos, refractor, bias, cosPlus, beforeOuter;
extern GLfloat lastX, lastY;
extern lightning currentLight;
extern glm::mat4 project_mat; //Matriz de Proyección
extern glm::mat4 view_mat; //Matriz de View
extern CGLSLProgram glslProgram; //Programa de shaders
extern CGLSLProgram glslSkyboxProgram; //Programa de shaders del skybox
extern CGLSLProgram glslDepthProgram; //Programa de shaders del depth map
extern CGLSLProgram glslShadowProgram; //Programa de shaders de las sombras
extern GLuint texture1, texture2, texture2_light, texture3, texture4, texture4_normal, texture5, texture5_normal, texture6, texture7, texture8, texture9, texture10, texture10_normal, texture11, texture12, texture13, texture14, texture15, texture16, cubemapTexture, depthMap;
extern GLuint depthMap, depthMapFBO, quadVAO, quadVBO;
extern glm::vec3 eye;

void initShadow() {

	glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

//Funcion que actualiza el depthMap (desde el punto de vista de la luz)
void drawShadows(bool flipped) {

	model m;
	glm::mat4 projectionLightMat, viewLightMat;
	glm::mat4 spaceLightMatrix;
	GLfloat near_plane = 0.0f, far_plane = 5.0f;
	projectionLightMat = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, near_plane, far_plane);

	if (currentLight == Directional) {
		projectionLightMat = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, near_plane, far_plane);
		if (!flipped) viewLightMat = glm::lookAt(glm::vec3(ejeXL, ejeYL, ejeZL), glm::vec3(0.0, -3.20, 0.0), glm::vec3(0.0, 1.0, 0.0));
		else viewLightMat = glm::lookAt(glm::vec3(ejeXL, ejeYL, -ejeZL - 4.0), glm::vec3(0.0, -3.20, -4.0), glm::vec3(0.0, 1.0, 0.0));
	}
	else if (currentLight == Spot) {

		float fov = outCos >= 1.0 ? 0.0 : 2.50 - (outCos*cosPlus);
		projectionLightMat = glm::perspective(fov, (GLfloat)1024 / (GLfloat)1024, 1.0f, 1000.0f);

		if (!flipped) {
			glm::vec3 lightInvDir = glm::vec3(lightDirection[0], lightDirection[1], lightDirection[2]);
			viewLightMat = glm::lookAt(glm::vec3(ejeXL, ejeYL, ejeZL), glm::vec3(ejeXL, ejeYL, ejeZL) + lightInvDir, glm::vec3(0, 1, 0));
		}
		else {
			glm::vec3 lightInvDir = glm::vec3(lightDirection[0], lightDirection[1], -lightDirection[2]);
			viewLightMat = glm::lookAt(glm::vec3(ejeXL, ejeYL, -ejeZL - 4.0), glm::vec3(ejeXL, ejeYL, -ejeZL - 4.0) + lightInvDir, glm::vec3(0, 1, 0));
		}
	}

	spaceLightMatrix = projectionLightMat * viewLightMat;

	glslShadowProgram.enable();
	glUniformMatrix4fv(glslShadowProgram.getLocation("light_matrix"), 1, GL_FALSE, glm::value_ptr(spaceLightMatrix));
	glViewport(0, 0, 1024, 1024);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	for (int i = 0; i < models.size() - 1; i++) { //Para los modelos quitando el espejo y la luz

		if (i != 2 && !(i >= 9 && i <= 12)) {

			GLuint model_matr_loc = glslShadowProgram.getLocation("model");

			//Matrices de view y proyeccion
			glm::mat4 model_mat = glm::mat4();

			if (flipped) {

				model_mat = m.translate_en_matriz(models[i].ejeX, models[i].ejeY, -models[i].ejeZ - 4);

				model_mat = model_mat * m.rotacion_en_matriz(-models[i].rotacion[0], -models[i].rotacion[1], models[i].rotacion[2], models[i].rotacion[3]);

				model_mat = model_mat * m.scale_en_matriz(models[i].scaleT);
				model_mat = glm::scale(model_mat, glm::vec3(1, 1, -1));

			}
			else {

				model_mat = m.translate_en_matriz(models[i].ejeX, models[i].ejeY, models[i].ejeZ);
				model_mat = model_mat * m.rotacion_en_matriz(models[i].rotacion[0], models[i].rotacion[1], models[i].rotacion[2], models[i].rotacion[3]);
				model_mat = model_mat * m.scale_en_matriz(models[i].scaleT);

			}
			glUniformMatrix4fv(model_matr_loc, 1, GL_FALSE, glm::value_ptr(model_mat));
			glBindBuffer(GL_ARRAY_BUFFER, models[i].vbo);

			//VERTICE Y NORMALES
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[i].vertices.size() * sizeof(float)));
			glEnableVertexAttribArray(1);

			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[i].vertices.size() * sizeof(float) + (models[i].coord_texturas.size() * sizeof(float))));
			glEnableVertexAttribArray(2);

			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[i].vertices.size() * sizeof(float) + (models[i].coord_texturas.size() * sizeof(float) + models[i].normales_vertice_fin.size() * sizeof(float))));
			glEnableVertexAttribArray(3);

			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glDrawArrays(GL_TRIANGLES, 0, models[i].vertices.size() / 3);

			glDisableClientState(GL_VERTEX_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
			glDisableVertexAttribArray(3);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		}

	}
	glslShadowProgram.disable();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void drawDepth() {

	if (quadVAO == 0) {

		GLfloat quadVertices[] = {
			-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
			1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
			1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
		};

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

}


void draw_depth_map() {

	glslDepthProgram.enable();

	glm::mat4 modelo;
	model m;
	modelo = m.translate_en_matriz(0.7, -0.7, -1.0);
	modelo = modelo * m.scale_en_matriz(0.3f);

	glUniformMatrix4fv(glslDepthProgram.getLocation("model"), 1, GL_FALSE, glm::value_ptr(modelo));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	drawDepth();
	glslDepthProgram.disable();

}