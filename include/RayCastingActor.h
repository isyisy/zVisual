#ifndef RAYCASTING_H
#define RAYCASTING_H

#include "GLSLShader.h"
#include "Drawable.h"
#include "TrackballCamera.h"
#include "GL/glm/glm.hpp"
#include "GL/glm/gtc/matrix_transform.hpp"
#include "GL/glm/gtc/type_ptr.hpp"

class RayCastingActor :public Drawable
{
public:
	RayCastingActor(TrackballCamera* cam=NULL);

	~RayCastingActor();

	//init shaders and vertice data
	virtual void initShader();

	//load data
	virtual void updateData();

	//render in timeInterval
	virtual void render();

private:
	void prepareVertexAndTcoord();

	void updateVertexAndTcoord(int w,int h,int d);

	void updatePixelClamp(short minValue, short maxValue);

private:

	TrackballCamera* mCamera;

	glm::vec3			mVertices[8];
	glm::vec3			mTcoord[8];
	GLushort			mIndices[36];

	GLSLShader  mShader;

	GLuint vaoID;
	GLuint vboVerticesID;
	GLuint vboTcoordID;
	GLuint vboIndicesID;
};

#endif
