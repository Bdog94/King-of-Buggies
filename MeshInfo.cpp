#include "MeshInfo.h"

MeshInfo::MeshInfo() {
	meshLoader = MeshInfoLoader();

	meshLoader.loadModel("models\\brick.obj");
	//unsigned int texID = meshLoader.LoadTexture("textures\\bricktexture.bmp");
	MeshObject brickObject = MeshObject(meshLoader.vertices, meshLoader.normals, meshLoader.uvs, meshLoader.indices);
	
	meshes.push_back(brickObject);

	meshLoader.clearData();
	
	meshLoader.loadModel("models\\cube.obj");
	MeshObject cubeObject = MeshObject(meshLoader.vertices, meshLoader.normals, meshLoader.uvs, meshLoader.indices);
	meshes.push_back(cubeObject);

	meshLoader.clearData();

	meshLoader.loadModel("models\\wheel.obj");
	MeshObject wheelObject = MeshObject(meshLoader.vertices, meshLoader.normals, meshLoader.uvs, meshLoader.indices);
	meshes.push_back(wheelObject);

	meshLoader.clearData();

	meshLoader.loadModel("models\\coin.obj");
	MeshObject coinObject = MeshObject(meshLoader.vertices, meshLoader.normals, meshLoader.uvs, meshLoader.indices);
	meshes.push_back(coinObject);

	meshLoader.clearData();

	meshLoader.loadModel("models\\donutlevel.obj");
	MeshObject levelObject = MeshObject(meshLoader.vertices, meshLoader.normals, meshLoader.uvs, meshLoader.indices);
	meshes.push_back(levelObject);

	meshLoader.clearData();

	meshLoader.loadModel("models\\oldlevel.obj");
	MeshObject oldObject = MeshObject(meshLoader.vertices, meshLoader.normals, meshLoader.uvs, meshLoader.indices);
	meshes.push_back(oldObject);

	meshLoader.clearData();

	meshLoader.loadModel("models\\buggy.obj");
	unsigned int buggyTexID = LoadTexture("textures\\buggytexture.png");
	MeshObject buggyObject = MeshObject(meshLoader.vertices, meshLoader.normals, meshLoader.uvs, meshLoader.indices, buggyTexID);
	meshes.push_back(buggyObject);

	meshLoader.clearData();

	meshLoader.loadModel("models\\unitbuggy.obj");
	MeshObject unitBuggyObject = MeshObject(meshLoader.vertices, meshLoader.normals, meshLoader.uvs, meshLoader.indices);
	meshes.push_back(unitBuggyObject);

	meshLoader.clearData();
}

MeshObject MeshInfo::getMesh(int objectType) {
	return meshes[objectType];
}

MeshObject* MeshInfo::getMeshPointer(int objectType){
	return &meshes[objectType];
}

vector<vec3> MeshInfo::getMeshVertices(int objectType) {
	return meshes[objectType].getVertices();
}

vector<vec3> MeshInfo::getMeshNormals(int objectType) {
	return meshes[objectType].getNormals();
}

vector<vec2> MeshInfo::getMeshUvs(int objectType) {
	return meshes[objectType].getUvs();
}

vector<unsigned int> MeshInfo::getMeshIndices(int objectType) {
	return meshes[objectType].getIndices();
}