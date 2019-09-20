#include "FiveCell.hpp"

#include <cstdio>
#include <cstdarg>
#include <random>
#include <ctime>
#include <assert.h>
#include <math.h>
#include <cmath>
#include <iostream>

#include "stb_image.h"

#ifdef __APPLE__ 
#include "GLFW/glfw3.h"
#elif _WIN32 
#include "glfw3.h"
#endif

//#include "log.h"
#include "ShaderManager.hpp"
//#include "utils.h"

#define PI 3.14159265359

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif

bool FiveCell::setup(std::string csd, GLuint skyboxProg, GLuint soundObjProg, GLuint groundPlaneProg, GLuint fiveCellProg, GLuint quadShaderProg){

//************************************************************
//Csound performance thread
//************************************************************
	std::string csdName = "";
	if(!csd.empty()) csdName = csd;
	session = new CsoundSession(csdName);

#ifdef _WIN32
	session->SetOption("-b -32"); 
	session->SetOption("-B 2048");
#endif

	session->StartThread();
	session->PlayScore();

	std::string val1 = "azimuth";
	const char* azimuth = val1.c_str();	
	if(session->GetChannelPtr(hrtfVals[0], azimuth, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0){
		std::cout << "GetChannelPtr could not get the azimuth input" << std::endl;
		return false;
	}
	std::string val2 = "elevation";
	const char* elevation = val2.c_str();
	if(session->GetChannelPtr(hrtfVals[1], elevation, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0){
		std::cout << "GetChannelPtr could not get the elevation input" << std::endl;
		return false;
	}	
	std::string val3 = "distance";
	const char* distance = val3.c_str();
	if(session->GetChannelPtr(hrtfVals[2], distance, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0){
		std::cout << "GetChannelPtr could not get the distance input" << std::endl;
		return false;
	}
	const char* randFreq = "randFreq";
	if(session->GetChannelPtr(randomFrequencyVal, randFreq, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0){
		std::cout << "GetChannelPtr could not get the randFreq value" << std::endl;
		return false;
	} 

//********* output values from csound to avr *******************//

	const char* rmsOut = "rmsOut";
	if(session->GetChannelPtr(m_pRmsOut, rmsOut, CSOUND_OUTPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0){
		std::cout << "Csound output value rmsOut not available" << std::endl;
		return false;
	}

	//const char* vert0 = "vert0";
	//if(session->GetChannelPtr(vert0Vol, vert0, CSOUND_OUTPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0){
	//	std::cout << "Csound output value vert0Vol not available" << std::endl;
	//	return false;
	//} 

	//const char* vert1 = "vert1";
	//if(session->GetChannelPtr(vert1Vol, vert1, CSOUND_OUTPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0){
	//	std::cout << "Csound output value vert1Vol not available" << std::endl;
	//	return false;
	//}

	//const char* vert2 = "vert2";
	//if(session->GetChannelPtr(vert2Vol, vert2, CSOUND_OUTPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0){
	//	std::cout << "Csound output value vert2Vol not available" << std::endl;
	//	return false;
	//}

	//const char* vert3 = "vert3";
	//if(session->GetChannelPtr(vert3Vol, vert3, CSOUND_OUTPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0){
	//	std::cout << "Csound output value vert3Vol not available" << std::endl;
	//	return false;
	//}

	//const char* vert4 = "vert4";
	//if(session->GetChannelPtr(vert4Vol, vert4, CSOUND_OUTPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0){
	//	std::cout << "Csound output value vert4Vol not available" << std::endl;
	//	return false;
	//}
//**********************************************************

//*********************************************************************************************
// Machine Learning
//********************************************************************************************

	m_bPrevSaveState = false;
	m_bPrevRandomState = false;
	m_bPrevTrainState = false;
	m_bPrevHaltState = false;
	m_bPrevLoadState = false;

//********************************************************************************************

	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//******************************************************************************************
// Matrices & Light Positions
//*******************************************************************************************
	
	//glm::mat4 projectionMatrix;
	//glm::mat4 viewMatrix;

	// projection matrix setup	
	//projectionMatrix = glm::perspective(45.0f, (float)g_gl_width / (float)g_gl_height, 0.1f, 1000.0f);

	// variables for view matrix
	//cameraPos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	//cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	//cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	//model matrix
	modelMatrix = glm::mat4(1.0f);

	lightPos = glm::vec3(-2.0f, -1.0f, -1.5f); 
	light2Pos = glm::vec3(1.0f, 40.0f, 1.5f);
//****************************************************************************************************


//***************************************************************************************************
// SoundObject 
//**************************************************************************************************
	//for(int i = 0; i < _countof(soundObjects); i++){

	//for(int i = 0; i < 5; i++){
	//	if(!soundObjects[i].setup(soundObjProg)){
	//		std::cout << "ERROR: SoundObject " << std::to_string(i) << " init failed" << std::endl;
	//		return false;
	//	}
	//}
	
//*************************************************************************************************
			
//***********************************************************************************************
// Quad to test texture rendering
//***********************************************************************************************

	//vertices
	//float quadVerts [20] = {
	//	//positions		//texCoords
	//	-1.0f, 1.0f, 0.0f,	-1.0f, 1.0f,
	//	-1.0f, -1.0f, 0.0f,	-1.0f, -1.0f,
	//	1.0f, -1.0f, 0.0f,	1.0f, -1.0f,
	//	1.0f, 1.0f, 0.0f,	1.0f, 1.0f
	//};
	
	//float quadVerts [12] = {
	//	//positions		
	//	-1.0f, 1.0f, 0.0f,		
	//	-1.0f, -1.0f, 0.0f,	
	//	1.0f, -1.0f, 0.0f,	
	//	1.0f, 1.0f, 0.0f	
	//};

	////indices
	//unsigned int quadIndices [6] = {
	//	0, 1, 2,
	//	0, 2, 3
	//};
	
	//float quadVerts [24] = {
	//	//top left front	
	//	-1.0f, 1.0f, 1.0f,
	//	//bottom left front
	//	-1.0f, -1.0f, 1.0f,
	//	//bottom right front
	//	1.0f, -1.0f, 1.0f,
	//	//top right front
	//	1.0f, 1.0f, 1.0f,
	//	//top left back
	//	-1.0f, 1.0f, -1.0f,
	//	//bottom left back
	//	-1.0f, -1.0f, -1.0f,
	//	//bottom right back
	//	1.0f, -1.0f, -1.0f,
	//	//top right back
	//	1.0f, 1.0f, -1.0f
	//};

	//unsigned int quadIndices [36] = {
	//	//front face
	//	0, 1, 2,
	//	0, 2, 3,
	//	//right face
	//	3, 2, 6,
	//	3, 6, 7,
	//	//back face
	//	7, 6, 5,
	//	7, 5, 4,
	//	//left face
	//	4, 5, 1,
	//	4, 1, 0,
	//	//bottom face
	//	1, 5, 6,
	//	1, 6, 2,
	//	//top face
	//	4, 0, 3,
	//	4, 3, 7
	//};

	////float quadNormals [24] = {
	////	//top front left
	////	-1.0f, 1.0f, 1.0f,
	////	//bottom front left
	////	-1.0f, -1.0f, 1.0f,
	////	//bottom front right
	////	1.0f, -1.0f, 1.0f,
	////	//top front right
	////	1.0f, 1.0f, 1.0f,
	////	//top left back
	////	-1.0f, 1.0f, -1.0f,
	////	//bottom left back
	////	-1.0f, -1.0f, -1.0f,
	////	//bottom right back
	////	1.0f, -1.0f, -1.0f,
	////	//top right back
	////	1.0f, 1.0f, -1.0f
	////};

	////Set up ground plane buffers
	//glGenVertexArrays(1, &quadVAO);
	//glBindVertexArray(quadVAO);

	//GLuint quadVBO;
	////glGenBuffers(1, &quadVBO);
	////glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	////glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(float), quadVerts, GL_STATIC_DRAW);

	////glEnableVertexAttribArray(0);
	////glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(0));
	////glEnableVertexAttribArray(1);
	////glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	////glBindBuffer(GL_ARRAY_BUFFER, 0);
	//
	//glGenBuffers(1, &quadVBO);
	//glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	//glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), quadVerts, GL_STATIC_DRAW);

	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	////GLuint quadNormalVBO;
	////glGenBuffers(1, &quadNormalVBO);
	////glBindBuffer(GL_ARRAY_BUFFER, quadNormalVBO);
	////glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), quadNormals, GL_STATIC_DRAW);
	////
	////glEnableVertexAttribArray(1);
	////glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	////glBindBuffer(GL_ARRAY_BUFFER, 0);

	//glGenTextures(1, &quadTexID);
	//glBindTexture(GL_TEXTURE_2D, quadTexID);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//
	////load texture
	//int texWidth;
	//int texHeight;
	//int texChnls;
	//unsigned char* texData;
        //texData	= stbi_load("misty_ft.tga", &texWidth, &texHeight, &texChnls, 0);
	//if(texData){
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
	//	//glGenerateMipmap(GL_TEXTURE_2D);
	//} else {
	//	std::cout << "Failed to load quad texture" << std::endl;
	//}
	//stbi_image_free(texData);

	//glGenBuffers(1, &quadIndexBuffer);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIndexBuffer);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(unsigned int), quadIndices, GL_STATIC_DRAW);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//
	////uniform setup
	//quad_projMatLoc = glGetUniformLocation(quadShaderProg, "projMat");
	//quad_viewMatLoc = glGetUniformLocation(quadShaderProg, "viewMat");
	//quad_modelMatLoc = glGetUniformLocation(quadShaderProg, "quadModelMat");

	////quad_lightPosLoc = glGetUniformLocation(quadShaderProg, "lightPos");
	////quad_light2PosLoc = glGetUniformLocation(quadShaderProg, "light2Pos");

	////quad_cameraPosLoc = glGetUniformLocation(quadShaderProg, "camPos");
	//
	//glBindVertexArray(0);

	////glm::vec3 scaleQuad = glm::vec3(20.0f, 20.0f, 0.0f);
	////glm::mat4 scaleQuadMatrix = glm::scale(modelMatrix, scaleQuad);

	//quadModelMatrix = modelMatrix;


//***************************************************************************************************
// Skybox
//**************************************************************************************************

	//bool result = skybox.setup();
	//if(!result){
	//	std::cout << "ERROR: Skybox init failed" << std::endl;
	//	return false;
	//}
	
	//Skybox vertices
	float skyboxVerts [24] = {
		//top left front	
		-1.0f, 1.0f, 1.0f,
		//bottom left front
		-1.0f, -1.0f, 1.0f,
		//bottom right front
		1.0f, -1.0f, 1.0f,
		//top right front
		1.0f, 1.0f, 1.0f,
		//top left back
		-1.0f, 1.0f, -1.0f,
		//bottom left back
		-1.0f, -1.0f, -1.0f,
		//bottom right back
		1.0f, -1.0f, -1.0f,
		//top right back
		1.0f, 1.0f, -1.0f
	};

	unsigned int skyboxIndices [36] = {
		//front face
		0, 1, 2,
		0, 2, 3,
		//right face
		3, 2, 6,
		3, 6, 7,
		//back face
		7, 6, 5,
		7, 5, 4,
		//left face
		4, 5, 1,
		4, 1, 0,
		//bottom face
		1, 5, 6,
		1, 6, 2,
		//top face
		4, 0, 3,
		4, 3, 7
	};

	//unsigned int skyboxIndices [36] = {
	//	//front face
	//	0, 3, 2,
	//	0, 2, 1,
	//	//right face
	//	3, 7, 6,
	//	3, 6, 2,
	//	//back face
	//	7, 4, 5,
	//	7, 5, 6,
	//	//left face
	//	4, 0, 1,
	//	4, 1, 5,
	//	//bottom face
	//	1, 2, 6,
	//	1, 6, 5,
	//	//top face
	//	4, 7, 3,
	//	4, 3, 0
	//};

	//float skyboxNormals [24] = {
	//	//top front left
	//	-1.0f, 1.0f, 1.0f,
	//	//bottom front left
	//	-1.0f, -1.0f, 1.0f,
	//	//bottom front right
	//	1.0f, -1.0f, 1.0f,
	//	//top front right
	//	1.0f, 1.0f, 1.0f,
	//	//top left back
	//	-1.0f, 1.0f, -1.0f,
	//	//bottom left back
	//	-1.0f, -1.0f, -1.0f,
	//	//bottom right back
	//	1.0f, -1.0f, -1.0f,
	//	//top right back
	//	1.0f, 1.0f, -1.0f
	//};	

	//Set up skybox buffers
	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);

	GLuint skyboxVBO;
	glGenBuffers(1, &skyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), skyboxVerts, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//GLuint skyboxNormalVBO;
	//glGenBuffers(1, &skyboxNormalVBO);
	//glBindBuffer(GL_ARRAY_BUFFER, skyboxNormalVBO);
	//glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), skyboxNormals, GL_STATIC_DRAW);

	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	//setup texture buffer
	//glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &skyboxTexID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexID);

	//load textures
	std::vector<std::string> textureNames;
	textureNames.push_back("desert3_px.jpg");
	textureNames.push_back("desert3_nx.jpg");
	textureNames.push_back("desert3_py.jpg");
	textureNames.push_back("desert3_ny.jpg");
	textureNames.push_back("desert3_pz.jpg");
	textureNames.push_back("desert3_nz.jpg");

	//std::string name1 = texName.append("_rt.tga");
	//textureNames.push_back(name1);
	//std::string name2 = texName.append("_lf.tga");
	//textureNames.push_back(name2);
	//std::string name3 = texName.append("_up.tga");
	//textureNames.push_back(name3);
	//std::string name4 = texName.append("_dn.tga");
	//textureNames.push_back(name4);
	//std::string name5 = texName.append("_ft.tga");
	//textureNames.push_back(name5);
	//std::string name6 = texName.append("_bk.tga");
	//textureNames.push_back(name6);	

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  	

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, numChannels;
	unsigned char* data;
	for(GLuint i = 0; i < textureNames.size(); i++){
		std::cout << textureNames[i] << std::endl;
		data = stbi_load(textureNames[i].c_str(), &width, &height, &numChannels, 0);
		if(data){
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		} else {
			std::cout << "ERROR: Cubemap not loaded" << std::endl;
			//return false;	
		}

		stbi_image_free(data);	
	}			
	
	
	//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glGenBuffers(1, &skyboxIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(unsigned int), skyboxIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//uniform setup
	skybox_projMatLoc = glGetUniformLocation(skyboxProg, "projMat");
	skybox_viewMatLoc = glGetUniformLocation(skyboxProg, "viewMat");
	skybox_modelMatLoc = glGetUniformLocation(skyboxProg, "modelMat");

	//skybox_texUniformLoc = glGetUniformLocation(skyboxProg, "skybox");

	//only use during development as computationally expensive
	bool validProgram = is_valid(skyboxProg);
	if(!validProgram){
		fprintf(stderr, "ERROR: skyboxhaderProg not valid\n");
		return 1;
	}

	glBindVertexArray(0);

	skyboxModelMatrix = modelMatrix;

//*************************************************************************************************

//**************************************************************************************************
//	Ground Plane Setup
//*********************************************************************************************
	// Ground plane vertices
	float groundVerts [12] = {
		-1.0f, 0.0f, -1.0f,
		-1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, -1.0f
	};

	unsigned int groundIndices [6] = {
		0, 1, 2,
		0, 2, 3
	};

	float groundTexCoords [8] = {
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};

	//Set up ground plane buffers
	glGenVertexArrays(1, &groundVAO);
	glBindVertexArray(groundVAO);

	GLuint groundVBO;
	glGenBuffers(1, &groundVBO);
	glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), groundVerts, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint groundTexCoordVBO;
	glGenBuffers(1, &groundTexCoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, groundTexCoordVBO);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), groundTexCoords, GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenTextures(1, &groundTexture);
	glBindTexture(GL_TEXTURE_2D, groundTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int texWidth, texHeight, texChannels;
	unsigned char* groundTexData = stbi_load("desertFloor1_pow2.jpeg", &texWidth, &texHeight, &texChannels, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, groundTexData);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(groundTexData);
	//glBindTexture(GL_TEXTURE_2D, 0);

	glGenBuffers(1, &groundIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), groundIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	//uniform setup
	ground_projMatLoc = glGetUniformLocation(groundPlaneProg, "projMat");
	ground_viewMatLoc = glGetUniformLocation(groundPlaneProg, "viewMat");
	ground_modelMatLoc = glGetUniformLocation(groundPlaneProg, "groundModelMat");

	ground_lightPosLoc = glGetUniformLocation(groundPlaneProg, "lightPos");
	ground_light2PosLoc = glGetUniformLocation(groundPlaneProg, "light2Pos");

	ground_cameraPosLoc = glGetUniformLocation(groundPlaneProg, "camPos");
	
	glBindVertexArray(0);

	groundModelMatrix = modelMatrix;
	glm::vec3 yTranslation = glm::vec3(0.0f, -2.0f, 0.0f);
	glm::mat4 translationMatrix = glm::translate(groundModelMatrix, yTranslation);
	//glm::mat4 groundRotationMatrix = glm::rotate(groundModelMatrix, -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));  
	groundModelMatrix = translationMatrix;// * groundRotationMatrix;
//***************************************************************************************************		
		

//*************************************************************************************************
// 5cell Polytope Setup
//*************************************************************************************************
	/* specify 4D coordinates of 5-cell from https://en.wikipedia.org/wiki/5-cell */
	/*float vertices [20] = {
		1.0f/sqrt(10.0f), 1.0f/sqrt(6.0f), 1.0f/sqrt(3.0f), 1.0f,
		1.0f/sqrt(10.0f), 1.0f/sqrt(6.0f), 1.0f/sqrt(3.0f), -1.0f,
		1.0f/sqrt(10.0f), 1.0f/sqrt(6.0f), -2.0f/sqrt(3.0f), 0.0f,
		1.0f/sqrt(10.0f), -sqrt(3.0f/2.0f), 0.0f, 0.0f,
		-2.0f * sqrt(2.0f/5.0f), 0.0f, 0.0f, 0.0f
	};*/
	//float vertices5Cell [20] = {
	//	0.3162f, 0.4082f, 0.5774f, 1.0f,
	//	0.3162f, 0.4082f, 0.5774f, -1.0f,
	//	0.3162f, 0.4082f, -1.1547, 0.0f,
	//	0.3162f, -1.2247f, 0.0f, 0.0f,
	//	-1.2649f, 0.0f, 0.0f, 0.0f
	//};

	/* indices specifying 10 faces */
	//unsigned int indices [30] = {
	//	4, 2, 3,
	//	3, 0, 2,
	//	2, 0, 4,
	//	4, 0, 3,
	//	3, 1, 0,
	//	1, 4, 0,
	//	0, 1, 2,
	//	2, 3, 1,
	//	1, 3, 4,
	//	4, 2, 1	
	//};

	//alt indices
	//unsigned int indices [30] = {
	//	0, 2, 1,
	//	0, 3, 2,
	//	0, 4, 3,
	//	0, 2, 4,
	//	2, 3, 4,
	//	3, 1, 4,
	//	4, 1, 2,
	//	2, 1, 3,
	//	3, 1, 0,
	//	0, 1, 4
	//};
	
	//unsigned int lineIndices [20] = {
	//	4, 2,
	//	2, 3,
	//	3, 4,
	//	4, 0,
	//	0, 2, 
	//	0, 3,
	//	3, 1,
	//	1, 2,
	//	4, 1,
	//	1, 0
	//};

	////array of verts
	//glm::vec4 vertArray [5] = {
	//	glm::vec4(vertices5Cell[0], vertices5Cell[1], vertices5Cell[2], vertices5Cell[3]),
	//	glm::vec4(vertices5Cell[4], vertices5Cell[5], vertices5Cell[6], vertices5Cell[7]),
	//	glm::vec4(vertices5Cell[8], vertices5Cell[9], vertices5Cell[10], vertices5Cell[11]),
	//	glm::vec4(vertices5Cell[12], vertices5Cell[13], vertices5Cell[14], vertices5Cell[15]),
	//	glm::vec4(vertices5Cell[16], vertices5Cell[17], vertices5Cell[18], vertices5Cell[19])
	//};

	//for(int i = 0; i < _countof(vertArray); i++){
	//	vertArray5Cell[i] = vertArray [i];
	//}	

	////array of faces
	//glm::vec3 faceArray [10] = {
	//	glm::vec3(indices[0], indices[1], indices[2]),
	//	glm::vec3(indices[3], indices[4], indices[5]),
	//	glm::vec3(indices[6], indices[7], indices[8]),
	//	glm::vec3(indices[9], indices[10], indices[11]),
	//	glm::vec3(indices[12], indices[13], indices[14]),
	//	glm::vec3(indices[15], indices[16], indices[17]),
	//	glm::vec3(indices[18], indices[19], indices[20]),
	//	glm::vec3(indices[21], indices[22], indices[23]),
	//	glm::vec3(indices[24], indices[25], indices[26]),
	//	glm::vec3(indices[27], indices[28], indices[29]),
	//};

	//glm::vec4 faceNormalArray [10];
	//
	////calculate vertex normals in 4D to send to shaders for lighting
	//for(int i = 0; i < _countof(faceArray); i++){
	//	//calculate three linearly independent vectors for each face
	//	unsigned int indexA = faceArray[i].x;
	//	unsigned int indexB = faceArray[i].y;
	//	unsigned int indexC = faceArray[i].z;

	//	glm::vec4 vertA = vertArray[indexA];
	//	glm::vec4 vertB = vertArray[indexB];
	//	glm::vec4 vertC = vertArray[indexC];

	//	glm::vec4 vectorA = glm::vec4(vertB.x - vertA.x, vertB.y - vertA.y, vertB.z - vertA.z, vertB.w - vertA.w);
	//	glm::vec4 vectorB = glm::vec4(vertC.x - vertB.x, vertC.y - vertB.y, vertC.z - vertB.z, vertC.w - vertB.w);
	//	glm::vec4 vectorC = glm::vec4(vertA.x - vertC.x, vertA.y - vertC.y, vertA.z - vertC.z, vertA.w - vertC.w);

	//	//calculate orthonormal basis for vectorA, B and C using Gram-Schmidt. We can then calculte
	//	//the 4D normal
	//	glm::vec4 u1 = glm::normalize(vectorA);
	//	
	//	glm::vec4 y2 = vectorB - ((glm::dot(vectorB, u1)) * u1);
	//	glm::vec4 u2 = glm::normalize(y2);

	//	glm::vec4 y3 = vectorC - ((glm::dot(vectorC, u2)) * u2);
	//	glm::vec4 u3 = glm::normalize(y3);
	//	
	//	//calculate the  normal for each face
	// 	//using matrices and  Laplace expansion we can find the normal 
	//	//vector in 4D given three input vectors	
	//	//this procedure is following the article at https://ef.gy/linear-algebra:normal-vectors-in-higher-dimensional-spaces 
	//	/* a x b x c = 	| a0 b0 c0 right|
	//			| a1 b1 c1 up	|
	//			| a2 b2 c2 back	|	
	//			| a3 b3 c3 charm|*/
	//	glm::vec4 right = glm::vec4(1.0, 0.0, 0.0, 0.0);	
	//	glm::vec4 up = glm::vec4(0.0, 1.0, 0.0, 0.0);	
	//	glm::vec4 back = glm::vec4(0.0, 0.0, 1.0, 0.0);	
	//	glm::vec4 charm = glm::vec4(0.0, 0.0, 0.0, 1.0);	

	//	glm::mat3 matA = glm::mat3(	u1.y, u2.y, u3.y,
	//					u1.z, u2.z, u3.z,
	//					u1.w, u2.w, u3.w);

	//	glm::mat3 matB = glm::mat3(	u1.x, u2.x, u3.x,
	//					u1.z, u2.z, u3.z,
	//					u1.w, u2.w, u3.w);

	//	glm::mat3 matC = glm::mat3(	u1.x, u2.x, u3.x,
	//					u1.y, u2.y, u3.y,
	//					u1.w, u2.w, u3.w);
	//
	//	glm::mat3 matD = glm::mat3(	u1.x, u2.x, u3.x,
	//					u1.y, u2.y, u3.y,
	//					u1.z, u2.z, u3.z);	

	//	float determinantA = glm::determinant(matA);	
	//	float determinantB = glm::determinant(matB);	
	//	float determinantC = glm::determinant(matC);	
	//	float determinantD = glm::determinant(matD);	

	//	glm::vec4 termA = (determinantA * right) * -1.0f;
	//	glm::vec4 termB = determinantB * up;
	//	glm::vec4 termC = (determinantC * back) * -1.0f;
	//	glm::vec4 termD = determinantD * charm;

	//	glm::vec4 faceNormal = termA + termB + termC + termD;
	//	faceNormalArray[i] += faceNormal;
	//}
	//
	//

	//float vertexNormalArray [20];

	////calculate the normal for each vertex by taking the average of the normals of each adjacent face
	//for(int i = 0; i < _countof(vertArray); i++){
	//	glm::vec4 cumulativeNormals = glm::vec4(0.0);
	//	for(int j = 0; j < _countof(faceArray); j++){
	//			
	//		//does this face [j] contain vert [i]?
	//		unsigned int vertA = faceArray[j].x;				 
	//		unsigned int vertB = faceArray[j].y;
	//		unsigned int vertC = faceArray[j].z;
	//		if(vertA == i || vertB == i || vertC == i){
	//			cumulativeNormals += faceNormalArray[j];
	//		}	
	//	}
	//	glm::vec4 vertexNormal = glm::normalize(cumulativeNormals);
	//	vertexNormalArray[i*4] += vertexNormal.x;
	//	vertexNormalArray[i*4+1] += vertexNormal.y;
	//	vertexNormalArray[i*4+2] += vertexNormal.z;
	//	vertexNormalArray[i*4+3] += vertexNormal.w;
	//}
	//
	//	

	//glGenVertexArrays(1, &vao);
	//glBindVertexArray(vao);

	//GLuint vbo;
	//glGenBuffers(1, &vbo);
	//glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(float), vertices5Cell, GL_STATIC_DRAW);

	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//
	//GLuint vertNormals;
	//glGenBuffers(1, &vertNormals);
	//glBindBuffer(GL_ARRAY_BUFFER, vertNormals);
	//glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(float), vertexNormalArray, GL_STATIC_DRAW);

	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//
	//glGenBuffers(1, &index);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, 30 * sizeof(unsigned int), indices, GL_STATIC_DRAW);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	////glGenBuffers(1, &lineIndex);
	////glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineIndex);
	////glBufferData(GL_ELEMENT_ARRAY_BUFFER, 20 * sizeof(unsigned int), lineIndices, GL_STATIC_DRAW);
	////glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	////uniforms for 4D shape
	//projMatLoc = glGetUniformLocation(fiveCellProg, "projMat");
	//viewMatLoc = glGetUniformLocation(fiveCellProg, "viewMat");
	//fiveCellModelMatLoc = glGetUniformLocation(fiveCellProg, "fiveCellModelMat");
	//rotationZWLoc = glGetUniformLocation(fiveCellProg, "rotZW");
	//rotationXWLoc = glGetUniformLocation(fiveCellProg, "rotXW");

	//lightPosLoc = glGetUniformLocation(fiveCellProg, "lightPos");
	//light2PosLoc = glGetUniformLocation(fiveCellProg, "light2Pos");

	//cameraPosLoc = glGetUniformLocation(fiveCellProg, "camPos");

	//alphaLoc = glGetUniformLocation(fiveCellProg, "alpha");
	//
	////only use during development as computationally expensive
	////bool validProgram = is_valid(fiveCellProg);
	////if(!validProgram){
	////	fprintf(stderr, "ERROR: fiveCellShaderProg not valid\n");
	////	return 1;
	////}

	//glBindVertexArray(0);

	//fiveCellModelMatrix = glm::mat4(1.0);

	//glm::vec3 scale5Cell = glm::vec3(5.0f, 5.0f, 5.0f);
	//scale5CellMatrix = glm::scale(fiveCellModelMatrix, scale5Cell);
	//fiveCellModelMatrix = scale5CellMatrix;
	
//***********************************************************************************************************

	//workaround for macOS Mojave bug
	//needDraw = true;

	//radius = 0.75f;
	
	

	return true;


}

bool FiveCell::BSetupRaymarchQuad(GLuint shaderProg)
{
//	float sceneWidth = FConvertUint32ToFloat(m_nRenderWidth);
//	float sceneHeight = FConvertUint32ToFloat(m_nRenderHeight);
//	float vertPosX = sceneWidth * 0.5f;
//	float vertPosY = sceneHeight * 0.5f;

//	float sceneVerts[] = {
//		-vertPosX, -vertPosY, 0.0f,
//		vertPosX, -vertPosY, 0.0f,
//		-vertPosX, vertPosY, 0.0f,
//		-vertPosX, vertPosY, 0.0f,
//		vertPosX, -vertPosY, 0.0f,
//		vertPosX, vertPosY, 0.0f		
//	};
	
	float sceneVerts[] = {
		-1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f
	};
	m_uiNumSceneVerts = _countof(sceneVerts);

	unsigned int sceneIndices[] = {
		0, 1, 2,
		2, 3, 0
	};
	m_uiNumSceneIndices = _countof(sceneIndices);

	glGenVertexArrays(1, &m_uiglSceneVAO);
	glBindVertexArray(m_uiglSceneVAO);

	GLuint m_uiglSceneVBO;
	glGenBuffers(1, &m_uiglSceneVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_uiglSceneVBO);
	glBufferData(GL_ARRAY_BUFFER, m_uiNumSceneVerts * sizeof(float), sceneVerts, GL_STATIC_DRAW);

	glGenBuffers(1, &m_uiglIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiglIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_uiNumSceneIndices * sizeof(unsigned int), sceneIndices, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);

	//m_gliAspectLocation = glGetUniformLocation(shaderProg, "aspect");
	//m_gliTanFovLocation = glGetUniformLocation(shaderProg, "fovYScale");
	//m_gliViewMatrixLocation = glGetUniformLocation(shaderProg, "view");
	//m_gliProjectionMatrixLocation = glGetUniformLocation(shaderProg, "proj");
	//m_gliEyeMatLocation = glGetUniformLocation(shaderProg, "eyeMat");
	m_gliMVEPMatrixLocation = glGetUniformLocation(shaderProg, "MVEPMat");
	m_gliInverseMVEPLocation = glGetUniformLocation(shaderProg, "InvMVEP");
	m_gliMVEMatrixLocation = glGetUniformLocation(shaderProg, "MVEMat");
	m_gliInverseMVELocation = glGetUniformLocation(shaderProg, "InvMVE");
	m_gliRandomSizeLocation = glGetUniformLocation(shaderProg, "randSize");
	m_gliRMSModulateValLocation = glGetUniformLocation(shaderProg, "rmsModVal");
	//m_gliRotation3DLocation = glGetUniformLocation(shaderProg, "rot3D");
	//m_gliTimerLocation = glGetUniformLocation(shaderProg, "timer");

	raymarchQuadModelMatrix = glm::mat4(1.0f);

	return true;
}

//***********************************************************************************************
// Cube signed distance function to calculate world position for audio location 
//***********************************************************************************************
//float FiveCell::cubeSDF(glm::vec3 samplePoint){
//	
//	glm::vec3 size = glm::vec3(1.0f, 1.0f, 1.0f);
//	
//	glm::vec3 d = glm::abs(samplePoint) - glm::vec3(size.x / 2.0, size.y / 2.0, size.z / 2.0);
//    
//    	// Assuming samplePoint is inside the cube, how far is it from the surface?
//    	// Result will be negative or zero. Because the point is assumed inside the cube, 
//	// the components of d will be negative values. This means taking the max between
//	// them is checking which side of the cube is closest as the bigger value will
//	// be closer to 0. The the min value is taken between the resulting distance and 0.
//    	float insideDistance = fmin(fmax(d.x, fmax(d.y, d.z)), 0.0);
//    	
//    	// Assuming p is outside the cube, how far is it from the surface?
//    	// Result will be positive or zero.
//    	float outsideDistance = glm::length(glm::max(d, glm::vec3(0.0)));
//    	
//    	return insideDistance + outsideDistance;
//}

//***********************************************************************************************
// Calculate distance to raymarched cube 
//***********************************************************************************************

//float FiveCell::distanceToObject(glm::vec3 origin, glm::vec3 direction){
//
//	float minDist = 0.0f;
//	float maxDist = 100.0f;
//	unsigned int maxMarchingSteps = 255;
//	float epsilon = 0.0001f;
//
//	float depth = minDist;
//
//    for (int i = 0; i < maxMarchingSteps; i++) {
//        float dist = cubeSDF(origin + depth * direction);
//        if (dist < epsilon) {
//		return depth;
//        }
//        depth += dist;
//        if (depth >= maxDist) {
//            return maxDist;
//        }
//    }
//    return maxDist;
//}

void FiveCell::update(glm::mat4 projMat, glm::mat4 viewMat, glm::mat4 eyeMat, glm::vec3 camFront, glm::vec3 camPos, MachineLearning& machineLearning){

//***********************************************************************************************************
// Update Stuff Here
//*********************************************************************************************************

	//rms value from Csound
	modulateVal = *m_pRmsOut;			
	
	//matrices for raymarch shaders
	modelViewEyeMat = eyeMat * viewMat * raymarchQuadModelMatrix;
	inverseMVEMat = glm::inverse(modelViewEyeMat);
	modelViewEyeProjectionMat = projMat * eyeMat * viewMat * raymarchQuadModelMatrix;
	inverseMVEPMat = glm::inverse(modelViewEyeProjectionMat);

	glm::vec4 mengerPosition = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::mat4 mengerModelMatrix = glm::mat4(1.0f);		

	//position of menger cube
	//glm::vec4 posCameraSpace = eyeMat * viewMat * mengerModelMatrix * mengerPosition;;		
	glm::vec4 posCameraSpace = viewMat * mengerModelMatrix * mengerPosition;;		

	//position of menger cube in world space
	glm::vec4 posWorldSpace = mengerModelMatrix * mengerPosition;
	
	//calculate azimuth and elevation values for hrtf
	//glm::vec4 viewerPosCameraSpace = viewMat * glm::vec4(camPos, 1.0f);
	glm::vec4 viewerPosCameraSpace = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 viewerPosWorldSpace = glm::vec4(camPos, 1.0f);;

	glm::vec4 soundPosCameraSpace = posCameraSpace;
	glm::vec4 soundPosWorldSpace = posWorldSpace;

	float rCamSpace = sqrt(pow(soundPosCameraSpace.x, 2) + pow(soundPosCameraSpace.y, 2) + pow(soundPosCameraSpace.z, 2));
		
	float rWorldSpace = sqrt(pow(soundPosWorldSpace.x - viewerPosWorldSpace.x, 2) + pow(soundPosWorldSpace.y - viewerPosWorldSpace.y, 2) + pow(soundPosWorldSpace.z - viewerPosWorldSpace.z, 2));

	//azimuth in camera space
	float valX = soundPosCameraSpace.x - viewerPosCameraSpace.x;
	float valZ = soundPosCameraSpace.z - viewerPosCameraSpace.z;

	float azimuth = atan2(valX, valZ);
	azimuth *= (180.0f/PI); 	
	
	//elevation in world space
	//float oppSide = soundPosWorldSpace.y - viewerPosWorldSpace.y;
	//float sinVal = oppSide / rWorldSpace;
	
	//elevation in camera space
	float oppSide = soundPosCameraSpace.y - viewerPosCameraSpace.y;
	float sinVal = oppSide / rCamSpace;
	float elevation = asin(sinVal);
	elevation *= (180.0f/PI);		
	
	*hrtfVals[0] = (MYFLT)azimuth;
	*hrtfVals[1] = (MYFLT)elevation;
	*hrtfVals[2] = (MYFLT)rCamSpace;

//*********************************************************************************************
// Machine Learning 
//*********************************************************************************************
	bool currentRandomState = m_bPrevRandomState;
	if(machineLearning.bRandomParams != currentRandomState && machineLearning.bRandomParams == true){
		//random audio params
		std::uniform_real_distribution<float> distribution(1, 10);
		std::random_device rd;
		std::default_random_engine generator (rd());
		float val = distribution(generator);
		*randomFrequencyVal = (MYFLT)val;
		//std::cout << "Freq val - " << *randomFrequencyVal << std::endl;
			
		//random visual params
		std::uniform_real_distribution<float> distribution2(1.0, 4.0);
		std::random_device rd2;
		std::default_random_engine gen2 (rd2());
		sizeVal = distribution2(gen2);
		//std::cout << "Random size val = " << sizeVal << std::endl;
	}
	m_bPrevRandomState = machineLearning.bRandomParams;

	//if(machineLearning.bRecord){
	//	inputData.push_back((double)viewerPosWorldSpace.x);	
	//	inputData.push_back((double)viewerPosWorldSpace.y);	
	//	inputData.push_back((double)viewerPosWorldSpace.z);	

	//	outputData.push_back((double)*randomFrequencyVal);
	//	outputData.push_back((double)sizeVal);

//#ifdef __APPLE__
	//	trainingData.recordSingleElement(inputData, outputData);	
//#elif _WIN32
	//	trainingData.input = inputData;
	//	trainingData.output = outputData;
	//	trainingSet.push_back(trainingData);
//#endif

	//	std::cout << "Recording Data" << std::endl;
	//	inputData.clear();
	//	outputData.clear();
	//}
	//machineLearning.bRecord = false;

	//bool currentTrainState = m_bPrevTrainState;
	//if(machineLearning.bTrainModel != currentTrainState && machineLearning.bTrainModel == true){

//#ifdef __APPLE__
	//	staticRegression.train(trainingData);
//#elif _WIN32
	//	staticRegression.train(trainingSet);
//#endif

	//	std::cout << "Model Trained" << std::endl;
	//}	
	//m_bPrevTrainState = machineLearning.bTrainModel;

	//bool currentHaltState = m_bPrevHaltState;
	//if(machineLearning.bRunModel && !machineLearning.bHaltModel){
	//	std::vector<double> modelOut;
	//	std::vector<double> modelIn;

	//	modelIn.push_back((double)viewerPosWorldSpace.x);
	//	modelIn.push_back((double)viewerPosWorldSpace.y);
	//	modelIn.push_back((double)viewerPosWorldSpace.z);

	//	modelOut = staticRegression.run(modelIn);

	//	if(modelOut[0] > 10.0f) modelOut[0] = 10.0f;
	//	if(modelOut[0] < 1.0f) modelOut[0] = 1.0f;
	//	*randomFrequencyVal = (MYFLT)modelOut[0];
	//	if(modelOut[1] > 4.0f) modelOut[0] = 4.0f;
	//	if(modelOut[1] < 1.0f) modelOut[0] = 1.0f;
	//	sizeVal = (float)modelOut[1];
	//	std::cout << "Model Running" << std::endl;
	//	modelIn.clear();
	//	modelOut.clear();
	//} else if(!machineLearning.bRunModel && machineLearning.bHaltModel != currentHaltState){
	//	machineLearning.bRunModel = false;
	//	std::cout << "Model Stopped" << std::endl;
	//}
	//m_bPrevHaltState = machineLearning.bHaltModel;

	//std::string mySavedModel = "mySavedModel.json";
	//bool currentSaveState = m_bPrevSaveState;
	//if(machineLearning.bSaveTrainingData!= currentSaveState && machineLearning.bSaveTrainingData == true){

//#ifdef __APPLE__
	//	trainingData.writeJSON(mySavedModel);	
//#elif _WIN32
	//	staticRegression.writeJSON(mySavedModel);
//#endif

	//	std::cout << "Saving Training Data" << std::endl;
	//}
	//m_bPrevSaveState = machineLearning.bSaveTrainingData;
	//
	//bool currentLoadState = m_bPrevLoadState;
	//if(machineLearning.bLoadTrainingData != currentLoadState && machineLearning.bLoadTrainingData == true){
	//	
//#ifdef __APPLE__
	//	trainingData.readJSON(mySavedModel);
	//	staticRegression.train(trainingData);
//#elif _WIN32
	//	staticRegression.readJSON(mySavedModel);	
//#endif

	//	std::cout << "Loading Data and Training Model" << std::endl;
	//}
	//m_bPrevLoadState = machineLearning.bLoadTrainingData;

//*********************************************************************************************
	
	//float rotAngle = glfwGetTime() * 0.2f;
	//glm::mat4 fiveCellRotationMatrix3D = glm::rotate(modelMatrix, rotAngle, glm::vec3(0, 1, 0)) ;
	//fiveCellModelMatrix = scale5CellMatrix;

	
}

void FiveCell::draw(GLuint skyboxProg, GLuint groundPlaneProg, GLuint soundObjProg, GLuint fiveCellProg, GLuint quadShaderProg, glm::mat4 projMat, glm::mat4 viewMat, glm::mat4 eyeMat, RaymarchData& raymarchData, GLuint mengerProg){
		
//**********************************************************************************************************
// Draw Stuff Here
//*********************************************************************************************************

	glm::mat4 viewEyeMat = eyeMat * viewMat;

	
	camPosPerEye = glm::vec3(viewEyeMat[0][3], viewEyeMat[1][3], viewEyeMat[2][3]);

	//draw 4D polytope	
	//float a = 0.0f;

	//glBindVertexArray(vao);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index);
	//glUseProgram(fiveCellProg);

	//glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, &projMat[0][0]);
	//glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, &viewEyeMat[0][0]);
	//glUniformMatrix4fv(fiveCellModelMatLoc, 1, GL_FALSE, &fiveCellModelMatrix[0][0]);
      	//glUniformMatrix4fv(rotationZWLoc, 1, GL_FALSE, &rotationZW[0][0]);
	//glUniformMatrix4fv(rotationXWLoc, 1, GL_FALSE, &rotationXW[0][0]);
	//glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
	//glUniform3f(light2PosLoc, light2Pos.x, light2Pos.y, light2Pos.z);
	//glUniform3f(cameraPosLoc, camPosPerEye.x, camPosPerEye.y, camPosPerEye.z);
	//glUniform1f(alphaLoc, a);

	////single draw call for refractive rendering
	////glDrawElements(GL_TRIANGLES, 30 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);
      	////glDrawElements(GL_LINES, 20 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);

      	////draw 5-cell using index buffer and 5 pass transparency technique from http://www.alecjacobson.com/weblog/?p=2750
	////1st pass
	//glDisable(GL_CULL_FACE);
	//glDepthFunc(GL_LESS);
	//float f = 0.75f;
	//float origAlpha = 0.4f;	
	//a = 0.0f;
	//glUniform1f(alphaLoc, a);
	//glDrawElements(GL_TRIANGLES, 30 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);

	////2nd pass
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glDepthFunc(GL_ALWAYS);
	//a = origAlpha * f;
	//glUniform1f(alphaLoc, a);
	//glDrawElements(GL_TRIANGLES, 30 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);
	//
	////3rd pass
	//glDepthFunc(GL_LEQUAL);
	//a = (origAlpha - (origAlpha * f)) / (1.0f - (origAlpha * f));
	//glUniform1f(alphaLoc, a);
	//glDrawElements(GL_TRIANGLES, 30 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);
	//
	////4th pass
	//glCullFace(GL_BACK);
	//glDepthFunc(GL_ALWAYS);
	//a = origAlpha * f;
	//glUniform1f(alphaLoc, a);
	//glDrawElements(GL_TRIANGLES, 30 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);

	////5th pass
	//glDisable(GL_CULL_FACE);
	//glDepthFunc(GL_LEQUAL);
	//a = (origAlpha - (origAlpha * f)) / (1.0f - (origAlpha * f));
	//glUniform1f(alphaLoc, a);
	//glDrawElements(GL_TRIANGLES, 30 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);

	// draw ground plane 
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);

	glBindVertexArray(groundVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundIndexBuffer); 
	glBindTexture(GL_TEXTURE_2D, groundTexture); 
	glUseProgram(groundPlaneProg);

	glUniformMatrix4fv(ground_projMatLoc, 1, GL_FALSE, &projMat[0][0]);
	glUniformMatrix4fv(ground_viewMatLoc, 1, GL_FALSE, &viewEyeMat[0][0]);
	glUniformMatrix4fv(ground_modelMatLoc, 1, GL_FALSE, &groundModelMatrix[0][0]);
	glUniform3f(ground_lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(ground_light2PosLoc, light2Pos.x, light2Pos.y, light2Pos.z);
	glUniform3f(ground_cameraPosLoc, camPosPerEye.x, camPosPerEye.y, camPosPerEye.z);

	glDrawElements(GL_TRIANGLES, 6 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);
	//glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//draw texture quad
	//glBindVertexArray(quadVAO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIndexBuffer); 
	//glBindTexture(GL_TEXTURE_2D, quadTexID); 
	//glUseProgram(quadShaderProg);

	//glUniformMatrix4fv(quad_projMatLoc, 1, GL_FALSE, &projMat[0][0]);
	//glUniformMatrix4fv(quad_viewMatLoc, 1, GL_FALSE, &viewEyeMat[0][0]);
	//glUniformMatrix4fv(quad_modelMatLoc, 1, GL_FALSE, &quadModelMatrix[0][0]);
	////glUniform3f(quad_lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
	////glUniform3f(quad_light2PosLoc, light2Pos.x, light2Pos.y, light2Pos.z);
	////glUniform3f(quad_cameraPosLoc, camPosPerEye.x, camPosPerEye.y, camPosPerEye.z);

	//glDrawElements(GL_TRIANGLES, 36 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);

	glDisable(GL_CULL_FACE);
	//draw skybox
	//skybox.draw(projMat, viewEyeMat, skyboxProg);
	glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(viewEyeMat));
	//glm::mat4 viewNTEyeMat = eyeMat * viewNoTranslation; 
		
	//glm::mat4 viewNTEyeMat = glm::mat4(
	//		viewEyeMat[0][0], viewEyeMat[1][0], viewEyeMat[2][0], 0.0f,
	//		viewEyeMat[0][1], viewEyeMat[1][1], viewEyeMat[2][1], 0.0f,
	//		viewEyeMat[0][2], viewEyeMat[1][2], viewEyeMat[2][2], 0.0f,
	//		0.0f, 0.0f, 0.0f, 1.0f
	//		);

	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);
		
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxIndexBuffer); 
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexID);
	glUseProgram(skyboxProg);
	
	//glUniform1i(skybox_texUniformLoc, 0);
	glUniformMatrix4fv(skybox_projMatLoc, 1, GL_FALSE, &projMat[0][0]);
	glUniformMatrix4fv(skybox_viewMatLoc, 1, GL_FALSE, &viewNoTranslation[0][0]);
	glUniformMatrix4fv(skybox_modelMatLoc, 1, GL_FALSE, &skyboxModelMatrix[0][0]);
		
	glDrawElements(GL_TRIANGLES, 36 * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);

	///glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	//draw sound test objects
	//for(int i = 0; i < _countof(soundObjects); i++){

	//for(int i = 0; i < 5; i++){
	//	soundObjects[i].draw(projMat, viewEyeMat, lightPos, light2Pos, camPosPerEye, soundObjProg);
	//}	
		
	//draw menger sponge
	float mengerAspect = raymarchData.aspect;
	float mengerTanFovYOver2 = raymarchData.tanFovYOver2;

	glUseProgram(mengerProg);
	//glUniform1f(m_gliAspectLocation, mengerAspect);
	//glUniform1f(m_gliTanFovLocation, mengerTanFovYOver2);
	//glUniformMatrix4fv(m_gliViewMatrixLocation, 1, GL_FALSE, &viewMat[0][0]);
	//glUniformMatrix4fv(m_gliProjectionMatrixLocation, 1, GL_FALSE, &projMat[0][0]);
	//glUniformMatrix4fv(m_gliEyeMatLocation, 1, GL_FALSE, &eyeMat[0][0]);
	glUniformMatrix4fv(m_gliMVEPMatrixLocation, 1, GL_FALSE, &modelViewEyeProjectionMat[0][0]);
	glUniformMatrix4fv(m_gliInverseMVEPLocation, 1, GL_FALSE, &inverseMVEPMat[0][0]);
	glUniformMatrix4fv(m_gliMVEMatrixLocation, 1, GL_FALSE, &modelViewEyeMat[0][0]);
	glUniformMatrix4fv(m_gliInverseMVELocation, 1, GL_FALSE, &inverseMVEMat[0][0]);
	glUniform1f(m_gliRandomSizeLocation, sizeVal);
	glUniform1f(m_gliRMSModulateValLocation, modulateVal);
	//glUniform1f(m_gliRotation3DLocation, static_cast<float>(*m_pRotationVal));
	//glUniform1f(m_gliTimerLocation, raymarchData.modAngle);

	glBindVertexArray(m_uiglSceneVAO);
	glDrawElements(GL_TRIANGLES, m_uiNumSceneIndices * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);
	
	//update other events like input handling
	//glfwPollEvents();

	// workaround for macOS Mojave bug
	//if(needDraw){
	//	glfwShowWindow(window);
	//	glfwHideWindow(window);
	//	glfwShowWindow(window);
	//	needDraw = false;
	//}

	//put the stuff we've been drawing onto the display
	//glfwSwapBuffers(window);

	//glfwSetCursorPosCallback(window, mouse_callback);
	//glfwSetWindowSizeCallback(window, glfw_window_size_callback);
	//glfwSetErrorCallback(glfw_error_callback);
		
	//lastFrame = currentFrame;
}

void FiveCell::exit(){
	//stop csound
	session->StopPerformance();
	//close GL context and any other GL resources
	glfwTerminate();
}
