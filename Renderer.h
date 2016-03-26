#ifndef RENDERER_H
#define RENDERER_H

#include <Windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>

#include "MeshObject.h"
#include "camera.h"
#include "Material.h"
#include "Specular.h"
#include "diffuse.h"
#include "torranceSparrow.h"
#include "unshaded.h"
#include "GameState.h"

using namespace std;
using namespace glm;



#define M_PI  3.14159265358979323846f

struct SHADOW_BEHAVIOUR{
	enum { CAST = 0, RECEIVE, CAST_AND_RECEIVE, NONE };
};
struct VAO{
	enum { VERT = 0, VERT_NORMALS, VERT_UVS, VERT_NORMALS_UVS, COUNT };
};

struct VBO{
	enum { VERT = 0, NORMALS, UVS, INDICES, COUNT };
};

struct Viewport{
	float x, y, width, height;		//Change to floats
	mat4 viewRatio;							//Get rid of?

	const unsigned int DEFAULT = 0;

	Viewport(float x, float y, float width, float height);
};

const unsigned int DEFAULT_WIDTH = 800;
const unsigned int DEFAULT_HEIGHT = 800;


class Renderer {
private:
	class ObjectInfo{
	public:
		ObjectInfo();

		vector<vec3>* mesh;
		vector<vec3>* normals;
		vector<vec2>* uvs;
		vector<unsigned int>* indices;
		GLuint texID;
		Material* mat;
		vec3 color;
		int shadowBehaviour;
		bool deleted;

		//Information about object orientation
		mat4 transform;
		mat4 scaling;
	};

	class LightInfo
	{
	public:
		vec3 pos;
		bool deleted;

		LightInfo();

	};

	GLFWwindow * window;
	


	vector<ObjectInfo> objects;		//Objects to be drawn
	vector<LightInfo> lights;
	vector<Viewport> viewports;

	unsigned int activeViewport;

	//Transform matrices
	mat4 projection;
	mat4 modelview;

	Camera* camera;


	GLuint vao[VAO::COUNT];
	GLuint vbo[VBO::COUNT];

	bool debugging;

	/**
	* Functions
	**/

	void initializeVAOs();		
	bool loadBuffers(const ObjectInfo& object);		
	bool loadVertBuffer(const ObjectInfo& object);	
	bool loadVertNormalBuffer(const ObjectInfo& object);
	bool loadVertUVBuffer(const ObjectInfo& object);	
	bool loadVertNormalUVBuffer(const ObjectInfo& object);

	//Debugging
	void debug_message(string message);		

public:
	Renderer(GLFWwindow* _window);
	~Renderer();
	void renderLoop();

	//Transformations
	void loadModelviewTransform(const mat4& _modelview);
	void loadProjectionTransform(const mat4& _projection);
	void loadPerspectiveTransform(float near, float far, float fov);
	void loadOrthographicTransform(float near, float far, float width, float height);
	void loadCamera(Camera* _camera);

	//Drawable objects
	unsigned int generateObjectID();
	void assignMeshObject(unsigned int id, MeshObject* mesh);
	void assignMesh(unsigned int object, vector<vec3>* mesh);
	void assignNormals(unsigned int object, vector<vec3>* normals);
	void assignUVs(unsigned int object, vector<vec2>* uvs);
	void assignIndices(unsigned int object, vector<unsigned int>* indices);		//If no indices are assigned, will traverse mesh in order
	void assignColor(unsigned int object, vec3 color);
	void assignTexture(unsigned int objectID, unsigned int textureID);
	void setShadowBehaviour(int object, int behaviour);
	void assignMaterial(unsigned int object, Material* mat);
	void assignTransform(unsigned int object, const mat4& transform);
	void assignScale(unsigned int object, const mat4& scaling);
	
	void assignCube(unsigned int object, float width, 
					vector<vec3>* mesh,
					vector<vec3>* normals,
					vector<unsigned int>* indices);
	void assignPlane(unsigned int object, float width,
					vector<vec3>* mesh,
					vector<vec3>* normals,
					vector<unsigned int>* indices);
	void assignSphere(unsigned int object, float radius, unsigned int divisions,
					vector<vec3>* mesh,
					vector<vec3>* normals,
					vector<vec2>* uvs,
					vector<unsigned int>* indices);

	void assignSkyDome(unsigned int object, float radius, unsigned int divisions,
					vector<vec3>* mesh,
					vector<vec2>* uvs,
					vector<unsigned int>* indices,
					unsigned int textureID);

	//Lights
	unsigned int generateLightObject();
	void setLightPosition(unsigned int object, vec3 lightPos);
	void assignLightMesh(unsigned int object, vector<vec3>* mesh);
	void assignLightColor(unsigned int object, const vec4& color);
	void assignLightIntensity(unsigned int object, float intensity);
	void enableShadowCasting(unsigned int object);
	void disableShadowCasting(unsigned int object);

	//Drawing calls
	void clearDrawBuffers(vec3 color);
	void draw(unsigned int object);
	void draw(vector<unsigned int> list);	//Preferred over individual calls, easier to optimize
	void drawAll();		//Easiest to optimize
	void drawRadar(vector<vec2> radarVecs);
	void Renderer::drawRadarForSplitScreen(vector<vec2> radarVecs, int numOfPlayers, int playerScreen);

	void drawUI(const vector<vector<vec3>>& segments, vector<vec3> colors);
	//Debugging draw calls
	void drawLines(const vector<vec3>& segments, vec3 color, const mat4& objectTransform);
	void drawPoints(const vector<vec3>& points, vec3 color, const mat4& objectTransform);

	//Viewports
	unsigned int addViewport(float x, float y, float width, float height);	//Set viewport in 0 to 1 floats
	unsigned int addViewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height);	  //Set viewport in pixels
	void useViewport(unsigned int index);
	void splitScreenViewports(unsigned int numViewports);
	mat4 getViewportRatio(unsigned int viewport);
	unsigned int numberOfViewports() { return viewports.size(); }
	Viewport getActiveViewport() { return viewports[activeViewport]; }

	//Delete objects
	void deleteDrawableObject(int object);
	void deleteLightObject(int object);
	void deleteIDs();

	//Update
	void updateObjectTransforms(GameState* state);

	//External buffer loading
	bool loadVertUVBuffer(vector<vec3>* verts, vector<vec2>* uvs);

	//Window dimensions
	unsigned int getHeight();
	unsigned int getWidth();

};

void displayVec4(const vec4& vec);
void displayMat4(const mat4& mat);

void resizeFunc(GLFWwindow* window, int width, int height);

#endif // RENDERER_H