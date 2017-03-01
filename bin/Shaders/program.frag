#version 330
#extension GL_NV_shadow_samplers_cube : enable

uniform vec3 view;
uniform int modelNumber; 
uniform int lightType;
uniform vec3 lightSpotDir;
uniform bool lightning;
uniform bool texturing;
uniform bool texturingNormal;
uniform bool texturingLight;
uniform bool shadowing;
uniform bool shadowingLightMap;
uniform bool bb;
uniform float shinyBlinn;
uniform float constant;
uniform float linear;
uniform float quadratic;
uniform float inCos;
uniform float outCos;
uniform float refractor;
uniform float bias;

uniform vec4 color_material_ambiental;
uniform vec4 color_material_difuso;
uniform vec4 color_material_especular;

uniform vec4 color_luz_ambiental;
uniform vec4 color_luz_difuso;
uniform vec4 color_luz_especular;

uniform sampler2D texture;
uniform sampler2D textureNormal;
uniform sampler2D shadowMap;
uniform samplerCube skybox;
uniform sampler2D textureLight;

in vec3 normales;
in vec3 lightPosfrag;
in vec3 FragPos;
in vec3 viewPos; 
in vec4 colorVertex; 
in vec2 coordTex;
in vec4 FragPosLightSpace;
in mat3 TBN;

out vec4 color; //Color final

float calcularSombras(vec4 fragPosLightSpace) {

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture2D(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;

    vec3 norm = normalize(normales);

	if (texturingNormal) {

		norm = texture2D(textureNormal, coordTex).xyz;
        norm = normalize(norm * 2.0 - 1.0);          
		norm = normalize(TBN * norm);
    }

    vec3 normal = normalize(norm);
    vec3 lightDir = normalize(lightPosfrag - FragPos);
    float biasVal = max(0.05 * (1.0 - dot(normal, lightDir)), bias);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture2D(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            if (currentDepth - biasVal > pcfDepth) shadow += 1.0;
            else shadow += 0.0;        
        }    
    }
    shadow /= 9.0;
    
   	if (projCoords.z > 1.0) shadow = 0.0;
        
    return shadow;

}


void main() {	

	if (!lightning) {
		
		if (modelNumber != 1 && modelNumber != 2 && modelNumber != 9) color = vec4(0.0, 0.0, 0.0, 0.0);
		else {

			//A los objetos con lightmap no les afecta la luz
			if (modelNumber == 1 || modelNumber == 13) {
				if (bb) color = vec4(0.0, 0.0, 0.0, 1.0);
				else {

					if (!shadowingLightMap) {

						if (texturingLight)	color = texture2D(texture, coordTex) * texture2D(textureLight, coordTex);
						else color = texture2D(texture, coordTex);

					} else color = vec4(0.0, 0.0, 0.0, 1.0);

				}
			}

			//Al objeto refractante no le afecta la luz
			if (modelNumber == 2) {
				if (bb) color = vec4(0.0, 0.0, 0.0, 1.0);
				else {

					float ratio = 1.00 / refractor;
				    vec3 I = normalize(FragPos - view);
				    vec3 R = refract(I, normalize(normales), ratio);
				    color = textureCube(skybox, R);

				}
			}

			//Al modelo reflectante no le afecta la luz, se refleja el skybox de igual manera
			if (modelNumber == 9) {
				if (bb) color = vec4(0.0, 0.0, 0.0, 1.0);
				else {

					vec3 I = normalize(FragPos - view);
				    vec3 R = reflect(I, normalize(normales));
				    //Se refleja el skybox
				    color = textureCube(skybox, R);

				}

			}

		}		

	} else {

		vec4 result = vec4(0.0, 0.0, 0.0, 0.0);

		vec3 norm = normalize(normales);

		if (texturingNormal) {

			norm = texture2D(textureNormal, coordTex).xyz;
	        norm = normalize(norm * 2.0 - 1.0);          
			norm = normalize(TBN * norm);
        }

		vec3 lightDir = normalize(lightPosfrag - FragPos);
	
		//Reflección Ambiental
		
		vec4 ambient = color_material_ambiental;

		//Reflección Difusa

		vec4 diffuse = vec4(0.0, 0.0, 0.0, 0.0);

		//Lambert		
		float diff = max(dot(norm, lightDir), 0.0);
		diffuse = vec4(diff,diff,diff,1.0);

		//Reflección Especular

		vec4 specular = vec4(0.0, 0.0, 0.0, 0.0);
		float intensity = 0.0;

		//Blinn-Phong
		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(norm, halfwayDir), 0.0), shinyBlinn);
		specular = vec4(spec,spec,spec,1.0);


		if (lightType == 1) {	//Luz Direccional

			//No se modifican los parámetros
						
		} else if (lightType == 2) {	//Luz puntual 

			//Atenuación
			float distance = length(lightPosfrag - FragPos);
			float attenuation = 1.0f / (constant + linear * distance + quadratic * (distance * distance));    
						
			ambient  *= attenuation;  
			diffuse  *= attenuation;
			specular *= attenuation;  

		} else if (lightType == 3) {	//Luz reflector
						
			float theta = dot(lightDir, normalize(-lightSpotDir)); 
			float epsilon = (inCos - outCos);
			intensity = clamp((theta - outCos) / epsilon, 0.0, 1.0);

			ambient  *= intensity;
			diffuse  *= intensity;
			specular *= intensity;

			//Atenuación
			float distance = length(lightPosfrag - FragPos);
			distance = 5.0/(5.0 - 0.0) + 5.0/(0.0 - 5.0)/distance; 
			float attenuation = 1.0f / (constant + linear * distance + quadratic * (distance * distance));    

			ambient  *= attenuation;
			diffuse  *= attenuation;
			specular *= attenuation;
		}	  
		    
		//Resultado final

		//Validar si quiero hacer shadows por modelo
		float shadow = 0.0;
		if (shadowing && (lightType == 1 || lightType == 3)) shadow = calcularSombras(FragPosLightSpace);    
		result = (color_luz_ambiental * ambient + (1.0 - shadow) * (color_luz_difuso * color_material_difuso * diffuse + color_luz_especular * color_material_especular * specular));
		if (texturing) result *= texture2D(texture, coordTex);
		if (bb) color = vec4(0.0, 0.0, 0.0, 1.0);
    	else color =  result;

		if (modelNumber == 20) {
			
			if (bb) color = vec4(1.0, 1.0, 1.0, 1.0);
			else {

				if (lightType == 1 || lightType == 2) color = color_luz_especular * color_luz_difuso;
				else color = color_luz_especular * color_luz_difuso * intensity ;

			}

		}

		if (modelNumber == 1 || modelNumber == 13) {

			if (bb) color = vec4(0.0, 0.0, 0.0, 1.0);
			else {

				if (!shadowingLightMap) {

					if (texturingLight)	color = texture2D(texture, coordTex) * texture2D(textureLight, coordTex);
					else color = texture2D(texture, coordTex);

				} else color = vec4(0.0, 0.0, 0.0, 1.0);

			}

		}


		if (modelNumber == 2) {

			if (bb) color = vec4(0.0, 0.0, 0.0, 1.0);
			else {

				float ratio = 1.00 / refractor;
			    vec3 I = normalize(FragPos - view);
			    vec3 R = refract(I, normalize(normales), ratio);
			    color = textureCube(skybox, R);

			}

		}

		if (modelNumber == 9) {

			if (bb) color = vec4(0.0, 0.0, 0.0, 1.0);
			else {

			    vec3 I = normalize(FragPos - view);
			    vec3 R = reflect(I, normalize(normales));
			    //Se refleja el skybox
			    color = textureCube(skybox, R);

			}

		}

	}	

}