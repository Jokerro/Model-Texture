// NewTrainingFramework.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../Utilities/utilities.h" 
#include "Vertex.h"
#include "Index.h"
#include "Shaders.h"
#include "Globals.h"
#include <conio.h>
#include <iostream>
#include <stdio.h>
#include <math.h>


GLuint vboId;
GLuint iboId;
GLuint textureHandle;
Shaders myShaders;
float time;
int vertices, indices;


Matrix transformMatrix;

int Init ( ESContext *esContext )
{
	glClearColor ( 0.0f, 0.0f, 0.0f, 1.0f );
	glEnable(GL_DEPTH_TEST);

	//triangle data (heap)
	FILE *pFile;
	fopen_s(&pFile, "../Resources/Models/Woman1.nfg", "r");
	fscanf_s(pFile, "NrVertices: %d", &vertices);
	Vertex *verticesData = new Vertex[vertices];
	for (int i = 0; i<vertices; i++) {
		fscanf_s(pFile, " %*d. pos:[%f, %f, %f]; norm:[%*f, %*f, %*f]; binorm:[%*f, %*f, %*f]; tgt:[%*f, %*f, %*f]; uv:[%f, %f]; ", &verticesData[i].pos.x, &verticesData[i].pos.y, &verticesData[i].pos.z, &verticesData[i].uv.x, &verticesData[i].uv.y);
	}

	//buffer object
	glGenBuffers(1, &vboId); //buffer object name generation
	glBindBuffer(GL_ARRAY_BUFFER, vboId); //buffer object binding
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*vertices, verticesData, GL_STATIC_DRAW); //creation and initializion of buffer onject storage
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	fscanf_s(pFile, "NrIndices: %d", &indices);
	Index *indicesData = new Index[indices];
	for (int i = 0; i<indices/3; i++) {
		fscanf_s(pFile, " %*d. %u, %u, %u ", &indicesData[i].x, &indicesData[i].y, &indicesData[i].z);
	}
	
	glGenBuffers(1, &iboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*indices, indicesData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	fclose(pFile);

	glGenTextures(1, &textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	int width, height, bpp;
	char* bufferTGA = LoadTGA("../Resources/Textures/Woman1.tga", &width, &height, &bpp);

	if (bpp == 24) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, bufferTGA);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferTGA);
	}
	delete []bufferTGA;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	delete []verticesData;
	delete []indicesData;
	//creation of shaders and program 
	return myShaders.Init("../Resources/Shaders/TriangleShaderVS.vs", "../Resources/Shaders/TriangleShaderFS.fs");

}

void Draw ( ESContext *esContext )
{
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(myShaders.program);

	glBindBuffer(GL_ARRAY_BUFFER, vboId);

	GLfloat* ptr = 0;
	if (myShaders.positionAttribute != -1) //attribute passing to shader, for uniforms use glUniform1f(time, deltaT); glUniformMatrix4fv( m_pShader->matrixWVP, 1, false, (GLfloat *)&rotationMat );
	{
		glEnableVertexAttribArray(myShaders.positionAttribute);
		glVertexAttribPointer(myShaders.positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), ptr);
	}

	if (myShaders.uvAttribute != -1) //attribute passing to shader, for uniforms use glUniform1f(time, deltaT); glUniformMatrix4fv( m_pShader->matrixWVP, 1, false, (GLfloat *)&rotationMat );
	{
		glEnableVertexAttribArray(myShaders.uvAttribute);
		glVertexAttribPointer(myShaders.uvAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), ptr+3);
	}	
	
	if (myShaders.matrixTransform != -1) {
		glUniformMatrix4fv(myShaders.matrixTransform, 1, false, (GLfloat*)&transformMatrix);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

	unsigned short textureUnit = 0;
	// make active a texture unit 
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	// bind the texture to the currently active texture unit 
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	// set the uniform sampler 
	glUniform1i(myShaders.textureUniform, textureUnit); 

	glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_SHORT, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	eglSwapBuffers ( esContext->eglDisplay, esContext->eglSurface );
}

void Update(ESContext *esContext, float deltaTime)
{
	time += deltaTime;
	transformMatrix.SetRotationY(time);
	transformMatrix.m[3][1] = -0.9f;
}

void Key ( ESContext *esContext, unsigned char key, bool bIsPressed)
{

}

void CleanUp()
{
	glDeleteBuffers(1, &vboId);
	glDeleteBuffers(1, &iboId);
	glDeleteTextures(1, &textureHandle);
}

int _tmain(int argc, _TCHAR* argv[])
{
	ESContext esContext;

    esInitContext ( &esContext );

	esCreateWindow ( &esContext, "Hello Model", Globals::screenWidth, Globals::screenHeight, ES_WINDOW_RGB | ES_WINDOW_DEPTH);

	if ( Init ( &esContext ) != 0 )
		return 0;

	esRegisterDrawFunc ( &esContext, Draw );
	esRegisterUpdateFunc ( &esContext, Update );
	esRegisterKeyFunc ( &esContext, Key);

	esMainLoop ( &esContext );

	//releasing OpenGL resources
	CleanUp();

	//identifying memory leaks
	MemoryDump();
	printf("Press any key...\n");
	_getch();

	return 0;
}
