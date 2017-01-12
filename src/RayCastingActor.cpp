#include "RayCastingActor.h"
#include "DataLoader.h"
#include <cmath>
#include <iostream>
#include <cstdlib>

#define MAXINT(x,y) (x>y?x:y)
#define MAPSIZE 1000

RayCastingActor::RayCastingActor(TrackballCamera* cam)
	: Drawable()
	, mCamera(cam)
{
}

RayCastingActor::~RayCastingActor()
{
	glDeleteBuffers(1, &vboVerticesID);
	glDeleteBuffers(1, &vboTcoordID);
	glDeleteBuffers(1, &vboIndicesID);
	glDeleteVertexArrays(1, &vaoID);
	mShader.DeleteShaderProgram();
}

void RayCastingActor::initShader()
{
	//init vertices and tcoords
	prepareVertexAndTcoord();

	mShader.LoadFromFile(GL_VERTEX_SHADER, "./shaders/rc.vert");
	mShader.LoadFromFile(GL_FRAGMENT_SHADER, "./shaders/rc.frag");
	mShader.CreateAndLinkProgram();

	mShader.Use();
	mShader.AddAttribute("vVertex");
	mShader.AddAttribute("vTcoord");

	mShader.AddUniform("texMap");
	mShader.AddUniform("MVP");
	mShader.AddUniform("mingray");
	mShader.AddUniform("maxgray");
	mShader.AddUniform("cameraPosition");
	mShader.AddUniform("volumeSizeRatio");

	glUniform1i(mShader("texMap"), 0);
	mShader.UnUse();

	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID);
	glGenBuffers(1, &vboTcoordID);
	glBindVertexArray(vaoID);

	glBindBuffer(GL_ARRAY_BUFFER, vboVerticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mVertices), &mVertices[0], GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(mShader["vVertex"]);
	glVertexAttribPointer(mShader["vVertex"], 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vboTcoordID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mTcoord), &mTcoord[0], GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(mShader["vTcoord"]);
	glVertexAttribPointer(mShader["vTcoord"], 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(mIndices), &mIndices[0], GL_STATIC_DRAW);
}

void RayCastingActor::render()
{
	if (!mIsActive) return;

	glEnable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(vaoID);
	mShader.Use();

	glm::vec3 localCameraPos = glm::vec3(glm::inverse(mCamera->GetModal())*glm::vec4(mCamera->GetCameraPosition(),1));
	glUniform3fv(mShader("cameraPosition"), 1, glm::value_ptr(localCameraPos));
	glUniformMatrix4fv(mShader("MVP"), 1, GL_FALSE, glm::value_ptr(mCamera->GetMVP()));
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
	mShader.UnUse();

	glBindVertexArray(0);
}

void RayCastingActor::updateData()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, DataLoader::instance()->textureId());

    int width, height, depth;
    DataLoader::instance()->getDimension(width,height,depth);
    updateVertexAndTcoord(width,height,depth);

    short minvalue, maxvalue;
    DataLoader::instance()->getPixelRange(minvalue,maxvalue);
    updatePixelClamp(minvalue,maxvalue);
}

/******************************** private methods **********************************************************/
void RayCastingActor::prepareVertexAndTcoord()
{
	//
	mVertices[0] = glm::vec3(-1, -1, -1);
	mVertices[1] = glm::vec3(1, -1, -1);
	mVertices[2] = glm::vec3(1, 1, -1);
	mVertices[3] = glm::vec3(-1, 1, -1);
	mVertices[4] = glm::vec3(-1, -1, 1);
	mVertices[5] = glm::vec3(1, -1, 1);
	mVertices[6] = glm::vec3(1, 1, 1);
	mVertices[7] = glm::vec3(-1, 1, 1);

	mTcoord[0] = glm::vec3(0, 0, 0);
	mTcoord[1] = glm::vec3(1, 0, 0);
	mTcoord[2] = glm::vec3(1, 1, 0);
	mTcoord[3] = glm::vec3(0, 1, 0);
	mTcoord[4] = glm::vec3(0, 0, 1);
	mTcoord[5] = glm::vec3(1, 0, 1);
	mTcoord[6] = glm::vec3(1, 1, 1);
	mTcoord[7] = glm::vec3(0, 1, 1);

	mIndices[0] = 0; mIndices[1] = 5; mIndices[2] = 4; mIndices[3] = 5; mIndices[4] = 0; mIndices[5] = 1;
	mIndices[6] = 3; mIndices[7] = 7; mIndices[8] = 6; mIndices[9] = 3; mIndices[10] = 6; mIndices[11] = 2;
	mIndices[12] = 7; mIndices[13] = 4; mIndices[14] = 6; mIndices[15] = 6; mIndices[16] = 4; mIndices[17] = 5;
	mIndices[18] = 2; mIndices[19] = 1; mIndices[20] = 3; mIndices[21] = 3; mIndices[22] = 1; mIndices[23] = 0;
	mIndices[24] = 3; mIndices[25] = 0; mIndices[26] = 7; mIndices[27] = 7; mIndices[28] = 0; mIndices[29] = 4;
	mIndices[30] = 6; mIndices[31] = 5; mIndices[32] = 2; mIndices[33] = 2; mIndices[34] = 5; mIndices[35] = 1;
}

void RayCastingActor::updateVertexAndTcoord(int w, int h, int d)
{
	float maxDim = (float)MAXINT(MAXINT(w, h), d);
	float X = (float)w / maxDim;
	float Y = (float)h / maxDim;
	float Z = (float)d / maxDim;
	mVertices[0] = glm::vec3(-X, -Y, -Z);
	mVertices[1] = glm::vec3(X, -Y, -Z);
	mVertices[2] = glm::vec3(X, Y, -Z);
	mVertices[3] = glm::vec3(-X, Y, -Z);
	mVertices[4] = glm::vec3(-X, -Y, Z);
	mVertices[5] = glm::vec3(X, -Y, Z);
	mVertices[6] = glm::vec3(X, Y, Z);
	mVertices[7] = glm::vec3(-X, Y, Z);

	mShader.Use();
	glUniform3fv(mShader("volumeSizeRatio"),1,glm::value_ptr(glm::vec3(X,Y,Z)));
	mShader.UnUse();

	glBindVertexArray(vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, vboVerticesID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(mVertices), &mVertices[0]);

}

void RayCastingActor::updatePixelClamp(short minValue, short maxValue)
{
	mShader.Use();
	glUniform1f(mShader("mingray"), (float)minValue / 32768.0f);
	glUniform1f(mShader("maxgray"), (float)maxValue / 32768.0f);
	mShader.UnUse();
}
