
#include "MeshInfoLoader.h"
#include <fstream>
#include <string>
#include <iostream>

using namespace std;
using namespace glm;

MeshInfoLoader::MeshInfoLoader() {
	//printf("Opening file\n");

	//loadModel("C:\\Users\\Mia\\GitHub\\King_of_Buggies\\models\\brick.obj");

	//printf("Closing file\n");
}

MeshInfoLoader::MeshInfoLoader(char *filename) {
	//printf("Opening file\n");

	loadModel(filename);

	//printf("Closing file\n");
}


void sharedIndices(vector<unsigned int> &_faces, vector<unsigned int> &_nFaces, vector<vec3> &_normals,
	vector<vec3> &vertices, vector<vec3> &normals, vector<unsigned int> &faces)
{

	normals.resize(vertices.size(), vec3(0.0, 0.0, 0.0));

	faces.clear();

	for (int i = 0; i<_faces.size(); i++)
	{
		int vi = _faces[i];
		int ni = _nFaces[i];

		if (normals[vi] == vec3())
		{
			normals[vi] = _normals[ni];
			faces.push_back(vi);
		}
		else if (normals[vi] != _normals[ni])
		{
			vertices.push_back(vertices[vi]);
			normals.push_back(_normals[ni]);
			faces.push_back(vertices.size() - 1);
		}
		else
			faces.push_back(vi);

	}
}

bool MeshInfoLoader::loadModel(char *filename)
{
	printf("Opening file\n");

	FILE* f = fopen(filename, "r");
	if (f == NULL)
	{
		printf("File could not be opened\n");
		return false;
	}

	//vector<GLv3f> vertices;
	vector<vec3> _normals;
	vector<unsigned int> _faces;
	vector<unsigned int> _nfaces;

	while (true)
	{
		char text[256];

		int res = fscanf(f, "%s", text);
		if (res == EOF)
			break;

		if (strcmp(text, "v") == 0)
		{
			vec3 vert;

			fscanf(f, "%f %f %f\n", &vert.x, &vert.y, &vert.z);
			vertices.push_back(vert);
		}
		else if (strcmp(text, "vn") == 0)
		{
			vec3 norm;

			fscanf(f, "%f %f %f\n", &norm.x, &norm.y, &norm.z);
			_normals.push_back(norm);
		}
		else if (strcmp(text, "f") == 0)
		{
			unsigned int vi1, vi2, vi3, ni1, ni2, ni3;

			if (fscanf(f, "%d//%d %d//%d %d//%d\n",
				&vi1, &ni1, &vi2, &ni2, &vi3, &ni3) != 6)
			{
				printf("Face couldn't be read\n");
				return false;
			}

			_faces.push_back(vi1 - 1);
			_faces.push_back(vi2 - 1);
			_faces.push_back(vi3 - 1);

			_nfaces.push_back(ni1 - 1);
			_nfaces.push_back(ni2 - 1);
			_nfaces.push_back(ni3 - 1);
		}
	}

	sharedIndices(_faces, _nfaces, _normals, vertices, normals, indices);

	printf("Closing file\n");
}

void MeshInfoLoader::clearData() {
	vertices.clear();
	normals.clear();
	indices.clear();
}

/*void ObjFile::draw (GLuint vertexLocation, GLuint normalLocation) {
glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

glEnableVertexAttribArray (vertexLocation);
glVertexAttribPointer (vertexLocation, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
glEnableVertexAttribArray (normalLocation);
glVertexAttribPointer (normalLocation, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)(sizeof(GLfloat) * vertices.size()));

glDrawElements (GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}*/

/*int ObjFile::numVertices () {
return vertices.size() / 4;
}

GLv3f ObjFile::getVertex (int i) {
return GLv3f (vertices[3*i], vertices[3*i+1], vertices[3*i+2]);
}

GLv3f ObjFile::getNormal (int i) {
return GLv3f (normals[3*i], normals[3*i+1], normals[3*i+2]);
}

void ObjFile::setVertex (int i, GLv3f v) {
vertices[3*i] = v.x;
vertices[3*i+1] = v.y;
vertices[3*i+2] = v.z;
}

void ObjFile::setNormal (int i, GLv3f v) {
normals[3*i] = v.x;
normals[3*i+1] = v.y;
normals[3*i+2] = v.z;
}

void ObjFile::calculateNormals () {

printf("Calc normals\n");

for (int i = 0; i < vertices.size(); i += 3) {
normals.push_back (0.0f);
normals.push_back (0.0f);
normals.push_back (0.0f);
}

for (int i = 0; i < indices.size(); i += 3) {
int i1 = indices[i];
int i2 = indices[i+1];
int i3 = indices[i+2];

GLv3f p1 = getVertex (i1);
GLv3f p2 = getVertex (i2);
GLv3f p3 = getVertex (i3);

GLv3f fNormal = cross (p2 - p1, p3 - p1);

float l = length (fNormal);
if (fNormal.x != 0.0f && fNormal.y != 0.0f && fNormal.z != 0.0f) {
fNormal = normalize (fNormal);
setNormal (i1, getNormal(i1) + fNormal);
setNormal (i2, getNormal(i2) + fNormal);
setNormal (i3, getNormal(i3) + fNormal);
}
}

for (int i = 0; i < normals.size(); i += 3) {
setNormal (i/3, normalize(getNormal(i/3)));
}

printf("Calc normals->\n");
}


void ObjFile::flipFaces(){
for(int i=0; i<indices.size(); i+=3)
{
GLuint temp = indices[i];
indices[i] = indices[i+2];
indices[i+2] = temp;
}
}*/

vec3 MeshInfoLoader::getCenter(){
	vec3 center;

	for (int i = 0; i<vertices.size(); i++)
	{
		center = center + vertices[i];
	}

	float divisor = (1.0 / (vertices.size()));

	return center * divisor;
}

float MeshInfoLoader::getBottom(){
	float bottom = 1000.0;

	for (int i = 0; i<vertices.size(); i++)
	{
		bottom = std::min(bottom, vertices[i].z);
	}

	return bottom;
}

float MeshInfoLoader::getBoundingRadius()
{
	vec3 center = getCenter();
	float boundingRadius = 0.0;

	for (int i = 0; i<vertices.size(); i++)
	{
		vec3 point = vertices[i];
		float distance = length(center - point);
		boundingRadius = std::max(boundingRadius, distance);
	}

	return boundingRadius;
}

// CPSC 453: Introduction to Computer Graphics
//
// Implementation file for a class to handle obj file data
//
// Created by Troy Alderson
// Department of Computer Science
// University of Calgary
//
// Used with permission for CPSC 453 HW3