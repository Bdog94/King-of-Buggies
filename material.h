#ifndef MATERIAL_H
#define MATERIAL_H

#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<GL/glew.h>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

const string shaderDir = "./shaders";

class Material
{
protected:
	GLuint programID;
	const string shader_name = "diffuse";
	const bool verticesUsed = true;
	const bool normalsUsed = true;
	const bool uvsUsed = true;
public:
	Material(string shader_name);

	void loadUniforms(mat4 transform, vec3 light);
	void useShader();

	bool usingVertices();
	bool usingNormals();
	bool usingUvs();
};


/**
* Shader compilation and initialization
**/
GLuint CreateShaderProgram(const string & vsSource,
	const string & fsSource);

GLuint CreateShaderProgram(const string & vsSource,
	const string & gsSource,
	const string & fsSource);

bool checkCompileStatus(GLint shaderID);
bool checkLinkStatus(GLint programID);

string loadShaderStringFromFile(const string & filePath);


#endif	//MATERIAL_H