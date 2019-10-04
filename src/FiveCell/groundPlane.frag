#version 410

const float GAMMA = 2.2;

//uniform vec3 lightPos;
//uniform vec3 light2Pos;
//uniform vec3 camPos;
uniform sampler2D groundTex;

//in vec3 fragPos_worldSpace;
//in vec3 normal_worldSpace;
in vec2 texCoordOut;

out vec4 colour_out;

void main(){

	//float specularStrength = 0.2;
	//vec3 lightColour = vec3(1.0, 1.0, 1.0);
	//
	////*** Ambient ***//
	//vec3 objectColour = vec3(0.75, 0.8125, 0.95);
	//float ambientStrength = 0.3;

	//vec3 ambient = ambientStrength * lightColour;

	////*** Diffuse ***//
	//vec3 norm = normalize(normal_worldSpace);
	//vec3 lightDir_worldSpace = normalize(lightPos- fragPos_worldSpace);
	//vec3 light2Dir_worldSpace = normalize(light2Pos - fragPos_worldSpace);
	//float diffuseAngle = max(dot(norm, lightDir_worldSpace), 0.0);
	//float diffuseAngle2 = max(dot(norm, light2Dir_worldSpace), 0.0);
	//vec3 diffuse = diffuseAngle * lightColour;
	//vec3 diffuse2 = diffuseAngle2 * lightColour;

	////*** Specular ***//
	//vec3 viewDir = normalize(camPos - fragPos_worldSpace);
	//vec3 halfWay = normalize(lightDir_worldSpace + viewDir);
	//vec3 halfWay2 = normalize(light2Dir_worldSpace + viewDir);
	//float specAngle = pow(max(dot(norm, halfWay), 0.0), 16.0);
	//float specAngle2 = pow(max(dot(norm, halfWay2), 0.0), 16.0);
	//vec3 specular = specularStrength * specAngle * lightColour;
	//vec3 specular2 = specularStrength * specAngle2 * lightColour;

	//vec3 result = (ambient + diffuse + specular) * objectColour;
	//vec3 result2 = (ambient + diffuse2 + specular2) * objectColour;

	//colour_out = texture(groundTexture, texCoordOut) * vec4(result + result2, 0.0);

	//vec4 colPreGamma = texture(groundTex, texCoordOut);
	//
	////gamma correction
	//vec3 colWithGamma = pow(colPreGamma.rgb, vec3(1.0 / GAMMA));

    	//colour_out = vec4(colWithGamma, 1.0);

	colour_out = texture(groundTex, texCoordOut);
} 
