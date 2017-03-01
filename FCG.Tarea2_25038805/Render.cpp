#include "Render.h"

extern vector<model> models; //Todos los modelos irán en este vector
extern model m;
extern TwBar *menuTW, *modelTW;
extern Camera camera;
extern int width, height, selectedModel, autoRotar, tiempoRotacion;
extern bool activateCamera, toggleTexture, toggleNormalTexture, toggleLightTexture, toggleLight, toggleShadows, toggleDepth, lastToggleDepth, firstMouse, selecting;
extern double xGlobal, yGlobal;
extern bool keys[1024]; //Todas las teclas				
extern float rotacionInicial[4], rotacionPrincipal[4], lightDirection[4], color_material_ambiental[4], color_material_difuso[4], color_material_especular[4], color_luz_ambiental[4], color_luz_difuso[4], color_luz_especular[4];
extern float shinyBlinn, scaleT, ejeX, ejeY, ejeZ, ejeXL, ejeYL, ejeZL, constant, linear, quadratic, inCos, outCos, refractor, bias, cosPlus, prevOut;
extern GLfloat lastX, lastY;
extern lightning currentLight;
extern glm::mat4 project_mat; //Matriz de Proyección
extern glm::mat4 view_mat; //Matriz de View
extern CGLSLProgram glslProgram; //Programa de shaders
extern CGLSLProgram glslSkyboxProgram; //Programa de shaders del skybox
extern CGLSLProgram glslDepthProgram; //Programa de shaders del depth map
extern CGLSLProgram glslShadowProgram; //Programa de shaders de las sombras
extern GLuint texture1, texture2, texture2_light, texture3, texture4, texture4_normal, texture5, texture5_normal, texture6, texture7, texture8, texture9, texture10, texture10_normal, texture11, texture12, texture13, texture14, texture15, texture16, cubemapTexture, depthMap;
extern glm::vec3 eye;

//Función que dibuja el modelo VBO con texturas
void draw(bool planar) {

	//Validaciones en AntTweakBar

	if (inCos < outCos) inCos = outCos;

	if (currentLight == Point) {
		TwDefine("Figura/Puntual visible=true");
		TwDefine("Figura/Sombras visible=false");
	}
	else {
		TwDefine("Figura/Puntual visible=false");
		TwDefine("Figura/Sombras visible=true");
	}

	if (currentLight == Spot) TwDefine("Figura/Reflector visible=true");
	else TwDefine("Figura/Reflector visible=false");

	if (selectedModel >= models.size()) selectedModel = models.size() - 1;

	models[20].ejeX = ejeXL;
	models[20].ejeY = ejeYL;
	models[20].ejeZ = ejeZL;

	if (!planar) {

		glEnable(GL_STENCIL_TEST);

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
		glDepthMask(GL_FALSE);

		glClear(GL_STENCIL_BUFFER_BIT);

	}

	//Dibujar el espejo
	if (!planar) {

		glslProgram.enable();
		glUniform1i(glslProgram.getLocation("texture"), 0);
		glUniform1i(glslProgram.getLocation("textureNormal"), 1);
		glUniform1i(glslProgram.getLocation("skybox"), 2);
		glUniform1i(glslProgram.getLocation("shadowMap"), 3);
		glUniform1i(glslProgram.getLocation("textureLight"), 4);
		GLuint view_matr_loc = glslProgram.getLocation("view_matrix");
		GLuint model_matr_loc = glslProgram.getLocation("model_matrix");
		GLuint reflect_matr_loc = glslProgram.getLocation("reflection_matrix");
		GLuint proj_matr_loc = glslProgram.getLocation("projection_matrix");
		GLuint light_matr_loc = glslProgram.getLocation("light_matrix");
		GLuint light_loc = glslProgram.getLocation("lightPos");
		GLuint view_loc = glslProgram.getLocation("view");
		GLuint shinyBlinn_loc = glslProgram.getLocation("shinyBlinn");
		GLuint model = glslProgram.getLocation("modelNumber");
		GLuint interpol = glslProgram.getLocation("interpol");
		GLuint tipo_difuso = glslProgram.getLocation("tipo_difuso");
		GLuint tipo_especular = glslProgram.getLocation("tipo_especular");
		GLuint lightType_loc = glslProgram.getLocation("lightType");
		GLuint lightDir_loc = glslProgram.getLocation("lightSpotDir");
		GLuint texturing_loc = glslProgram.getLocation("texturing");
		GLuint texturing_normal_loc = glslProgram.getLocation("texturingNormal");
		GLuint lightningLoc = glslProgram.getLocation("lightning");
		GLuint bb_loc = glslProgram.getLocation("bb");
		GLuint constantLoc = glslProgram.getLocation("constant");
		GLuint linearLoc = glslProgram.getLocation("linear");
		GLuint quadraticLoc = glslProgram.getLocation("quadratic");
		GLuint inCosLoc = glslProgram.getLocation("inCos");
		GLuint outCosLoc = glslProgram.getLocation("outCos");
		GLuint refractor_loc = glslProgram.getLocation("refractor");

		GLuint ambMat_loc = glslProgram.getLocation("color_material_ambiental");
		GLuint difMat_loc = glslProgram.getLocation("color_material_difuso");
		GLuint specMat_loc = glslProgram.getLocation("color_material_especular");

		GLuint ambLight_loc = glslProgram.getLocation("color_luz_ambiental");
		GLuint difLight_loc = glslProgram.getLocation("color_luz_difuso");
		GLuint specLight_loc = glslProgram.getLocation("color_luz_especular");

		glUniform3f(view_loc, camera.Position[0], camera.Position[1], camera.Position[2]);
		glUniform3f(lightDir_loc, lightDirection[0], lightDirection[1], lightDirection[2]);
		glUniform3f(light_loc, ejeXL, ejeYL, ejeZL);

		glUniform1f(shinyBlinn_loc, models[9].shinyBlinn);
		glUniform1i(lightType_loc, currentLight);
		glUniform1i(lightningLoc, toggleLight);
		glUniform1i(bb_loc, 0);

		glUniform1f(refractor_loc, refractor);

		glUniform4f(ambMat_loc, models[9].color_material_ambiental[0], models[9].color_material_ambiental[1], models[9].color_material_ambiental[2], models[9].color_material_ambiental[3]);
		glUniform4f(difMat_loc, models[9].color_material_difuso[0], models[9].color_material_difuso[1], models[9].color_material_difuso[2], models[9].color_material_difuso[3]);
		glUniform4f(specMat_loc, models[9].color_material_especular[0], models[9].color_material_especular[1], models[9].color_material_especular[2], models[9].color_material_especular[3]);

		glUniform4f(ambLight_loc, color_luz_ambiental[0], color_luz_ambiental[1], color_luz_ambiental[2], color_luz_ambiental[3]);
		glUniform4f(difLight_loc, color_luz_difuso[0], color_luz_difuso[1], color_luz_difuso[2], color_luz_difuso[3]);
		glUniform4f(specLight_loc, color_luz_especular[0], color_luz_especular[1], color_luz_especular[2], color_luz_especular[3]);

		glUniform1f(constantLoc, constant);
		glUniform1f(linearLoc, linear);
		glUniform1f(quadraticLoc, quadratic);
		glUniform1f(inCosLoc, inCos);
		glUniform1f(outCosLoc, outCos);
		glUniform1i(model, 9);
		glUniform1i(texturing_loc, models[9].texturing);
		glUniform1i(texturing_normal_loc, models[9].texturingNormal);
		//Matrices de view y projection
		glm::mat4 model_mat;
		glm::mat4 reflect_mat;
		glm::vec3 norm(0.0f, 0.0f, 0.0f);
		glm::vec3 up(0.0f, 1.0f, 0.0f);
		view_mat = camera.GetViewMatrix();
		gluLookAt(eye[0], eye[1], eye[2], norm[0], norm[1], norm[2], up[0], up[1], up[2]);

		model_mat = m.translate_en_matriz(models[9].ejeX, models[9].ejeY, models[9].ejeZ);
		model_mat = model_mat * m.rotacion_en_matriz(models[9].rotacion[0], models[9].rotacion[1], models[9].rotacion[2], models[9].rotacion[3]);

		model_mat = model_mat * m.scale_en_matriz(models[9].scaleT);

		glUniformMatrix4fv(model_matr_loc, 1, GL_FALSE, glm::value_ptr(model_mat));
		glUniformMatrix4fv(reflect_matr_loc, 1, GL_FALSE, glm::value_ptr(reflect_mat));
		glUniformMatrix4fv(view_matr_loc, 1, GL_FALSE, glm::value_ptr(view_mat));

		project_mat = glm::perspective(camera.Zoom, (float)width / (float)height, 0.1f, 1000.0f);
		glUniformMatrix4fv(proj_matr_loc, 1, GL_FALSE, glm::value_ptr(project_mat));
		glm::mat4 shadowMat = glm::mat4();
		glUniformMatrix4fv(glslProgram.getLocation("planar_matrix"), 1, GL_FALSE, glm::value_ptr(shadowMat));

		glslProgram.enable();

		glBindBuffer(GL_ARRAY_BUFFER, models[9].vbo);
		//Se bindean los vértices, normales, coordenadas de texturas y tangentes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[9].vertices.size() * sizeof(float)));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[9].vertices.size() * sizeof(float) + models[9].coord_texturas.size() * sizeof(float)));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[9].vertices.size() * sizeof(float) + models[9].coord_texturas.size() * sizeof(float) + models[9].normales_vertice_fin.size() * sizeof(float)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, models[9].vertices.size() / 3);
		glDisableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glslProgram.disable();

		glPushMatrix();
		glPopMatrix();
	}

	if (!planar) {

		glStencilFunc(GL_EQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDepthMask(GL_TRUE);

	}

	//Esto hará el stencil entre la sombra de los modelos y el piso reflejado
	if (planar) {

		glEnable(GL_STENCIL_TEST);

		glStencilFunc(GL_ALWAYS, 21, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	}

	//Dibujar el piso reflejado
	if (planar) {

		glslProgram.enable();
		glUniform1i(glslProgram.getLocation("texture"), 0);
		glUniform1i(glslProgram.getLocation("textureNormal"), 1);
		glUniform1i(glslProgram.getLocation("skybox"), 2);
		glUniform1i(glslProgram.getLocation("shadowMap"), 3);
		glUniform1i(glslProgram.getLocation("textureLight"), 4);
		GLuint view_matr_loc = glslProgram.getLocation("view_matrix");
		GLuint model_matr_loc = glslProgram.getLocation("model_matrix");
		GLuint reflect_matr_loc = glslProgram.getLocation("reflection_matrix");
		GLuint proj_matr_loc = glslProgram.getLocation("projection_matrix");
		GLuint light_matr_loc = glslProgram.getLocation("light_matrix");
		GLuint light_loc = glslProgram.getLocation("lightPos");
		GLuint view_loc = glslProgram.getLocation("view");
		GLuint shinyBlinn_loc = glslProgram.getLocation("shinyBlinn");
		GLuint model = glslProgram.getLocation("modelNumber");
		GLuint interpol = glslProgram.getLocation("interpol");
		GLuint tipo_difuso = glslProgram.getLocation("tipo_difuso");
		GLuint tipo_especular = glslProgram.getLocation("tipo_especular");
		GLuint lightType_loc = glslProgram.getLocation("lightType");
		GLuint lightDir_loc = glslProgram.getLocation("lightSpotDir");
		GLuint texturing_loc = glslProgram.getLocation("texturing");
		GLuint texturing_normal_loc = glslProgram.getLocation("texturingNormal");
		GLuint lightningLoc = glslProgram.getLocation("lightning");
		GLuint bb_loc = glslProgram.getLocation("bb");
		GLuint constantLoc = glslProgram.getLocation("constant");
		GLuint linearLoc = glslProgram.getLocation("linear");
		GLuint quadraticLoc = glslProgram.getLocation("quadratic");
		GLuint inCosLoc = glslProgram.getLocation("inCos");
		GLuint outCosLoc = glslProgram.getLocation("outCos");
		GLuint refractor_loc = glslProgram.getLocation("refractor");

		GLuint ambMat_loc = glslProgram.getLocation("color_material_ambiental");
		GLuint difMat_loc = glslProgram.getLocation("color_material_difuso");
		GLuint specMat_loc = glslProgram.getLocation("color_material_especular");

		GLuint ambLight_loc = glslProgram.getLocation("color_luz_ambiental");
		GLuint difLight_loc = glslProgram.getLocation("color_luz_difuso");
		GLuint specLight_loc = glslProgram.getLocation("color_luz_especular");

		glUniform3f(view_loc, camera.Position[0], camera.Position[1], camera.Position[2]);
		glUniform3f(lightDir_loc, lightDirection[0], lightDirection[1], lightDirection[2]);
		glUniform3f(light_loc, ejeXL, ejeYL, ejeZL);

		glUniform1f(shinyBlinn_loc, models[0].shinyBlinn);
		glUniform1i(lightType_loc, currentLight);
		glUniform1i(lightningLoc, toggleLight);
		glUniform1i(bb_loc, 0);

		glUniform1f(refractor_loc, refractor);

		glUniform4f(ambMat_loc, models[0].color_material_ambiental[0], models[0].color_material_ambiental[1], models[0].color_material_ambiental[2], models[0].color_material_ambiental[3]);
		glUniform4f(difMat_loc, models[0].color_material_difuso[0], models[0].color_material_difuso[1], models[0].color_material_difuso[2], models[0].color_material_difuso[3]);
		glUniform4f(specMat_loc, models[0].color_material_especular[0], models[0].color_material_especular[1], models[0].color_material_especular[2], models[0].color_material_especular[3]);

		glUniform4f(ambLight_loc, color_luz_ambiental[0], color_luz_ambiental[1], color_luz_ambiental[2], color_luz_ambiental[3]);
		glUniform4f(difLight_loc, color_luz_difuso[0], color_luz_difuso[1], color_luz_difuso[2], color_luz_difuso[3]);
		glUniform4f(specLight_loc, color_luz_especular[0], color_luz_especular[1], color_luz_especular[2], color_luz_especular[3]);

		glUniform1f(constantLoc, constant);
		glUniform1f(linearLoc, linear);
		glUniform1f(quadraticLoc, quadratic);
		glUniform1f(inCosLoc, inCos);
		glUniform1f(outCosLoc, outCos);
		glUniform1i(model, 0);
		glUniform1i(texturing_loc, models[0].texturing);
		glUniform1i(texturing_normal_loc, models[0].texturingNormal);
		//Matrices de view y projection
		glm::mat4 model_mat;
		glm::mat4 reflect_mat;
		glm::vec3 norm(0.0f, 0.0f, 0.0f);
		glm::vec3 up(0.0f, 1.0f, 0.0f);
		view_mat = camera.GetViewMatrix();
		gluLookAt(eye[0], eye[1], eye[2], norm[0], norm[1], norm[2], up[0], up[1], up[2]);

		model_mat = m.translate_en_matriz(models[0].ejeX, models[0].ejeY, -models[0].ejeZ-4);
		model_mat = model_mat * m.rotacion_en_matriz(models[0].rotacion[0], models[0].rotacion[1], models[0].rotacion[2], models[0].rotacion[3]);

		model_mat = model_mat * m.scale_en_matriz(models[0].scaleT);

		glUniformMatrix4fv(model_matr_loc, 1, GL_FALSE, glm::value_ptr(model_mat));
		glUniformMatrix4fv(reflect_matr_loc, 1, GL_FALSE, glm::value_ptr(reflect_mat));
		glUniformMatrix4fv(view_matr_loc, 1, GL_FALSE, glm::value_ptr(view_mat));

		project_mat = glm::perspective(camera.Zoom, (float)width / (float)height, 0.1f, 1000.0f);
		glUniformMatrix4fv(proj_matr_loc, 1, GL_FALSE, glm::value_ptr(project_mat));
		glm::mat4 shadowMat = glm::mat4();
		glUniformMatrix4fv(glslProgram.getLocation("planar_matrix"), 1, GL_FALSE, glm::value_ptr(shadowMat));

		glEnable(GL_TEXTURE_2D);

		if (models[0].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture1);
		}

		glslProgram.enable();

		glBindBuffer(GL_ARRAY_BUFFER, models[0].vbo);
		//Se bindean los vértices, normales, coordenadas de texturas y tangentes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[0].vertices.size() * sizeof(float)));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[0].vertices.size() * sizeof(float) + models[0].coord_texturas.size() * sizeof(float)));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[0].vertices.size() * sizeof(float) + models[0].coord_texturas.size() * sizeof(float) + models[0].normales_vertice_fin.size() * sizeof(float)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, models[0].vertices.size() / 3);
		glDisableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glslProgram.disable();

		glPushMatrix();
		glPopMatrix();

		glDisable(GL_STENCIL_TEST);
	}

	if (planar) {

		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, 21, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

	}

	//Modelos reflejados
	for (int i = 0; i < models.size(); i++) { //Para los modelos

		if (planar && i == 0) i = 1;
		if (planar && i == 2) i = 3;
		if (planar && i >= 9 && i <= 12) i = 13;
		if (planar && i == 20) break;
		glslProgram.enable();
		glUniform1i(glslProgram.getLocation("texture"), 0);
		glUniform1i(glslProgram.getLocation("textureNormal"), 1);
		glUniform1i(glslProgram.getLocation("skybox"), 2);
		glUniform1i(glslProgram.getLocation("shadowMap"), 3);
		glUniform1i(glslProgram.getLocation("textureLight"), 4);
		GLuint view_matr_loc = glslProgram.getLocation("view_matrix");
		GLuint model_matr_loc = glslProgram.getLocation("model_matrix");
		GLuint reflect_matr_loc = glslProgram.getLocation("reflection_matrix");
		GLuint proj_matr_loc = glslProgram.getLocation("projection_matrix");
		GLuint light_loc = glslProgram.getLocation("lightPos");
		GLuint view_loc = glslProgram.getLocation("view");
		GLuint shinyBlinn_loc = glslProgram.getLocation("shinyBlinn");
		GLuint model = glslProgram.getLocation("modelNumber");
		GLuint interpol = glslProgram.getLocation("interpol");
		GLuint tipo_difuso = glslProgram.getLocation("tipo_difuso");
		GLuint tipo_especular = glslProgram.getLocation("tipo_especular");
		GLuint shadowing_light_loc = glslProgram.getLocation("shadowingLightMap");
		GLuint lightType_loc = glslProgram.getLocation("lightType");
		GLuint lightDir_loc = glslProgram.getLocation("lightSpotDir");
		GLuint texturing_loc = glslProgram.getLocation("texturing");
		GLuint texturing_normal_loc = glslProgram.getLocation("texturingNormal");
		GLuint texturing_light_loc = glslProgram.getLocation("texturingLight");
		GLuint lightningLoc = glslProgram.getLocation("lightning");
		GLuint bb_loc = glslProgram.getLocation("bb");
		GLuint constantLoc = glslProgram.getLocation("constant");
		GLuint linearLoc = glslProgram.getLocation("linear");
		GLuint quadraticLoc = glslProgram.getLocation("quadratic");
		GLuint inCosLoc = glslProgram.getLocation("inCos");
		GLuint outCosLoc = glslProgram.getLocation("outCos");
		GLuint refractor_loc = glslProgram.getLocation("refractor");

		GLuint ambMat_loc = glslProgram.getLocation("color_material_ambiental");
		GLuint difMat_loc = glslProgram.getLocation("color_material_difuso");
		GLuint specMat_loc = glslProgram.getLocation("color_material_especular");

		GLuint ambLight_loc = glslProgram.getLocation("color_luz_ambiental");
		GLuint difLight_loc = glslProgram.getLocation("color_luz_difuso");
		GLuint specLight_loc = glslProgram.getLocation("color_luz_especular");

		glUniform3f(view_loc, camera.Position[0], camera.Position[1], camera.Position[2]);
		glUniform3f(lightDir_loc, lightDirection[0], lightDirection[1], -lightDirection[2]);
		glUniform3f(light_loc, ejeXL, ejeYL, -ejeZL - 4);

		glUniform1f(shinyBlinn_loc, models[i].shinyBlinn);
		glUniform1i(lightType_loc, currentLight);
		glUniform1i(lightningLoc, toggleLight);
		glUniform1i(bb_loc, 0);

		glUniform1f(refractor_loc, refractor);
		glUniform1f(glslProgram.getLocation("bias"), models[i].bias);

		glUniform4f(ambMat_loc, models[i].color_material_ambiental[0], models[i].color_material_ambiental[1], models[i].color_material_ambiental[2], models[i].color_material_ambiental[3]);
		glUniform4f(difMat_loc, models[i].color_material_difuso[0], models[i].color_material_difuso[1], models[i].color_material_difuso[2], models[i].color_material_difuso[3]);
		glUniform4f(specMat_loc, models[i].color_material_especular[0], models[i].color_material_especular[1], models[i].color_material_especular[2], models[i].color_material_especular[3]);

		glUniform4f(ambLight_loc, color_luz_ambiental[0], color_luz_ambiental[1], color_luz_ambiental[2], color_luz_ambiental[3]);
		glUniform4f(difLight_loc, color_luz_difuso[0], color_luz_difuso[1], color_luz_difuso[2], color_luz_difuso[3]);
		glUniform4f(specLight_loc, color_luz_especular[0], color_luz_especular[1], color_luz_especular[2], color_luz_especular[3]);

		glUniform1f(constantLoc, constant);
		glUniform1f(linearLoc, linear);
		glUniform1f(quadraticLoc, quadratic);
		glUniform1f(inCosLoc, inCos);
		glUniform1f(outCosLoc, outCos);
		glUniform1i(model, i);
		glUniform1i(texturing_loc, models[i].texturing);
		glUniform1i(texturing_normal_loc, models[i].texturingNormal);
		glUniform1i(texturing_light_loc, models[i].texturingLight);
		if (planar) glUniform1i(shadowing_light_loc, 1);
		else glUniform1i(shadowing_light_loc, 0);

		//Matrices de view y projection
		glm::mat4 model_mat;
		glm::mat4 reflect_mat;
		glm::vec3 norm(0.0f, 0.0f, 0.0f);
		glm::vec3 up(0.0f, 1.0f, 0.0f);
		view_mat = camera.GetViewMatrix();
		gluLookAt(eye[0], eye[1], eye[2], norm[0], norm[1], norm[2], up[0], up[1], up[2]);

		model_mat = m.translate_en_matriz(models[i].ejeX, models[i].ejeY, -models[i].ejeZ - 4);

		model_mat = model_mat * m.rotacion_en_matriz(-models[i].rotacion[0], -models[i].rotacion[1], models[i].rotacion[2], models[i].rotacion[3]);

		model_mat = model_mat * m.scale_en_matriz(models[i].scaleT);
		model_mat = glm::scale(model_mat, glm::vec3(1, 1, -1));

		glm::mat4 projectionLightMat, viewLightMat;
		glm::mat4 spaceLightMatrix;
		GLfloat near_plane = 0.0f, far_plane = 5.0f;

		if (currentLight == Directional) {
			projectionLightMat = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, near_plane, far_plane);
			viewLightMat = glm::lookAt(glm::vec3(ejeXL, ejeYL, -ejeZL - 4.0), glm::vec3(0.0, -3.20, -4.0), glm::vec3(0.0, 1.0, 0.0));
		}
		else if (currentLight == Spot) {
			float fov = outCos >= 1.0 ? 0.0 : 2.50 - (outCos*cosPlus);
			projectionLightMat = glm::perspective(fov, (GLfloat)1024 / (GLfloat)1024, 1.0f, 1000.0f);
			glm::vec3 lightInvDir = glm::vec3(lightDirection[0], lightDirection[1], -lightDirection[2]);
			viewLightMat = glm::lookAt(glm::vec3(ejeXL, ejeYL, -ejeZL - 4.0), glm::vec3(ejeXL, ejeYL, -ejeZL - 4.0) + lightInvDir, glm::vec3(0, 1, 0));
		}

		spaceLightMatrix = projectionLightMat * viewLightMat;

		glUniformMatrix4fv(glslProgram.getLocation("light_matrix"), 1, GL_FALSE, glm::value_ptr(spaceLightMatrix));

		glUniformMatrix4fv(model_matr_loc, 1, GL_FALSE, glm::value_ptr(reflect_mat));
		glUniformMatrix4fv(view_matr_loc, 1, GL_FALSE, glm::value_ptr(view_mat));
		glUniformMatrix4fv(reflect_matr_loc, 1, GL_FALSE, glm::value_ptr(model_mat));
		project_mat = glm::perspective(camera.Zoom, (float)width / (float)height, 0.1f, 1000.0f);
		glUniformMatrix4fv(proj_matr_loc, 1, GL_FALSE, glm::value_ptr(project_mat));

		if (planar) {
			//Paso al vertex
			glm::mat4 shadowMat = glm::mat4();

			glm::vec3 light = glm::vec3(ejeXL, ejeYL, -ejeZL - 4);
			shadowMat[0][0] = light[1];
			shadowMat[1][0] = -light[0];
			shadowMat[2][0] = 0.0;
			shadowMat[3][0] = 0.0;

			shadowMat[0][1] = 0.0;
			shadowMat[1][1] = 0.0;
			shadowMat[2][1] = 0.0;
			shadowMat[3][1] = 0.0;

			shadowMat[0][2] = 0.0;
			shadowMat[1][2] = -light[2];
			shadowMat[2][2] = light[1];
			shadowMat[3][2] = 0.0;

			shadowMat[0][3] = 0.0;
			shadowMat[1][3] = -1.0;
			shadowMat[2][3] = 0.0;
			shadowMat[3][3] = light[1];

			//shadowMat = glm::mat4();
			glUniformMatrix4fv(glslProgram.getLocation("planar_matrix"), 1, GL_FALSE, glm::value_ptr(shadowMat));

		}
		else {

			//Paso al vertex la identidad
			glm::mat4 shadowMat = glm::mat4();
			glUniformMatrix4fv(glslProgram.getLocation("planar_matrix"), 1, GL_FALSE, glm::value_ptr(shadowMat));

		}

		glEnable(GL_TEXTURE_2D);

		if (i == 0 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture1);
		}
		else if (i == 1 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture2);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, texture2_light);
		}
		else if (i == 2 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		}
		else if (i == 3) {
			if (models[i].texturing) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture4);
			}
			if (models[i].texturingNormal) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, texture4_normal);
			}
		}
		else if (i == 4) {
			if (models[i].texturing) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture5);
			}
			if (models[i].texturingNormal) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, texture5_normal);
			}
		}
		else if (i == 5 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture6);
		}
		else if (i == 6 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture7);
		}
		else if (i == 7 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture8);
		}
		else if (i == 8 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture9);
		}
		else if (i >= 9 && i <= 12) {
			if (models[i].texturing) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture10);
			}
			if (models[i].texturingNormal) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, texture10_normal);
			}
		}
		else if (i == 13 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture11);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, texture2_light);
		}
		else if (i == 14 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture12);
		}
		else if (i == 15 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture13);
		}
		else if (i == 16 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture14);
		}
		else if (i == 17 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture15);
		}
		else if (i == 18 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture15);
		}
		else if (i == 19 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture15);
		}

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glActiveTexture(GL_TEXTURE0);

		glslProgram.disable();

		if (i == 2) {
			glslSkyboxProgram.enable();
			glUniform1i(glslSkyboxProgram.getLocation("refracting"), true);
		}
		else {
			glUniform1i(glslSkyboxProgram.getLocation("refracting"), false);
			glslSkyboxProgram.disable();
		}

		glslProgram.enable();

		glBindBuffer(GL_ARRAY_BUFFER, models[i].vbo);
		//Se bindean los vértices, normales, coordenadas de texturas y tangentes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[i].vertices.size() * sizeof(float)));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[i].vertices.size() * sizeof(float) + models[i].coord_texturas.size() * sizeof(float)));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[i].vertices.size() * sizeof(float) + models[i].coord_texturas.size() * sizeof(float) + models[i].normales_vertice_fin.size() * sizeof(float)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		if (i != 9)	glDrawArrays(GL_TRIANGLES, 0, models[i].vertices.size() / 3);
		glDisableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glslProgram.disable();

		glPushMatrix();
		glPopMatrix();

		if (selecting && i == selectedModel) {

			models[selectedModel].shinyBlinn = shinyBlinn;

			models[selectedModel].texturing = toggleTexture;
			models[selectedModel].texturingNormal = toggleNormalTexture;
			models[selectedModel].texturingLight = toggleLightTexture;
			models[selectedModel].shadowing = toggleShadows;

			models[selectedModel].color_material_ambiental[0] = color_material_ambiental[0];
			models[selectedModel].color_material_ambiental[1] = color_material_ambiental[1];
			models[selectedModel].color_material_ambiental[2] = color_material_ambiental[2];
			models[selectedModel].color_material_ambiental[3] = color_material_ambiental[3];

			models[selectedModel].color_material_difuso[0] = color_material_difuso[0];
			models[selectedModel].color_material_difuso[1] = color_material_difuso[1];
			models[selectedModel].color_material_difuso[2] = color_material_difuso[2];
			models[selectedModel].color_material_difuso[3] = color_material_difuso[3];

			models[selectedModel].color_material_especular[0] = color_material_especular[0];
			models[selectedModel].color_material_especular[1] = color_material_especular[1];
			models[selectedModel].color_material_especular[2] = color_material_especular[2];
			models[selectedModel].color_material_especular[3] = color_material_especular[3];

			if (selectedModel == 2) { //Si se selecciona al modelo refractante

				models[selectedModel].scaleT = scaleT;

				models[selectedModel].ejeX = ejeX;
				models[selectedModel].ejeY = ejeY;
				models[selectedModel].ejeZ = ejeZ;

				models[selectedModel].rotacion[0] = rotacionPrincipal[0];
				models[selectedModel].rotacion[1] = rotacionPrincipal[1];
				models[selectedModel].rotacion[2] = rotacionPrincipal[2];
				models[selectedModel].rotacion[3] = rotacionPrincipal[3];
				TwDefine("Figura/Refracción visible=true");

			}
			else if (selectedModel != 20) {	//Si se selecciona a algún modelo

				models[selectedModel].scaleT = scaleT;
				models[selectedModel].bias = bias;

				models[selectedModel].ejeX = ejeX;
				models[selectedModel].ejeY = ejeY;
				models[selectedModel].ejeZ = ejeZ;

				models[selectedModel].rotacion[0] = rotacionPrincipal[0];
				models[selectedModel].rotacion[1] = rotacionPrincipal[1];
				models[selectedModel].rotacion[2] = rotacionPrincipal[2];
				models[selectedModel].rotacion[3] = rotacionPrincipal[3];
				TwDefine("Figura/Transformaciones visible=true");
				TwDefine("Figura/Luz visible=false");
				TwDefine("Figura/Material visible=true");
				TwDefine("Figura/toggleTextures visible=true");
				TwDefine("Figura/Refracción visible=false");

			}
			else { //Si se selecciona la luz

				TwDefine("Figura/Transformaciones visible=false");
				TwDefine("Figura/Luz visible=true");
				TwDefine("Figura/Material visible=false");
				TwDefine("Figura/toggleTextures visible=false");
				TwDefine("Figura/Refracción visible=false");

			}

			glDisable(GL_CULL_FACE);

			glLineWidth(2.0);

			glslProgram.enable();

			bb_loc = glslProgram.getLocation("bb");
			glUniform1i(bb_loc, 1);

			if (!planar) {

				glBegin(GL_LINE_STRIP);
				glVertex3f(models[selectedModel].minX, models[selectedModel].minY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].minY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].minY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].minY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].minY, models[selectedModel].minZ);
				glEnd();

				glBegin(GL_LINE_STRIP);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].maxY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].maxY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].minY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].minY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].maxY, models[selectedModel].maxZ);
				glEnd();

				glBegin(GL_LINE_STRIP);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].maxY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].maxY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].maxY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].maxY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].maxY, models[selectedModel].maxZ);
				glEnd();

				glBegin(GL_LINE_STRIP);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].maxY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].minY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].minY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].maxY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].maxY, models[selectedModel].maxZ);
				glEnd();

				glBegin(GL_LINE_STRIP);
				glVertex3f(models[selectedModel].minX, models[selectedModel].minY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].maxY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].maxY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].minY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].minY, models[selectedModel].minZ);
				glEnd();

				glBegin(GL_LINE_STRIP);
				glVertex3f(models[selectedModel].minX, models[selectedModel].minY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].maxY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].maxY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].minY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].minY, models[selectedModel].minZ);
				glEnd();

			}

			glslProgram.disable();
		}
	}
	glDisable(GL_STENCIL_TEST);

	//Devuelvo a propiedades originales para realizar el picking

	if (!planar) {

		glStencilMask(0xFF);
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glClear(GL_STENCIL_BUFFER_BIT);

		if (currentLight == Directional || currentLight == Spot) drawShadows(false);

		glViewport(0, 0, width, height);

		//Devuelvo a propiedades originales para realizar el picking
		glStencilMask(0xFF);
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glClear(GL_STENCIL_BUFFER_BIT);

	}

	//Esto hará el stencil entre la sombra de los modelos y el piso

	if (planar) {

		glEnable(GL_STENCIL_TEST);

		glStencilFunc(GL_ALWAYS, 21, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	}

	//Dibujar el piso
	if (planar) {

		glslProgram.enable();
		glUniform1i(glslProgram.getLocation("texture"), 0);
		glUniform1i(glslProgram.getLocation("textureNormal"), 1);
		glUniform1i(glslProgram.getLocation("skybox"), 2);
		glUniform1i(glslProgram.getLocation("shadowMap"), 3);
		glUniform1i(glslProgram.getLocation("textureLight"), 4);
		GLuint view_matr_loc = glslProgram.getLocation("view_matrix");
		GLuint model_matr_loc = glslProgram.getLocation("model_matrix");
		GLuint reflect_matr_loc = glslProgram.getLocation("reflection_matrix");
		GLuint proj_matr_loc = glslProgram.getLocation("projection_matrix");
		GLuint light_matr_loc = glslProgram.getLocation("light_matrix");
		GLuint light_loc = glslProgram.getLocation("lightPos");
		GLuint view_loc = glslProgram.getLocation("view");
		GLuint shinyBlinn_loc = glslProgram.getLocation("shinyBlinn");
		GLuint model = glslProgram.getLocation("modelNumber");
		GLuint interpol = glslProgram.getLocation("interpol");
		GLuint tipo_difuso = glslProgram.getLocation("tipo_difuso");
		GLuint tipo_especular = glslProgram.getLocation("tipo_especular");
		GLuint lightType_loc = glslProgram.getLocation("lightType");
		GLuint lightDir_loc = glslProgram.getLocation("lightSpotDir");
		GLuint texturing_loc = glslProgram.getLocation("texturing");
		GLuint texturing_normal_loc = glslProgram.getLocation("texturingNormal");
		GLuint lightningLoc = glslProgram.getLocation("lightning");
		GLuint bb_loc = glslProgram.getLocation("bb");
		GLuint constantLoc = glslProgram.getLocation("constant");
		GLuint linearLoc = glslProgram.getLocation("linear");
		GLuint quadraticLoc = glslProgram.getLocation("quadratic");
		GLuint inCosLoc = glslProgram.getLocation("inCos");
		GLuint outCosLoc = glslProgram.getLocation("outCos");
		GLuint refractor_loc = glslProgram.getLocation("refractor");

		GLuint ambMat_loc = glslProgram.getLocation("color_material_ambiental");
		GLuint difMat_loc = glslProgram.getLocation("color_material_difuso");
		GLuint specMat_loc = glslProgram.getLocation("color_material_especular");

		GLuint ambLight_loc = glslProgram.getLocation("color_luz_ambiental");
		GLuint difLight_loc = glslProgram.getLocation("color_luz_difuso");
		GLuint specLight_loc = glslProgram.getLocation("color_luz_especular");

		glUniform3f(view_loc, camera.Position[0], camera.Position[1], camera.Position[2]);
		glUniform3f(lightDir_loc, lightDirection[0], lightDirection[1], lightDirection[2]);
		glUniform3f(light_loc, ejeXL, ejeYL, ejeZL);

		glUniform1f(shinyBlinn_loc, models[0].shinyBlinn);
		glUniform1i(lightType_loc, currentLight);
		glUniform1i(lightningLoc, toggleLight);
		glUniform1i(bb_loc, 0);

		glUniform1f(refractor_loc, refractor);

		glUniform4f(ambMat_loc, models[0].color_material_ambiental[0], models[0].color_material_ambiental[1], models[0].color_material_ambiental[2], models[0].color_material_ambiental[3]);
		glUniform4f(difMat_loc, models[0].color_material_difuso[0], models[0].color_material_difuso[1], models[0].color_material_difuso[2], models[0].color_material_difuso[3]);
		glUniform4f(specMat_loc, models[0].color_material_especular[0], models[0].color_material_especular[1], models[0].color_material_especular[2], models[0].color_material_especular[3]);

		glUniform4f(ambLight_loc, color_luz_ambiental[0], color_luz_ambiental[1], color_luz_ambiental[2], color_luz_ambiental[3]);
		glUniform4f(difLight_loc, color_luz_difuso[0], color_luz_difuso[1], color_luz_difuso[2], color_luz_difuso[3]);
		glUniform4f(specLight_loc, color_luz_especular[0], color_luz_especular[1], color_luz_especular[2], color_luz_especular[3]);

		glUniform1f(constantLoc, constant);
		glUniform1f(linearLoc, linear);
		glUniform1f(quadraticLoc, quadratic);
		glUniform1f(inCosLoc, inCos);
		glUniform1f(outCosLoc, outCos);
		glUniform1i(model, 0);
		glUniform1i(texturing_loc, models[0].texturing);
		glUniform1i(texturing_normal_loc, models[0].texturingNormal);
		//Matrices de view y projection
		glm::mat4 model_mat;
		glm::mat4 reflect_mat;
		glm::vec3 norm(0.0f, 0.0f, 0.0f);
		glm::vec3 up(0.0f, 1.0f, 0.0f);
		view_mat = camera.GetViewMatrix();
		gluLookAt(eye[0], eye[1], eye[2], norm[0], norm[1], norm[2], up[0], up[1], up[2]);

		model_mat = m.translate_en_matriz(models[0].ejeX, models[0].ejeY, models[0].ejeZ);
		model_mat = model_mat * m.rotacion_en_matriz(models[0].rotacion[0], models[0].rotacion[1], models[0].rotacion[2], models[0].rotacion[3]);

		model_mat = model_mat * m.scale_en_matriz(models[0].scaleT);

		glUniformMatrix4fv(model_matr_loc, 1, GL_FALSE, glm::value_ptr(model_mat));
		glUniformMatrix4fv(reflect_matr_loc, 1, GL_FALSE, glm::value_ptr(reflect_mat));
		glUniformMatrix4fv(view_matr_loc, 1, GL_FALSE, glm::value_ptr(view_mat));

		project_mat = glm::perspective(camera.Zoom, (float)width / (float)height, 0.1f, 1000.0f);
		glUniformMatrix4fv(proj_matr_loc, 1, GL_FALSE, glm::value_ptr(project_mat));
		glm::mat4 shadowMat = glm::mat4();
		glUniformMatrix4fv(glslProgram.getLocation("planar_matrix"), 1, GL_FALSE, glm::value_ptr(shadowMat));

		glEnable(GL_TEXTURE_2D);

		if (models[0].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture1);
		}

		glslProgram.enable();

		glBindBuffer(GL_ARRAY_BUFFER, models[0].vbo);
		//Se bindean los vértices, normales, coordenadas de texturas y tangentes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[0].vertices.size() * sizeof(float)));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[0].vertices.size() * sizeof(float) + models[0].coord_texturas.size() * sizeof(float)));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[0].vertices.size() * sizeof(float) + models[0].coord_texturas.size() * sizeof(float) + models[0].normales_vertice_fin.size() * sizeof(float)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, models[0].vertices.size() / 3);
		glDisableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glslProgram.disable();

		glPushMatrix();
		glPopMatrix();

		glDisable(GL_STENCIL_TEST);
	}

	if (planar) {

		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, 21, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

	}

	//Modelos originales
	for (int i = 0; i<models.size(); i++) { //Para los modelos

		if (planar && i == 0) i = 1;
		if (planar && i == 2) i = 3;
		if (planar && i >= 9 && i <= 12) i = 13;
		if (planar && i == 20) break;
		if (!planar) glStencilFunc(GL_ALWAYS, i, -1);
		glslProgram.enable();
		glUniform1i(glslProgram.getLocation("texture"), 0);
		glUniform1i(glslProgram.getLocation("textureNormal"), 1);
		glUniform1i(glslProgram.getLocation("skybox"), 2);
		glUniform1i(glslProgram.getLocation("shadowMap"), 3);
		glUniform1i(glslProgram.getLocation("textureLight"), 4);
		GLuint view_matr_loc = glslProgram.getLocation("view_matrix");
		GLuint model_matr_loc = glslProgram.getLocation("model_matrix");
		GLuint reflect_matr_loc = glslProgram.getLocation("reflection_matrix");
		GLuint proj_matr_loc = glslProgram.getLocation("projection_matrix");
		GLuint light_loc = glslProgram.getLocation("lightPos");
		GLuint view_loc = glslProgram.getLocation("view");
		GLuint shinyBlinn_loc = glslProgram.getLocation("shinyBlinn");
		GLuint model = glslProgram.getLocation("modelNumber");
		GLuint interpol = glslProgram.getLocation("interpol");
		GLuint tipo_difuso = glslProgram.getLocation("tipo_difuso");
		GLuint tipo_especular = glslProgram.getLocation("tipo_especular");
		GLuint lightType_loc = glslProgram.getLocation("lightType");
		GLuint lightDir_loc = glslProgram.getLocation("lightSpotDir");
		GLuint texturing_loc = glslProgram.getLocation("texturing");
		GLuint texturing_normal_loc = glslProgram.getLocation("texturingNormal");
		GLuint texturing_light_loc = glslProgram.getLocation("texturingLight");
		GLuint shadowing_loc = glslProgram.getLocation("shadowing");
		GLuint shadowing_light_loc = glslProgram.getLocation("shadowingLightMap");
		GLuint lightningLoc = glslProgram.getLocation("lightning");
		GLuint bb_loc = glslProgram.getLocation("bb");
		GLuint constantLoc = glslProgram.getLocation("constant");
		GLuint linearLoc = glslProgram.getLocation("linear");
		GLuint quadraticLoc = glslProgram.getLocation("quadratic");
		GLuint inCosLoc = glslProgram.getLocation("inCos");
		GLuint outCosLoc = glslProgram.getLocation("outCos");
		GLuint refractor_loc = glslProgram.getLocation("refractor");

		GLuint ambMat_loc = glslProgram.getLocation("color_material_ambiental");
		GLuint difMat_loc = glslProgram.getLocation("color_material_difuso");
		GLuint specMat_loc = glslProgram.getLocation("color_material_especular");

		GLuint ambLight_loc = glslProgram.getLocation("color_luz_ambiental");
		GLuint difLight_loc = glslProgram.getLocation("color_luz_difuso");
		GLuint specLight_loc = glslProgram.getLocation("color_luz_especular");

		glUniform3f(view_loc, camera.Position[0], camera.Position[1], camera.Position[2]);
		glUniform3f(lightDir_loc, lightDirection[0], lightDirection[1], lightDirection[2]);
		glUniform3f(light_loc, ejeXL, ejeYL, ejeZL);

		glUniform1f(shinyBlinn_loc, models[i].shinyBlinn);
		glUniform1i(lightType_loc, currentLight);
		glUniform1i(lightningLoc, toggleLight);
		glUniform1i(bb_loc, 0);

		glUniform1f(refractor_loc, refractor);
		glUniform1f(glslProgram.getLocation("bias"), models[i].bias);

		glUniform4f(ambMat_loc, models[i].color_material_ambiental[0], models[i].color_material_ambiental[1], models[i].color_material_ambiental[2], models[i].color_material_ambiental[3]);
		glUniform4f(difMat_loc, models[i].color_material_difuso[0], models[i].color_material_difuso[1], models[i].color_material_difuso[2], models[i].color_material_difuso[3]);
		glUniform4f(specMat_loc, models[i].color_material_especular[0], models[i].color_material_especular[1], models[i].color_material_especular[2], models[i].color_material_especular[3]);

		glUniform4f(ambLight_loc, color_luz_ambiental[0], color_luz_ambiental[1], color_luz_ambiental[2], color_luz_ambiental[3]);
		glUniform4f(difLight_loc, color_luz_difuso[0], color_luz_difuso[1], color_luz_difuso[2], color_luz_difuso[3]);
		glUniform4f(specLight_loc, color_luz_especular[0], color_luz_especular[1], color_luz_especular[2], color_luz_especular[3]);

		glUniform1f(constantLoc, constant);
		glUniform1f(linearLoc, linear);
		glUniform1f(quadraticLoc, quadratic);
		glUniform1f(inCosLoc, inCos);
		glUniform1f(outCosLoc, outCos);
		glUniform1i(model, i);
		glUniform1i(texturing_loc, models[i].texturing);
		glUniform1i(texturing_normal_loc, models[i].texturingNormal);
		glUniform1i(texturing_light_loc, models[i].texturingLight);
		glUniform1i(shadowing_loc, models[i].shadowing);

		if (planar) glUniform1i(shadowing_light_loc, 1);
		else glUniform1i(shadowing_light_loc, 0);

		//Matrices de view y projection
		glm::mat4 model_mat;
		glm::mat4 reflect_mat;
		glm::vec3 norm(0.0f, 0.0f, 0.0f);
		glm::vec3 up(0.0f, 1.0f, 0.0f);
		view_mat = camera.GetViewMatrix();
		gluLookAt(eye[0], eye[1], eye[2], norm[0], norm[1], norm[2], up[0], up[1], up[2]);

		model_mat = m.translate_en_matriz(models[i].ejeX, models[i].ejeY, models[i].ejeZ);

		model_mat = model_mat * m.rotacion_en_matriz(models[i].rotacion[0], models[i].rotacion[1], models[i].rotacion[2], models[i].rotacion[3]);

		model_mat = model_mat * m.scale_en_matriz(models[i].scaleT);

		glm::mat4 projectionLightMat, viewLightMat;
		glm::mat4 spaceLightMatrix;
		GLfloat near_plane = 0.0f, far_plane = 5.0f;

		if (currentLight == Directional) {
			projectionLightMat = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, near_plane, far_plane);
			viewLightMat = glm::lookAt(glm::vec3(ejeXL, ejeYL, ejeZL), glm::vec3(0.0, -3.20, 0.0), glm::vec3(0.0, 1.0, 0.0));
		}
		else if (currentLight == Spot) {
			float fov = outCos >= 1.0 ? 0.0 : 2.50 - (outCos*cosPlus);
			projectionLightMat = glm::perspective(fov, (GLfloat)1024 / (GLfloat)1024, 1.0f, 1000.0f);
			glm::vec3 lightInvDir = glm::vec3(lightDirection[0], lightDirection[1], lightDirection[2]);
			viewLightMat = glm::lookAt(glm::vec3(ejeXL, ejeYL, ejeZL), glm::vec3(ejeXL, ejeYL, ejeZL) + lightInvDir, glm::vec3(0, 1, 0));
		}
		if (currentLight == Directional || currentLight == Spot) spaceLightMatrix = projectionLightMat * viewLightMat;

		glUniformMatrix4fv(glslProgram.getLocation("light_matrix"), 1, GL_FALSE, glm::value_ptr(spaceLightMatrix));

		glUniformMatrix4fv(model_matr_loc, 1, GL_FALSE, glm::value_ptr(model_mat));
		glUniformMatrix4fv(view_matr_loc, 1, GL_FALSE, glm::value_ptr(view_mat));
		glUniformMatrix4fv(reflect_matr_loc, 1, GL_FALSE, glm::value_ptr(reflect_mat));
		project_mat = glm::perspective(camera.Zoom, (float)width / (float)height, 0.1f, 1000.0f);
		glUniformMatrix4fv(proj_matr_loc, 1, GL_FALSE, glm::value_ptr(project_mat));

		if (planar) {
			//Paso al vertex
			glm::mat4 shadowMat = glm::mat4();

			glm::vec3 light = glm::vec3(ejeXL, ejeYL, ejeZL);
			shadowMat[0][0] = light[1];
			shadowMat[1][0] = -light[0];
			shadowMat[2][0] = 0.0;
			shadowMat[3][0] = 0.0;

			shadowMat[0][1] = 0.0;
			shadowMat[1][1] = 0.0;
			shadowMat[2][1] = 0.0;
			shadowMat[3][1] = 0.0;

			shadowMat[0][2] = 0.0;
			shadowMat[1][2] = -light[2];
			shadowMat[2][2] = light[1];
			shadowMat[3][2] = 0.0;

			shadowMat[0][3] = 0.0;
			shadowMat[1][3] = -1.0;
			shadowMat[2][3] = 0.0;
			shadowMat[3][3] = light[1];

			glUniformMatrix4fv(glslProgram.getLocation("planar_matrix"), 1, GL_FALSE, glm::value_ptr(shadowMat));

		}
		else {

			//Paso al vertex la identidad
			glm::mat4 shadowMat = glm::mat4();
			glUniformMatrix4fv(glslProgram.getLocation("planar_matrix"), 1, GL_FALSE, glm::value_ptr(shadowMat));

		}

		glEnable(GL_TEXTURE_2D);

		if (i == 0 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture1);
		}
		else if (i == 1 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture2);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, texture2_light);
		}
		else if (i == 2 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		}
		else if (i == 3) {
			if (models[i].texturing) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture4);
			}
			if (models[i].texturingNormal) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, texture4_normal);
			}
		}
		else if (i == 4) {
			if (models[i].texturing) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture5);
			}
			if (models[i].texturingNormal) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, texture5_normal);
			}
		}
		else if (i == 5 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture6);
		}
		else if (i == 6 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture7);
		}
		else if (i == 7 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture8);
		}
		else if (i == 8 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture9);
		}
		else if (i >= 9 && i <= 12) {
			if (models[i].texturing) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture10);
			}
			if (models[i].texturingNormal) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, texture10_normal);
			}
		}
		else if (i == 13 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture11);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, texture2_light);
		}
		else if (i == 14 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture12);
		}
		else if (i == 15 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture13);
		}
		else if (i == 16 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture14);
		}
		else if (i == 17 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture15);
		}
		else if (i == 18 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture15);
		}
		else if (i == 19 && models[i].texturing) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture15);
		}

		glslProgram.disable();

		if (i == 2) {
			glslSkyboxProgram.enable();
			glUniform1i(glslSkyboxProgram.getLocation("refracting"), true);
		}
		else {
			glUniform1i(glslSkyboxProgram.getLocation("refracting"), false);
			glslSkyboxProgram.disable();
		}

		glslProgram.enable();

		glBindBuffer(GL_ARRAY_BUFFER, models[i].vbo);
		//Se bindean los vértices, normales, coordenadas de texturas y tangentes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[i].vertices.size() * sizeof(float)));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[i].vertices.size() * sizeof(float) + models[i].coord_texturas.size() * sizeof(float)));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(models[i].vertices.size() * sizeof(float) + models[i].coord_texturas.size() * sizeof(float) + models[i].normales_vertice_fin.size() * sizeof(float)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		if (i != 9) glDrawArrays(GL_TRIANGLES, 0, models[i].vertices.size() / 3);
		glDisableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glslProgram.disable();

		if (selecting && i == selectedModel) {

			models[selectedModel].shinyBlinn = shinyBlinn;

			models[selectedModel].texturing = toggleTexture;
			models[selectedModel].texturingNormal = toggleNormalTexture;
			models[selectedModel].texturingLight = toggleLightTexture;
			models[selectedModel].shadowing = toggleShadows;

			models[selectedModel].color_material_ambiental[0] = color_material_ambiental[0];
			models[selectedModel].color_material_ambiental[1] = color_material_ambiental[1];
			models[selectedModel].color_material_ambiental[2] = color_material_ambiental[2];
			models[selectedModel].color_material_ambiental[3] = color_material_ambiental[3];

			models[selectedModel].color_material_difuso[0] = color_material_difuso[0];
			models[selectedModel].color_material_difuso[1] = color_material_difuso[1];
			models[selectedModel].color_material_difuso[2] = color_material_difuso[2];
			models[selectedModel].color_material_difuso[3] = color_material_difuso[3];

			models[selectedModel].color_material_especular[0] = color_material_especular[0];
			models[selectedModel].color_material_especular[1] = color_material_especular[1];
			models[selectedModel].color_material_especular[2] = color_material_especular[2];
			models[selectedModel].color_material_especular[3] = color_material_especular[3];

			if (selectedModel == 1 || selectedModel == 13) { //Si se selecciona a los modelos con lightmap

				models[selectedModel].scaleT = scaleT;

				models[selectedModel].ejeX = ejeX;
				models[selectedModel].ejeY = ejeY;
				models[selectedModel].ejeZ = ejeZ;

				models[selectedModel].rotacion[0] = rotacionPrincipal[0];
				models[selectedModel].rotacion[1] = rotacionPrincipal[1];
				models[selectedModel].rotacion[2] = rotacionPrincipal[2];
				models[selectedModel].rotacion[3] = rotacionPrincipal[3];
				TwDefine("Figura/Transformaciones visible=true");
				TwDefine("Figura/Refracción visible=false");
				TwDefine("Figura/Luz visible=false");
				TwDefine("Figura/Material visible=false");
				TwDefine("Figura/toggleTextures visible=false");
				TwDefine("Figura/toggleNormalTextures visible=false");
				TwDefine("Figura/toggleLightTextures visible=true");
				TwDefine("Figura/Sombras visible=false");
				TwDefine("Figura/Blinn-Phong visible=false");

			}
			else if (selectedModel == 2) { //Si se selecciona al modelo refractante

				models[selectedModel].scaleT = scaleT;

				models[selectedModel].ejeX = ejeX;
				models[selectedModel].ejeY = ejeY;
				models[selectedModel].ejeZ = ejeZ;

				models[selectedModel].rotacion[0] = rotacionPrincipal[0];
				models[selectedModel].rotacion[1] = rotacionPrincipal[1];
				models[selectedModel].rotacion[2] = rotacionPrincipal[2];
				models[selectedModel].rotacion[3] = rotacionPrincipal[3];
				TwDefine("Figura/Transformaciones visible=true");
				TwDefine("Figura/Refracción visible=true");
				TwDefine("Figura/Luz visible=false");
				TwDefine("Figura/Material visible=false");
				TwDefine("Figura/toggleTextures visible=false");
				TwDefine("Figura/toggleNormalTextures visible=false");
				TwDefine("Figura/toggleLightTextures visible=false");
				TwDefine("Figura/Sombras visible=false");
				TwDefine("Figura/Blinn-Phong visible=false");

			}
			else if (selectedModel != 20) {	//Si se selecciona a algún modelo

				models[selectedModel].scaleT = scaleT;
				models[selectedModel].bias = bias;

				models[selectedModel].ejeX = ejeX;
				models[selectedModel].ejeY = ejeY;
				models[selectedModel].ejeZ = ejeZ;

				models[selectedModel].rotacion[0] = rotacionPrincipal[0];
				models[selectedModel].rotacion[1] = rotacionPrincipal[1];
				models[selectedModel].rotacion[2] = rotacionPrincipal[2];
				models[selectedModel].rotacion[3] = rotacionPrincipal[3];
				TwDefine("Figura/Transformaciones visible=true");
				TwDefine("Figura/Luz visible=false");
				TwDefine("Figura/Material visible=true");
				TwDefine("Figura/toggleTextures visible=true");
				TwDefine("Figura/toggleLightTextures visible=false");
				if (currentLight != Point) TwDefine("Figura/Sombras visible=true");
				TwDefine("Figura/Blinn-Phong visible=true");
				if (selectedModel == 3 || selectedModel == 4 || (selectedModel >= 10 && selectedModel <= 12)) {
					TwDefine("Figura/toggleNormalTextures visible=true");
				}
				else {
					TwDefine("Figura/toggleNormalTextures visible=false");
				}
				TwDefine("Figura/Refracción visible=false");

			}
			else { //Si se selecciona la luz

				TwDefine("Figura/Transformaciones visible=false");
				TwDefine("Figura/Luz visible=true");
				TwDefine("Figura/Material visible=false");
				TwDefine("Figura/toggleTextures visible=false");
				TwDefine("Figura/toggleNormalTextures visible=false");
				TwDefine("Figura/toggleLightTextures visible=false");
				TwDefine("Figura/Sombras visible=false");
				TwDefine("Figura/Refracción visible=false");
				TwDefine("Figura/Blinn-Phong visible=false");
				if (currentLight == Point) TwDefine("Figura/toggleDepthMap visible=false");
				else TwDefine("Figura/toggleDepthMap visible=true");				

			}

			glDisable(GL_CULL_FACE);

			glLineWidth(2.0);

			glslProgram.enable();

			bb_loc = glslProgram.getLocation("bb");
			glUniform1i(bb_loc, 1);

			if (!planar) {

				glBegin(GL_LINE_STRIP);
				glVertex3f(models[selectedModel].minX, models[selectedModel].minY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].minY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].minY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].minY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].minY, models[selectedModel].minZ);
				glEnd();

				glBegin(GL_LINE_STRIP);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].maxY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].maxY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].minY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].minY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].maxY, models[selectedModel].maxZ);
				glEnd();

				glBegin(GL_LINE_STRIP);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].maxY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].maxY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].maxY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].maxY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].maxY, models[selectedModel].maxZ);
				glEnd();

				glBegin(GL_LINE_STRIP);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].maxY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].minY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].minY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].maxY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].maxY, models[selectedModel].maxZ);
				glEnd();

				glBegin(GL_LINE_STRIP);
				glVertex3f(models[selectedModel].minX, models[selectedModel].minY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].maxY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].maxY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].minY, models[selectedModel].maxZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].minY, models[selectedModel].minZ);
				glEnd();

				glBegin(GL_LINE_STRIP);
				glVertex3f(models[selectedModel].minX, models[selectedModel].minY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].maxY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].maxY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].maxX, models[selectedModel].minY, models[selectedModel].minZ);
				glVertex3f(models[selectedModel].minX, models[selectedModel].minY, models[selectedModel].minZ);
				glEnd();

			}

			glslProgram.disable();
		}
	}

	glDisable(GL_STENCIL_TEST);

}