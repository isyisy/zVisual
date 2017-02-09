#ifndef MARCHINGCUBES_H
#define MARCHINGCUBES_H

#include "GLSLShader.h"
#include "Drawable.h"
#include "TrackballCamera.h"
#include "GL/glm/glm.hpp"
#include "GL/glm/gtc/matrix_transform.hpp"
#include "GL/glm/gtc/type_ptr.hpp"

#include <vector>

class MarchingCubesActor :public Drawable
{
public:
	MarchingCubesActor(TrackballCamera* cam=NULL);

	~MarchingCubesActor();

	//init shaders and vertice data
	virtual void initShader();

	//update data
	virtual void updateData();

	//render in timeInterval
	virtual void render();

	void setIsoValue(const short level);

	short getIsoValue() const { return mIsoValue; }

private:
	void prepareVertices();

	void updateSizeRatio(int,int,int);

	void updatePixelClamp(short minValue, short maxValue);

private:
    TrackballCamera* mCamera;

	std::vector<glm::vec3>			mVertices;

	GLSLShader  mShader;
	GLuint vaoID;
	GLuint vboVerticesID;

	GLuint mTableTex;
	GLuint mEdgeEndsTex;//edge -- end points index

	int mSteps[3];

	short mIsoValue;
};

#endif
