#version 430 core

out vec4 FragColor;

uniform vec3 lightPos;
uniform mat4 V;		//view matrix
uniform vec3 diffuseColor;

smooth in vec3 eyeSpacePosition;

smooth in vec3 eyeSpaceNormal;

void main()
{
	vec4 diffuseColor = vec4(1,0.49,0.25,0.8);
	
	vec3 eyeSpaceLight = (V*vec4(lightPos,1)).xyz;
	
	vec3 lightDir = normalize(eyeSpaceLight-eyeSpacePosition);
	
	float diffuse = max(0,dot(eyeSpaceNormal,lightDir)); 
	
	vec3 viewDir = normalize(-eyeSpacePosition);
	vec3 reflectDir = reflect(-lightDir,eyeSpaceNormal);
	
	float spec = pow(max(dot(viewDir,reflectDir),0),32);
	
	FragColor = (diffuse+spec)*diffuseColor;
	
}