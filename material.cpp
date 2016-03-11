#ifndef MATERIAL_CPP
#define MATERIAL_CPP

#include "material.h"

ShaderList shaderList;

void glErrorCheckMaterial(const char* location)
{
	GLenum code = glGetError();

	switch (code)
	{
	case GL_INVALID_ENUM:
		cout << "GL_INVALID_ENUM - " << location << endl;
		break;
	case GL_INVALID_VALUE:
		cout << "GL_INVALID_VALUE - " << location << endl;
		break;
	case GL_INVALID_OPERATION:
		cout << "GL_INVALID_OPERATION - " << location << endl;
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		cout << "GL_INVALID_FRAMEBUFFER_OPERATION - " << location << endl;
		break;
	case GL_OUT_OF_MEMORY:
		cout << "GL_OUT_OF_MEMORY - " << location << endl;
		break;
	}
}

void ShaderList::initShaders()
{
	shaderIDs[ShaderList::DIFFUSE] = GetShader("diffuse");
	shaderIDs[ShaderList::DIFFUSE_TEX] = GetShader("diffuseTex");
	shaderIDs[ShaderList::SPECULAR] = GetShader("specular");
	shaderIDs[ShaderList::SPECULAR_TEX] = GetShader("specularTex");
	shaderIDs[ShaderList::TORRANCE_SPARROW] = GetShader("torranceSparrow");
	shaderIDs[ShaderList::TORRANCE_SPARROW_TEX] = GetShader("torranceSparrowTex");

}


Material::Material()
{
	
}

bool Material::usingVertices(){ return verticesUsed; }
bool Material::usingNormals(){ return normalsUsed; }
bool Material::usingUvs(){ return uvsUsed; }

void Material::useShader(){ glUseProgram(programID); }

void Material::loadUniforms(const mat4& transform, const mat4& objectTransform,
							vec3 viewer, vec3 light, vec3 color)
{
	GLuint uniformLocation = glGetUniformLocation(programID, "transform");
	glUniformMatrix4fv(uniformLocation, 1, false, &transform[0][0]);

	uniformLocation = glGetUniformLocation(programID, "objectTransform");
	glUniformMatrix4fv(uniformLocation, 1, false, &objectTransform[0][0]);

	uniformLocation = glGetUniformLocation(programID, "viewPos");
	glUniform3f(uniformLocation, viewer.x, viewer.y, viewer.z);

	uniformLocation = glGetUniformLocation(programID, "light");
	glUniform3f(uniformLocation, light.x, light.y, light.z);
	
	uniformLocation = glGetUniformLocation(programID, "color");
	glUniform3f(uniformLocation, color.x, color.y, color.z);

	glErrorCheckMaterial("Uniforms");
	
}

void Material::loadUniforms(const mat4& transform, const mat4& objectTransform,
	vec3 viewer, vec3 light, unsigned int texID, unsigned int texUnit)
{
	GLuint uniformLocation = glGetUniformLocation(programID, "transform");
	glUniformMatrix4fv(uniformLocation, 1, false, &transform[0][0]);

	uniformLocation = glGetUniformLocation(programID, "objectTransform");
	glUniformMatrix4fv(uniformLocation, 1, false, &objectTransform[0][0]);

	uniformLocation = glGetUniformLocation(programID, "viewPos");
	glUniform3f(uniformLocation, viewer.x, viewer.y, viewer.z);

	uniformLocation = glGetUniformLocation(programID, "light");
	glUniform3f(uniformLocation, light.x, light.y, light.z);

	uniformLocation = glGetUniformLocation(programID, "colorTexture");
	glActiveTexture(GL_TEXTURE0+texUnit);
	glBindTexture(GL_TEXTURE_2D, texID);
	glUniform1i(uniformLocation, 0);

	glErrorCheckMaterial("Uniforms");

}



/**
* Shader compilation and initialization
**/
GLuint GetShader(const string& name)
{
	const string shaderDir = "./shaders/";

	//Load shaders
	string vertShaderSource = loadShaderStringFromFile(shaderDir + name + ".vert");
	string fragShaderSource = loadShaderStringFromFile(shaderDir + name + ".frag");

	return CreateShaderProgram(vertShaderSource, fragShaderSource);
}

GLuint CreateShaderProgram(const string & vsSource,
	const string & fsSource)
{
	GLuint programID = glCreateProgram();
	GLuint vsID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fsID = glCreateShader(GL_FRAGMENT_SHADER);

	if (programID == 0 || vsID == 0 || fsID == 0)
	{
		// Clean up others that were created
		glDeleteProgram(programID);
		glDeleteShader(vsID);
		glDeleteShader(fsID);

		cerr << "Cannot create Shaders or Program" << endl;
		return 0; // invalid ID
	}

	// glShaderSource() expects char**, so these are helper variables
	const char * vsSourceArray = vsSource.c_str();
	const char * fsSourceArray = fsSource.c_str();

	// https://www.opengl.org/sdk/docs/man4/xhtml/glShaderSource.xml
	glShaderSource(vsID,
		1,
		&vsSourceArray,
		NULL);

	glShaderSource(fsID,
		1,
		&fsSourceArray,
		NULL);

	// Compile the Shader Sources, check for errors
	cout << "Compiling vertex shader" << endl;
	glCompileShader(vsID);
	bool vertSuccess = checkCompileStatus(vsID);

	cout << "Compiling fragment shader" << endl;
	glCompileShader(fsID);
	bool fragSuccess = checkCompileStatus(fsID);

	if (!vertSuccess ||
		!fragSuccess)
	{
		// Clean up others that were created
		glDeleteProgram(programID);
		glDeleteShader(vsID);
		glDeleteShader(fsID);

		cerr << "Cannot create Shaders or Program" << endl;
		return 0; // invalid ID
	}

	glAttachShader(programID, vsID);
	glAttachShader(programID, fsID);

	glLinkProgram(programID);

	if (!checkLinkStatus(programID))
	{
		// Clean up others that were created
		glDeleteProgram(programID);
		glDeleteShader(vsID);
		glDeleteShader(fsID);

		cerr << "Cannot create Shaders or Program" << endl;
		return 0; // invalid ID
	}

	return programID;
}

bool checkLinkStatus(GLint programID)
{
	GLint result;
	int infoLogLength;
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	if (!result)
	{
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
		vector<char> errorMsg(infoLogLength);
		glGetProgramInfoLog(programID,
			infoLogLength,
			NULL,
			errorMsg.data());
		cerr << errorMsg.data() << endl;
		return false;
	}
	return true;
}

bool checkCompileStatus(GLint shaderID)
{
	GLint result;
	int infoLogLength;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
		vector<char> errorMsg(infoLogLength);
		glGetShaderInfoLog(shaderID,
			infoLogLength,
			NULL,
			errorMsg.data());
		cerr << errorMsg.data() << endl;
		return false;
	}
	return true; // otherwise
}

// Returns Empty String if can't load from file
string loadShaderStringFromFile(const string & filePath)
{
	printf("Loading source: %s\n", filePath.c_str());

	string shaderCode;
	ifstream fileStream(filePath.c_str(), ios::in);
	if (fileStream.is_open())
	{
		string line;
		while (getline(fileStream, line))
		{
			shaderCode += "\n" + line;
		}
		fileStream.close();

	}
	else
	{
		cerr << "Could Not Open File " << filePath << endl;
	}
	return shaderCode;
}



#endif	//MATERIAL_CPP