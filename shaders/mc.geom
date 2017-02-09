#version 330 core

layout(points) in;
layout(triangle_strip,max_vertices=15) out;

uniform float mingray;
uniform float maxgray;
uniform float isovalue;	//threshold
uniform float deviation;//error tolerance, so the threshold is [isovalue-deviation,isovalue+deviation]

uniform vec3 sizeRatio;	//width:height:depth

uniform sampler3D  cubeMap;		//volume data
uniform isampler2D triTable;
uniform isampler2D edvTable;

uniform vec3 texOffsetMap[8];

uniform mat4 MVP; 	//projection*view*model
uniform mat4 MV; 	//view*model matrix	
uniform mat3 N;  	//normal matrix

smooth out vec3 eyeSpacePosition;
smooth out vec3 eyeSpaceNormal;

//get value of vertex i 
float getValue(vec3 texPos, int i)
{
	return (texture(cubeMap,texPos+texOffsetMap[i]).r-mingray)/(maxgray-mingray);
}

//get position of vertex i
vec3 getPosition(vec3 cubePos, int i)
{
	return cubePos+texOffsetMap[i]*sizeRatio*2.0;
}

//get normal of vertex i
vec3 getNormal(vec3 texPos, int i)
{
	vec3 vPos = texPos+texOffsetMap[i];
	float deltaX = abs(texOffsetMap[0].x)*2.0f;
	float deltaY = abs(texOffsetMap[0].y)*2.0f;
	float deltaZ = abs(texOffsetMap[0].z)*2.0f;
	vec3 X0 = vec3(vPos.x-deltaX,vPos.yz);
	vec3 X1 = vec3(vPos.x+deltaX,vPos.yz);
	vec3 Y0 = vec3(vPos.x,vPos.y-deltaY,vPos.z);
	vec3 Y1 = vec3(vPos.x,vPos.y+deltaY,vPos.z);
	vec3 Z0 = vec3(vPos.xy,vPos.z-deltaZ);
	vec3 Z1 = vec3(vPos.xy,vPos.z+deltaZ);
	
	float vx0 = (texture(cubeMap,X0).r-mingray)/(maxgray-mingray);
	float vx1 = (texture(cubeMap,X1).r-mingray)/(maxgray-mingray);
	float vy0 = (texture(cubeMap,Y0).r-mingray)/(maxgray-mingray);
	float vy1 = (texture(cubeMap,Y1).r-mingray)/(maxgray-mingray);
	float vz0 = (texture(cubeMap,Z0).r-mingray)/(maxgray-mingray);
	float vz1 = (texture(cubeMap,Z1).r-mingray)/(maxgray-mingray);
	
	float dX = (vx1-vx0)/deltaX/2.0f;
	float dY = (vy1-vy0)/deltaY/2.0f;
	float dZ = (vz1-vz0)/deltaZ/2.0f;
	
	return normalize(vec3(dX,dY,dZ));
}

//calculate the point position on the edge 
vec3 interpPosition(vec3 p0, float v0, vec3 p1, float v1, float level)
{
	//return mix(p0,p1,(level-v0)/(v1-v0));
	return (p0+p1)*0.5f;
}

//calculate the point normal on the edge 
vec3 interpNormal(vec3 n0, float v0, vec3 n1, float v1, float level)
{
	//return mix(n0,n1,(level-v0)/(v1-v0));
	return (n0+n1)*0.5f;
}

//get edge index from triangle table
int getEdgeIndex(int x, int y)
{
	return texelFetch(triTable, ivec2(x, y), 0).r; 
}

//get endpoint index from edge-vertex table
int getEdgeEnd(int x, int y)
{
	return texelFetch(edvTable, ivec2(x, y), 0).r;
}

void main()
{
	vec3 localPos = gl_in[0].gl_Position.xyz;
	
	//the width, height, and depth may be not equal
	vec3 tempPos = vec3(localPos.x/sizeRatio.x,localPos.y/sizeRatio.y,localPos.z/sizeRatio.z);
	
	//position in texture coordinate system
	vec3 texCoord = tempPos/2.0f+0.5f; 
		
	float 	cubeVertexValue[8];
	vec3 	cubeVertexPosition[8];
	vec3 	cubeVertexNormal[8];
	
	float isolevel = (isovalue-mingray)/(maxgray-mingray);//normalize
	float isolevel_low = (isovalue-deviation-mingray)/(maxgray-mingray);//normalize
	float isolevel_hig = (isovalue+deviation-mingray)/(maxgray-mingray);//normalize
	
	int 	cubeIndex = 0;

	for(int i=0,j=1; i<8; i++,j*=2)
	{
		//vertex position
		cubeVertexPosition[i] = getPosition(localPos,i);
		
		//vertex normal
		cubeVertexNormal[i] = getNormal(texCoord,i);
		
		//vertex value
		cubeVertexValue[i] = getValue(texCoord,i);
		
		cubeIndex += int((cubeVertexValue[i]>isolevel_low) && (cubeVertexValue[i]<isolevel_hig))*j;	
	}
	if(cubeIndex==0 || cubeIndex==255)
		return;
	
	//get intersection points on edges
	vec3 	edgePoints[12];
	vec3	edgeNormal[12];
	for(int i=0;i<12;i++)
	{
		int index0 = getEdgeEnd(0,i);
		int index1 = getEdgeEnd(1,i);
		edgePoints[i] = interpPosition(cubeVertexPosition[index0],cubeVertexValue[index0],
									cubeVertexPosition[index1],cubeVertexValue[index1],isolevel);
		
		edgeNormal[i] = interpNormal(cubeVertexNormal[index0],cubeVertexValue[index0],
									cubeVertexNormal[index1],cubeVertexValue[index1],isolevel);
	}
	
	//create new vertices
	int pointIndex = 0;
	while(pointIndex<15)
	{
		int edge0 = getEdgeIndex(pointIndex,cubeIndex);
		if(edge0==-1) 
			return;
		//vertex 1
		gl_Position = MVP*vec4(edgePoints[edge0],1);
		eyeSpacePosition = (MV*vec4(edgePoints[edge0],1)).xyz;
		eyeSpaceNormal = N*edgeNormal[edge0];
		EmitVertex();
		
		//vertex 2
		int edge1 = getEdgeIndex(pointIndex+1,cubeIndex);
		gl_Position = MVP*vec4(edgePoints[edge1],1);
		eyeSpacePosition = (MV*vec4(edgePoints[edge1],1)).xyz;
		eyeSpaceNormal = N*edgeNormal[edge1];
		EmitVertex();
		
		//vertex 3
		int edge2 = getEdgeIndex(pointIndex+2,cubeIndex);
		gl_Position = MVP*vec4(edgePoints[edge2],1);
		eyeSpacePosition = (MV*vec4(edgePoints[edge2],1)).xyz;
		eyeSpaceNormal = N*edgeNormal[edge2];
		EmitVertex();

		EndPrimitive();
		
		pointIndex+=3;		
	} 
	
}