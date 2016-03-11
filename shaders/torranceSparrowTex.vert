#version 330 core
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 uv;

uniform mat4 transform;
uniform mat4 objectTransform;

out vec3 worldPosition;
out vec3 fragNormal;
out vec2 texCoord;

void main()
{
	texCoord = uv;
	fragNormal = (objectTransform*vec4(vertexNormal, 0)).xyz;
    worldPosition = (objectTransform*vec4(vertexPosition, 1)).xyz;
 	gl_Position =  transform*vec4(vertexPosition, 1);
}
