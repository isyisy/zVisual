#version 330 core

in vec3 vVertex;
in vec3 vTcoord;
uniform mat4 MVP;
//uniform vec3 cameraPosition;
smooth out vec3 vUV;
//out vec3  campos;

void main()
{
	gl_Position = MVP*vec4(vVertex, 1);
	vUV = vTcoord; 
}