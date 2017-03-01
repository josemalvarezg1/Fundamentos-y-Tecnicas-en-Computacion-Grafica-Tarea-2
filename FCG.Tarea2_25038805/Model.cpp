#include "Model.h"

extern vector<model> models; //Todos los modelos irán en este vector
extern int imageWidth, imageHeight, imageFormat; //Tamaño y formato de la textura
extern CGLSLProgram glslProgram; //Programa de shaders
extern CGLSLProgram glslSkyboxProgram; //Programa de shaders del skybox
extern CGLSLProgram glslDepthProgram; //Programa de shaders del depth map
extern CGLSLProgram glslShadowProgram; //Programa de shaders de las sombras

//Crea los VBOS con vértice, normal y textura
void model::initVBO(int size) {
	glGenBuffers(1, &(models[models.size() - 1].vbo));
	glBindBuffer(GL_ARRAY_BUFFER, models[models.size() - 1].vbo);
	glBufferData(GL_ARRAY_BUFFER, models[models.size() - 1].vertices.size() * sizeof(float) + models[models.size() - 1].coord_texturas.size() * sizeof(float) + models[models.size() - 1].normales_vertice_fin.size() * sizeof(float) + models[models.size() - 1].tangents.size() * sizeof(glm::vec3), NULL, GL_STATIC_DRAW);
	//Guardo Vertices en el VBO
	glBufferSubData(GL_ARRAY_BUFFER,
		0,
		models[models.size() - 1].vertices.size() * sizeof(float),
		models[models.size() - 1].vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER,
		models[models.size() - 1].vertices.size() * sizeof(float),
		models[models.size() - 1].coord_texturas.size() * sizeof(float),
		models[models.size() - 1].coord_texturas.data());
	glBufferSubData(GL_ARRAY_BUFFER,
		models[models.size() - 1].vertices.size() * sizeof(float) + models[models.size() - 1].coord_texturas.size() * sizeof(float),
		models[models.size() - 1].normales_vertice_fin.size() * sizeof(float),
		models[models.size() - 1].normales_vertice_fin.data());
	glBufferSubData(GL_ARRAY_BUFFER,
		models[models.size() - 1].vertices.size() * sizeof(float) + models[models.size() - 1].coord_texturas.size() * sizeof(float) + models[models.size() - 1].normales_vertice_fin.size() * sizeof(float),
		models[models.size() - 1].tangents.size() * sizeof(glm::vec3),
		models[models.size() - 1].tangents.data());
	glBindVertexArray(0);
}

//Lector de OBJ (con normales, textura y posición de modelos en el mapa)
void model::read_obj(char *filename) {
	vector<vertice> auxVertices_1, auxVertices_2;
	vector<vertice> coord_texturasAux;
	char line[4096] = ""; //Leo 4096 char en una línea
	ifstream file;
	bool centrado = true;
	file.open(filename);
	string comprobacion;
	double x, y, z, minX = INT_MAX, minY = INT_MAX, minZ = INT_MAX, maxX = INT_MIN, maxY = INT_MIN, maxZ = INT_MIN;
	vertice v_secundario, normal;
	model m;
	vector<face> faces;
	while (!file.eof()) { //Hasta terminar el archivo
		file >> comprobacion; // Leo el primer string (para ver si es vn, vt, v o f)
		if (comprobacion == "v") {
			//Como es v leo los x y z, agrego al vector y sumo un vértice
			file >> x >> y >> z;
			if (x>maxX) maxX = x; if (y>maxY) maxY = y; if (z>maxZ) maxZ = z;
			if (x<minX) minX = x; if (y<minY) minY = y; if (z<minZ) minZ = z;
			v_secundario.x = x;
			v_secundario.y = y;
			v_secundario.z = z;
			auxVertices_1.push_back(v_secundario);
		}
		else if (comprobacion == "vt") {
			//Obtengo las coordenadas de texturas
			file >> x >> y;
			vertice a;
			a.x = x;
			a.y = y;
			coord_texturasAux.push_back(a);
		}
		else if (comprobacion == "f") {              
			//Obtengo la cara
			face cara;
			int nFacesAux = 0;
			file.getline(line, 4096); //Obtengo toda la línea (sin el "f")
			vector<string> indices_vertices;
			string f_auxiliar = "";
			string auxiliar = line;
			auxiliar.erase(auxiliar.find_last_not_of(" ") + 1); //Borro espacios al final de la línea
			stringstream ss(auxiliar);
			while (!ss.eof()) { //Recorro la línea que agarré hasta el final
				ss >> f_auxiliar; //Leo strings
				if (f_auxiliar != "\0") {
					indices_vertices = split(f_auxiliar, '/'); //Separo el string por "/"
					nFacesAux++;
					const char * c = indices_vertices[0].c_str(); //Siempre el [0] sera el vértice, lo transformo a int
					int real_index = atoi(c) - 1;
					cara.f.push_back(real_index);
					c = indices_vertices[1].c_str(); //Siempre el [1] sera la textura, la transformo a int
					real_index = atoi(c) - 1;
					cara.t.push_back(real_index);
					auxiliar = f_auxiliar;
				}

			}
			//Terminé esa cara, pusheo al vector de caras
			if (nFacesAux != 0) {
				cara.n_vertex = nFacesAux;
				faces.push_back(cara);
			}

		}
		else if ((comprobacion[0] != 'v' && comprobacion[0] != 'f')) {
			//Si es vn salto la línea
			file.getline(line, 4096);
		}
	}

	//Saco los vértices centrales del modelo
	double cx = (maxX + minX) / 2.0; double cy = (maxY + minY) / 2.0; double cz = (maxZ + minZ) / 2.0;
	vertice v, nor;
	double maxXYZ = max(maxX, max(maxY, maxZ));
	minX = INT_MAX, minY = INT_MAX, minZ = INT_MAX, maxX = INT_MIN, maxY = INT_MIN, maxZ = INT_MIN;

	vector<int> totales;
	vector<vertice> normales_pavo;

	for (int i = 0; i < auxVertices_1.size(); i++) {

		v.x = (auxVertices_1[i].x - cx) / maxXYZ;
		v.y = (auxVertices_1[i].y - cy) / maxXYZ;
		v.z = (auxVertices_1[i].z - cz) / maxXYZ;
		if (v.x>maxX) maxX = v.x; if (v.y>maxY) maxY = v.y; if (v.z>maxZ) maxZ = v.z;
		if (v.x<minX) minX = v.x; if (v.y<minY) minY = v.y; if (v.z<minZ) minZ = v.z;

		auxVertices_2.push_back(v);

		nor.x = 0;
		nor.y = 0;
		nor.z = 0;
		normales_pavo.push_back(nor);

		totales.push_back(0);
	}

	for (int i = 0; i < faces.size(); i++) {
		int v1, v2, v3, t1, t2, t3, aux = 1, k = 0, aux2 = 1;
		vertice u, v;

		while (faces[i].n_vertex - 1 > aux) {
			v1 = faces[i].f[k];
			v2 = faces[i].f[aux];
			v3 = faces[i].f[++aux];

			m.vertices.push_back(auxVertices_2[v1].x);
			m.vertices.push_back(auxVertices_2[v1].y);
			m.vertices.push_back(auxVertices_2[v1].z);

			m.vertices.push_back(auxVertices_2[v2].x);
			m.vertices.push_back(auxVertices_2[v2].y);
			m.vertices.push_back(auxVertices_2[v2].z);

			m.vertices.push_back(auxVertices_2[v3].x);
			m.vertices.push_back(auxVertices_2[v3].y);
			m.vertices.push_back(auxVertices_2[v3].z);

			t1 = faces[i].t[k];
			t2 = faces[i].t[aux2];
			t3 = faces[i].t[++aux2];

			m.coord_texturas.push_back(coord_texturasAux[t1].x);
			m.coord_texturas.push_back(coord_texturasAux[t1].y);

			m.coord_texturas.push_back(coord_texturasAux[t2].x);
			m.coord_texturas.push_back(coord_texturasAux[t2].y);

			m.coord_texturas.push_back(coord_texturasAux[t3].x);
			m.coord_texturas.push_back(coord_texturasAux[t3].y);

			//Saco normales
			u.x = auxVertices_2[v2].x - auxVertices_2[v1].x;
			u.y = auxVertices_2[v2].y - auxVertices_2[v1].y;
			u.z = auxVertices_2[v2].z - auxVertices_2[v1].z;
			v.x = auxVertices_2[v3].x - auxVertices_2[v1].x;
			v.y = auxVertices_2[v3].y - auxVertices_2[v1].y;
			v.z = auxVertices_2[v3].z - auxVertices_2[v1].z;
			normal.x = u.y*v.z - u.z*v.y;
			normal.y = u.z*v.x - u.x*v.z;
			normal.z = u.x*v.y - u.y*v.x;

			//Cálculo de las tangentes

			glm::vec3 pos0 = glm::vec3(auxVertices_2[v1].x, auxVertices_2[v1].y, auxVertices_2[v1].z);
			glm::vec3 pos1 = glm::vec3(auxVertices_2[v2].x, auxVertices_2[v2].y, auxVertices_2[v2].z);
			glm::vec3 pos2 = glm::vec3(auxVertices_2[v3].x, auxVertices_2[v3].y, auxVertices_2[v3].z);

			glm::vec2 uv0 = glm::vec2(coord_texturasAux[t1].x, coord_texturasAux[t1].y);
			glm::vec2 uv1 = glm::vec2(coord_texturasAux[t2].x, coord_texturasAux[t2].y);
			glm::vec2 uv2 = glm::vec2(coord_texturasAux[t3].x, coord_texturasAux[t3].y);
			
			glm::vec3 deltaPos1 = pos1 - pos0;
			glm::vec3 deltaPos2 = pos2 - pos0;

			glm::vec2 deltaUV1 = uv1 - uv0;
			glm::vec2 deltaUV2 = uv2 - uv0;

			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
			glm::vec3 tangent;

			tangent.x = r * (deltaUV2.y * deltaPos1.x - deltaUV1.y * deltaPos2.x);
			tangent.y = r * (deltaUV2.y * deltaPos1.y - deltaUV1.y * deltaPos2.y);
			tangent.z = r * (deltaUV2.y * deltaPos1.z - deltaUV1.y * deltaPos2.z);

			if (tangent.x == -0) tangent.x = abs(tangent.x);
			if (tangent.y == -0) tangent.y = abs(tangent.y);
			if (tangent.z == -0) tangent.z = abs(tangent.z);

			tangent = glm::normalize(tangent);

			m.tangents.push_back(tangent);
			m.tangents.push_back(tangent);
			m.tangents.push_back(tangent);

			//Guardo suma de normales por vértice
			normales_pavo[v1].x += normal.x;
			normales_pavo[v1].y += normal.y;
			normales_pavo[v1].z += normal.z;

			normales_pavo[v2].x += normal.x;
			normales_pavo[v2].y += normal.y;
			normales_pavo[v2].z += normal.z;

			normales_pavo[v3].x += normal.x;
			normales_pavo[v3].y += normal.y;
			normales_pavo[v3].z += normal.z;

			//Guardo total de la suma de normales para dividir
			totales[v1] += 1;
			totales[v2] += 1;
			totales[v3] += 1;

		}
	}

	
	for (int i = 0; i < faces.size(); i++) {
		int v1, v2, v3, aux = 1, k = 0;
		vertice u, v, final;

		while (faces[i].n_vertex - 1 > aux) {
			v1 = faces[i].f[k];
			v2 = faces[i].f[aux];
			v3 = faces[i].f[++aux];
			final.x = (normales_pavo[v1].x / (float)totales[v1]);
			final.y = (normales_pavo[v1].y / (float)totales[v1]);
			final.z = (normales_pavo[v1].z / (float)totales[v1]);

			m.normales_vertice_fin.push_back(final.x); m.normales_vertice_fin.push_back(final.y); m.normales_vertice_fin.push_back(final.z);

			final.x = (normales_pavo[v2].x / (float)totales[v2]);
			final.y = (normales_pavo[v2].y / (float)totales[v2]);
			final.z = (normales_pavo[v2].z / (float)totales[v2]);

			m.normales_vertice_fin.push_back(final.x); m.normales_vertice_fin.push_back(final.y); m.normales_vertice_fin.push_back(final.z);

			final.x = (normales_pavo[v3].x / (float)totales[v3]);
			final.y = (normales_pavo[v3].y / (float)totales[v3]);
			final.z = (normales_pavo[v3].z / (float)totales[v3]);

			m.normales_vertice_fin.push_back(final.x); m.normales_vertice_fin.push_back(final.y); m.normales_vertice_fin.push_back(final.z);
			
			m.tangents[v1].x = (m.tangents[v1].x / (float)totales[v1]);
			m.tangents[v1].y = (m.tangents[v1].y / (float)totales[v1]);
			m.tangents[v1].z = (m.tangents[v1].z / (float)totales[v1]);

			m.tangents[v2].x = (m.tangents[v2].x / (float)totales[v2]);
			m.tangents[v2].y = (m.tangents[v2].y / (float)totales[v2]);
			m.tangents[v2].z = (m.tangents[v2].z / (float)totales[v2]);

			m.tangents[v3].x = (m.tangents[v3].x / (float)totales[v3]);
			m.tangents[v3].y = (m.tangents[v3].y / (float)totales[v3]);
			m.tangents[v3].z = (m.tangents[v3].z / (float)totales[v3]);

		}
	}

	m.minX = minX;
	m.maxX = maxX;
	m.minY = minY;
	m.maxY = maxY;
	m.minZ = minZ;
	m.maxZ = maxZ;

	models.push_back(m); //Pusheo el modelo a vector de modelos
	if (models.size() - 1 == 0) {	//Plano
		models[models.size() - 1].scaleT = 2.00;
		models[models.size() - 1].ejeY = -0.001;
	}
	else if (models.size() - 1 == 1) {	//Cubo Master
		models[models.size() - 1].scaleT = 0.15;
		models[models.size() - 1].ejeX = 1.51;
		models[models.size() - 1].ejeY = 0.23;
		models[models.size() - 1].ejeZ = -1.33;
		models[models.size() - 1].texturingLight = 1;
		models[models.size() - 1].bias = 0.001;
	}
	else if (models.size() - 1 == 2) { //Brawler
		models[models.size() - 1].scaleT = 0.72;
		models[models.size() - 1].ejeY = 0.37;
	}
	else if (models.size() - 1 == 3) { //Flash
		models[models.size() - 1].scaleT = 0.61;
		models[models.size() - 1].ejeX = 0.95;
		models[models.size() - 1].ejeY = 0.30;
		models[models.size() - 1].ejeZ = -1.23;
		models[models.size() - 1].texturingNormal = 1;
		models[models.size() - 1].bias = 0.011;
	}
	else if (models.size() - 1 == 4) { //Arrow
		models[models.size() - 1].scaleT = 0.61;
		models[models.size() - 1].ejeX = 0.17;
		models[models.size() - 1].ejeY = 0.18;
		models[models.size() - 1].ejeZ = -1.23;
		models[models.size() - 1].texturingNormal = 1;
		models[models.size() - 1].bias = 0.019;
	}
	else if (models.size() - 1 == 5) { //Joker
		models[models.size() - 1].scaleT = 0.61;
		models[models.size() - 1].ejeX = -0.71;
		models[models.size() - 1].ejeY = 0.30;
		models[models.size() - 1].ejeZ = -1.23;
		models[models.size() - 1].bias = 0.015;
	}
	else if (models.size() - 1 == 6) { //Spider-Man
		models[models.size() - 1].scaleT = 0.61;
		models[models.size() - 1].ejeX = 0.17;
		models[models.size() - 1].ejeY = 0.31;
		models[models.size() - 1].ejeZ = 1.18;
		models[models.size() - 1].bias = 0.02;
		models[models.size() - 1].rotacion[0] = 0.0;
		models[models.size() - 1].rotacion[1] = 1.0;
		models[models.size() - 1].rotacion[2] = 0.0;
		models[models.size() - 1].rotacion[3] = 0.0;
	}
	else if (models.size() - 1 == 7) { //Silver Surfer
		models[models.size() - 1].scaleT = 0.61;
		models[models.size() - 1].ejeX = -0.71;
		models[models.size() - 1].ejeY = 0.54;
		models[models.size() - 1].ejeZ = 1.18;
		models[models.size() - 1].bias = 0.069;
		models[models.size() - 1].rotacion[0] = 0.0;
		models[models.size() - 1].rotacion[1] = 1.0;
		models[models.size() - 1].rotacion[2] = 0.0;
		models[models.size() - 1].rotacion[3] = 0.0;
	}
	else if (models.size() - 1 == 8) { //Iron Man
		models[models.size() - 1].scaleT = 0.61;
		models[models.size() - 1].ejeX = 0.95;
		models[models.size() - 1].ejeY = 0.30;
		models[models.size() - 1].ejeZ = 1.18;
		models[models.size() - 1].bias = 0.001;
		models[models.size() - 1].rotacion[0] = 0.0;
		models[models.size() - 1].rotacion[1] = 1.0;
		models[models.size() - 1].rotacion[2] = 0.0;
		models[models.size() - 1].rotacion[3] = 0.0;
	}
	else if (models.size() - 1 == 9) { //Pared 1 (Ahora espejo)
		models[models.size() - 1].scaleT = 2.83;
		models[models.size() - 1].ejeX = 0.00;
		models[models.size() - 1].ejeY = -0.70; //-2.12 Piso
		models[models.size() - 1].ejeZ = -2.00;
		models[models.size() - 1].texturingNormal = 1;
	}
	else if (models.size() - 1 == 10) { //Pared 2
		models[models.size() - 1].scaleT = 2.95;
		models[models.size() - 1].ejeX = 0.00;
		models[models.size() - 1].ejeY = -1.63;
		models[models.size() - 1].ejeZ = 1.99;
		models[models.size() - 1].rotacion[0] = 0.0;
		models[models.size() - 1].rotacion[1] = 1.0;
		models[models.size() - 1].rotacion[2] = 0.0;
		models[models.size() - 1].rotacion[3] = 0.0;
		models[models.size() - 1].texturingNormal = 1;
	}
	else if (models.size() - 1 == 11) { //Pared 3
		models[models.size() - 1].scaleT = 3.04;
		models[models.size() - 1].ejeX = -1.95;
		models[models.size() - 1].ejeY = -1.69;
		models[models.size() - 1].ejeZ = -0.02;
		models[models.size() - 1].rotacion[0] = 0.0;
		models[models.size() - 1].rotacion[1] = 0.70;
		models[models.size() - 1].rotacion[2] = 0.0;
		models[models.size() - 1].rotacion[3] = 0.70;
		models[models.size() - 1].texturingNormal = 1;
	}
	else if (models.size() - 1 == 12) { //Pared 4
		models[models.size() - 1].scaleT = 3.09;
		models[models.size() - 1].ejeX = 1.96;
		models[models.size() - 1].ejeY = -1.73;
		models[models.size() - 1].ejeZ = 0.00;
		models[models.size() - 1].rotacion[0] = 0.0;
		models[models.size() - 1].rotacion[1] = 0.70;
		models[models.size() - 1].rotacion[2] = 0.0;
		models[models.size() - 1].rotacion[3] = -0.70;
		models[models.size() - 1].texturingNormal = 1;
	}
	else if (models.size() - 1 == 13) {	//Cuadro
		models[models.size() - 1].scaleT = 0.37;
		models[models.size() - 1].ejeX = 1.84;
		models[models.size() - 1].ejeY = 0.37;
		models[models.size() - 1].ejeZ = -1.44;
		models[models.size() - 1].rotacion[0] = 0.55;
		models[models.size() - 1].rotacion[1] = 0.44;
		models[models.size() - 1].rotacion[2] = 0.54;
		models[models.size() - 1].rotacion[3] = -0.45;
		models[models.size() - 1].texturingLight = 1;
	}
	else if (models.size() - 1 == 14) {	//Mario
		models[models.size() - 1].scaleT = 0.52;
		models[models.size() - 1].ejeX = -1.44;
		models[models.size() - 1].ejeY = 0.28;
		models[models.size() - 1].ejeZ = 0.09;
		models[models.size() - 1].bias = 0.078;
		models[models.size() - 1].rotacion[0] = 0.0;
		models[models.size() - 1].rotacion[1] = 0.70;
		models[models.size() - 1].rotacion[2] = 0.0;
		models[models.size() - 1].rotacion[3] = 0.70;
	}
	else if (models.size() - 1 == 15) {	//Luigi
		models[models.size() - 1].scaleT = 0.62;
		models[models.size() - 1].ejeX = -1.42;
		models[models.size() - 1].ejeY = 0.33;
		models[models.size() - 1].ejeZ = -0.36;
		models[models.size() - 1].bias = 0.078;
		models[models.size() - 1].rotacion[0] = 0.0;
		models[models.size() - 1].rotacion[1] = 0.70;
		models[models.size() - 1].rotacion[2] = 0.0;
		models[models.size() - 1].rotacion[3] = 0.70;
	}
	else if (models.size() - 1 == 16) {	//Moto
		models[models.size() - 1].scaleT = 0.49;
		models[models.size() - 1].ejeX = 1.34;
		models[models.size() - 1].ejeY = 0.17;
		models[models.size() - 1].ejeZ = 0.00;
		models[models.size() - 1].bias = 0.025;
	}
	else if (models.size() - 1 == 17) {	//Planta 1
		models[models.size() - 1].scaleT = 1;
		models[models.size() - 1].ejeX = -1.39;
		models[models.size() - 1].ejeY = 0.48;
		models[models.size() - 1].ejeZ = -1.49;
		models[models.size() - 1].rotacion[0] = 0.0;
		models[models.size() - 1].rotacion[1] = -0.92;
		models[models.size() - 1].rotacion[2] = 0.0;
		models[models.size() - 1].rotacion[3] = 0.38;
		models[models.size() - 1].bias = 0.150;
	}
	else if (models.size() - 1 == 18) {	//Planta 2
		models[models.size() - 1].scaleT = 1;
		models[models.size() - 1].ejeX = -1.39;
		models[models.size() - 1].ejeY = 0.48;
		models[models.size() - 1].ejeZ = 1.55;
		models[models.size() - 1].rotacion[0] = 0.0;
		models[models.size() - 1].rotacion[1] = -0.92;
		models[models.size() - 1].rotacion[2] = 0.0;
		models[models.size() - 1].rotacion[3] = 0.38;
		models[models.size() - 1].bias = 0.150;
	}
	else if (models.size() - 1 == 19) {	//Planta 3
		models[models.size() - 1].scaleT = 1;
		models[models.size() - 1].ejeX = 1.41;
		models[models.size() - 1].ejeY = 0.48;
		models[models.size() - 1].ejeZ = 1.55;
		models[models.size() - 1].rotacion[0] = 0.0;
		models[models.size() - 1].rotacion[1] = -0.92;
		models[models.size() - 1].rotacion[2] = 0.0;
		models[models.size() - 1].rotacion[3] = 0.38;
		models[models.size() - 1].bias = 0.150;
	}
	else if (models.size() - 1 == 20) {	//Objeto de luz
		models[models.size() - 1].scaleT = 0.05;
	}

	file.close();
	initVBO(models.size());
}

//Cargar textura
unsigned int model::LoadTexture(const char* filename, bool text) {
	ilInit();
	ILuint imageID;
	GLuint textureID;
	ILboolean success;
	ILenum error;
	ilGenImages(1, &imageID);
	ilBindImage(imageID);
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

	success = ilLoadImage(filename);
	if (success) {
		success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
		if (!success) {
			error = ilGetError();
			cout << "Image conversion fails" << endl;
		}
		glGenTextures(1, &textureID);
		if (text)
			glActiveTexture(GL_TEXTURE0);
		else
			glActiveTexture(GL_TEXTURE1);

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexImage2D(GL_TEXTURE_2D,
			0,
			ilGetInteger(IL_IMAGE_FORMAT),
			ilGetInteger(IL_IMAGE_WIDTH),
			ilGetInteger(IL_IMAGE_HEIGHT),
			0,
			ilGetInteger(IL_IMAGE_FORMAT),
			GL_UNSIGNED_BYTE,
			ilGetData()
		);
		imageWidth = ilGetInteger(IL_IMAGE_WIDTH);
		imageHeight = ilGetInteger(IL_IMAGE_HEIGHT);
		imageFormat = ilGetInteger(IL_IMAGE_FORMAT);
	}

	ilDeleteImages(1, &imageID);
	return textureID;
}

//Para la autorotación
int model::getTimeMs() {
	//Para obtener el tiempo de rotación
	#if !defined(_WIN32)
		return (float)glfwGetTime();
	#else
		return (int)GetTickCount();
	#endif
}

//Para la rotación (autorotación)
void model::setQuat(const float *eje, float angulo, float *quat) {
	float sina2, norm;
	sina2 = (float)sin(0.5f * angulo);
	norm = (float)sqrt(eje[0] * eje[0] + eje[1] * eje[1] + eje[2] * eje[2]);
	quat[0] = sina2 * eje[0] / norm;
	quat[1] = sina2 * eje[1] / norm;
	quat[2] = sina2 * eje[2] / norm;
	quat[3] = (float)cos(0.5f * angulo);
}

//Para la rotación (autorotación)
void model::multiplicarQuat(const float *q1, const float *q2, float *qout) {
	float qr[4];
	qr[0] = q1[3] * q2[0] + q1[0] * q2[3] + q1[1] * q2[2] - q1[2] * q2[1];
	qr[1] = q1[3] * q2[1] + q1[1] * q2[3] + q1[2] * q2[0] - q1[0] * q2[2];
	qr[2] = q1[3] * q2[2] + q1[2] * q2[3] + q1[0] * q2[1] - q1[1] * q2[0];
	qr[3] = q1[3] * q2[3] - (q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2]);
	qout[0] = qr[0]; qout[1] = qr[1]; qout[2] = qr[2]; qout[3] = qr[3];
}

//Para crear la matriz de escalamiento
glm::mat4 model::scale_en_matriz(float scale_tx) {
	glm::mat4 scaleMatrix = glm::mat4(glm::vec4(scale_tx, 0.0, 0.0, 0.0), glm::vec4(0.0, scale_tx, 0.0, 0.0), glm::vec4(0.0, 0.0, scale_tx, 0.0), glm::vec4(0.0, 0.0, 0.0, 1)); //Creo matriz de escalamiento
	return scaleMatrix;
}

//Para crear la matriz de translate
glm::mat4 model::translate_en_matriz(float translate_tx, float translate_ty, float translate_tz) {
	glm::mat4 translateMatrix = glm::mat4(glm::vec4(1.0, 0.0, 0.0, 0.0), glm::vec4(0.0, 1.0, 0.0, 0.0), glm::vec4(0.0, 0.0, 1.0, 0.0), glm::vec4(translate_tx, translate_ty, translate_tz, 1)); //Creo matriz de translate
	return translateMatrix;
}

//Para crear la matriz de rotación
glm::mat4 model::rotacion_en_matriz(float rotacion_tx, float rotacion_ty, float rotacion_tz, float rotacion_ta) {
	glm::mat4 rotateMatrix;
	//Creo matriz de rotación usando los quat
	rotateMatrix[0][0] = 1.0 - 2.0 * (rotacion_ty * rotacion_ty + rotacion_tz * rotacion_tz);
	rotateMatrix[0][1] = 2.0 * (rotacion_tx * rotacion_ty + rotacion_tz * rotacion_ta);
	rotateMatrix[0][2] = 2.0 * (rotacion_tx * rotacion_tz - rotacion_ty * rotacion_ta);
	rotateMatrix[0][3] = 0.0;
	rotateMatrix[1][0] = 2.0 * (rotacion_tx * rotacion_ty - rotacion_tz * rotacion_ta);
	rotateMatrix[1][1] = 1.0 - 2.0 * (rotacion_tx * rotacion_tx + rotacion_tz * rotacion_tz);
	rotateMatrix[1][2] = 2.0 * (rotacion_ty * rotacion_tz + rotacion_tx * rotacion_ta);
	rotateMatrix[1][3] = 0.0;
	rotateMatrix[2][0] = 2.0 * (rotacion_tx * rotacion_tz + rotacion_ty * rotacion_ta);
	rotateMatrix[2][1] = 2.0 * (rotacion_ty * rotacion_tz - rotacion_tx * rotacion_ta);
	rotateMatrix[2][2] = 1.0 - 2.0 * (rotacion_tx * rotacion_tx + rotacion_ty * rotacion_ty);
	rotateMatrix[2][3] = 0.0;
	rotateMatrix[3][0] = 0.0;
	rotateMatrix[3][1] = 0.0;
	rotateMatrix[3][2] = 0.0;
	rotateMatrix[3][3] = 1.0;
	return rotateMatrix;
}

//Impresión de pantalla y lectura de shaders
bool model::initGlew() {

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
		return false;
	else {
		std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
		std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
		std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
		std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

		glslShadowProgram.loadShader("Shaders/shadow.vert", CGLSLProgram::VERTEX);
		glslShadowProgram.loadShader("Shaders/shadow.frag", CGLSLProgram::FRAGMENT);

		glslShadowProgram.create_link();

		glslShadowProgram.enable();

		glslShadowProgram.addAttribute("position");
		glslShadowProgram.addAttribute("coordTex");
		glslShadowProgram.addAttribute("normal");
		glslShadowProgram.addAttribute("tangentes");
		
		glslShadowProgram.addUniform("light_matrix");
		glslShadowProgram.addUniform("model");

		glslShadowProgram.disable();

		glslDepthProgram.loadShader("Shaders/depth.vert", CGLSLProgram::VERTEX);
		glslDepthProgram.loadShader("Shaders/depth.frag", CGLSLProgram::FRAGMENT);

		glslDepthProgram.create_link();

		glslDepthProgram.enable();

		glslDepthProgram.addAttribute("position");
		glslDepthProgram.addAttribute("coordTex");

		glslDepthProgram.addUniform("model");

		glslDepthProgram.disable();

		glslSkyboxProgram.loadShader("Shaders/skybox.vert", CGLSLProgram::VERTEX);
		glslSkyboxProgram.loadShader("Shaders/skybox.frag", CGLSLProgram::FRAGMENT);

		glslSkyboxProgram.create_link();

		glslSkyboxProgram.enable();

		glslSkyboxProgram.addAttribute("position");

		glslSkyboxProgram.addUniform("projection");
		glslSkyboxProgram.addUniform("view");
		glslSkyboxProgram.addUniform("skybox");

		glslSkyboxProgram.disable();


		glslProgram.loadShader("Shaders/program.vert", CGLSLProgram::VERTEX);
		glslProgram.loadShader("Shaders/program.frag", CGLSLProgram::FRAGMENT);

		glslProgram.create_link();

		glslProgram.enable();

		glslProgram.addAttribute("position");
		glslProgram.addAttribute("coord_texturas");
		glslProgram.addAttribute("normal");
		glslProgram.addAttribute("tangents");

		glslProgram.addUniform("view_matrix");
		glslProgram.addUniform("projection_matrix");
		glslProgram.addUniform("model_matrix");
		glslProgram.addUniform("reflection_matrix");
		glslProgram.addUniform("planar_matrix");
		glslProgram.addUniform("light_matrix");		
		glslProgram.addUniform("lightPos");
		glslProgram.addUniform("view");
		glslProgram.addUniform("shinyBlinn");
		glslProgram.addUniform("bias");
		glslProgram.addUniform("modelNumber");
		glslProgram.addUniform("interpol");
		glslProgram.addUniform("tipo_difuso");
		glslProgram.addUniform("tipo_especular");
		glslProgram.addUniform("lightSpotDir");
		glslProgram.addUniform("lightType");
		glslProgram.addUniform("lightning");
		glslProgram.addUniform("texture");
		glslProgram.addUniform("textureNormal");
		glslProgram.addUniform("textureLight");
		glslProgram.addUniform("shadowing");
		glslProgram.addUniform("shadowingLightMap");
		glslProgram.addUniform("skybox");
		glslProgram.addUniform("shadowMap");
		glslProgram.addUniform("bb");
		glslProgram.addUniform("texturing");
		glslProgram.addUniform("texturingNormal");
		glslProgram.addUniform("texturingLight");		
		glslProgram.addUniform("constant");
		glslProgram.addUniform("linear");
		glslProgram.addUniform("quadratic");
		glslProgram.addUniform("inCos");
		glslProgram.addUniform("outCos");
		glslProgram.addUniform("refractor");

		glslProgram.addUniform("color_material_ambiental");
		glslProgram.addUniform("color_material_difuso");
		glslProgram.addUniform("color_material_especular");

		glslProgram.addUniform("color_luz_ambiental");
		glslProgram.addUniform("color_luz_difuso");
		glslProgram.addUniform("color_luz_especular");

		glslProgram.disable();

		return true;
	}

}