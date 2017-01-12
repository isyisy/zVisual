#include "MarchingCubesActor.h"
#include "DataLoader.h"
#include "LookupTable.h"
#include "common.h"

#define MAXSTEPS 200

MarchingCubesActor::MarchingCubesActor(TrackballCamera* cam)
	: Drawable()
	, mCamera(cam)
	, mIsoValue(1500)
{
	mSteps[0] = MAXSTEPS;
	mSteps[1] = MAXSTEPS;
	mSteps[2] = MAXSTEPS;
}

MarchingCubesActor::~MarchingCubesActor()
{
	glDeleteBuffers(1, &vboVerticesID);
	glDeleteVertexArrays(1, &vaoID);

	//delete table texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, 0);
	glDeleteTextures(1, &mTableTex);
	glDeleteTextures(1, &mEdgeEndsTex);
	mShader.DeleteShaderProgram();
}

void MarchingCubesActor::initShader()
{
	prepareVertices();

	mShader.LoadFromFile(GL_VERTEX_SHADER, "./shaders/mc.vert");
	mShader.LoadFromFile(GL_FRAGMENT_SHADER, "./shaders/mc.frag");
	mShader.LoadFromFile(GL_GEOMETRY_SHADER, "./shaders/mc.geom");
	mShader.CreateAndLinkProgram();

	mShader.Use();
	mShader.AddAttribute("vVertex");

	mShader.AddUniform("MVP");
	mShader.AddUniform("MV");
	mShader.AddUniform("N");
	mShader.AddUniform("V");

	mShader.AddUniform("cubeMap");//texture0
	mShader.AddUniform("triTable");//texture1
	mShader.AddUniform("edvTable");//texture2

	mShader.AddUniform("mingray");
	mShader.AddUniform("maxgray");
	mShader.AddUniform("isovalue");
	mShader.AddUniform("deviation");
	mShader.AddUniform("sizeRatio");

	mShader.AddUniform("lightPos");

	mShader.AddUniform("texOffsetMap");//8*1 vec3

	glUniform1i(mShader("cubeMap"), 0);
	glUniform1i(mShader("triTable"), 1);
	glUniform1i(mShader("edvTable"), 2);
	glUniform3fv(mShader("lightPos"), 1, glm::value_ptr(glm::vec3(0, 1.2, 4)));
	mShader.UnUse();

	//cube vertex
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);

	glBindVertexArray(vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, vboVerticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*mVertices.size(), (const void*)&mVertices.at(0), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(mShader["vVertex"]);
	glVertexAttribPointer(mShader["vVertex"], 3, GL_FLOAT, GL_FALSE, 0, 0);

	//tri table
	glGenTextures(1, &mTableTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mTableTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16I, 16, 256, 0, GL_RED_INTEGER, GL_INT, &triTable[0][0]);

	//tri table
	glGenTextures(1, &mEdgeEndsTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mEdgeEndsTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16I, 2, 12, 0, GL_RED_INTEGER, GL_INT, &edgeEndsTable[0][0]);

}

void MarchingCubesActor::render()
{
	glBindVertexArray(vaoID);

	mShader.Use();
	glUniformMatrix4fv(mShader("MVP"), 1, GL_FALSE, glm::value_ptr(mCamera->GetMVP()));
	glUniformMatrix4fv(mShader("MV"), 1, GL_FALSE, glm::value_ptr(mCamera->GetMV()));
	glUniformMatrix4fv(mShader("V"), 1, GL_FALSE, glm::value_ptr(mCamera->GetView()));
	glm::mat3 NormalMatrix = glm::transpose(glm::inverse(glm::mat3(mCamera->GetMV())));
	glUniformMatrix3fv(mShader("N"), 1, GL_FALSE, glm::value_ptr(NormalMatrix));

	glBindBuffer(GL_ARRAY_BUFFER, vboVerticesID);
	glDrawArrays(GL_POINTS, 0, mVertices.size());
	mShader.UnUse();

	glBindVertexArray(0);
}

void MarchingCubesActor::updateData()
{
    glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, DataLoader::instance()->textureId());

	int width, height, depth;
    DataLoader::instance()->getDimension(width,height,depth);
    updateSizeRatio(width,height,depth);

    short minvalue, maxvalue;
    DataLoader::instance()->getPixelRange(minvalue,maxvalue);
    updatePixelClamp(minvalue,maxvalue);
}

void MarchingCubesActor::updateSizeRatio(int w, int h, int d)
{
	float maxDim = (float)UMAX(UMAX(w, h), d);
	float X = (float)w / maxDim;
	float Y = (float)h / maxDim;
	float Z = (float)d / maxDim;

    //mSteps[0] = w;
    //mSteps[1] = h;
    //mSteps[2] = d;
	//float stepSize = X/mSteps[0]; //stepSize are equal in three directions

	float stepX = X / (int)mSteps[0];
	float stepY = Y / (int)mSteps[0];
	float stepZ = Z / (int)mSteps[0];

	float texOffset[24] = { -stepX / 2.0f, -stepY / 2.0f, -stepZ / 2.0f,
		stepX / 2.0f, -stepY / 2.0f, -stepZ / 2.0f,
		stepX / 2.0f, stepY / 2.0f, -stepZ / 2.0f,
		-stepX / 2.0f, stepY / 2.0f, -stepZ / 2.0f,
		-stepX / 2.0f, -stepY / 2.0f, stepZ / 2.0f,
		stepX / 2.0f, -stepY / 2.0f, stepZ / 2.0f,
		stepX / 2.0f, stepY / 2.0f, stepZ / 2.0f,
		-stepX / 2.0f, stepY / 2.0f, stepZ / 2.0f, };

	int index = 0;
	for (float z = -Z; z <= Z; z += stepZ*2.0)
	{
		for (float y = -Y; y <= Y; y += stepY*2.0)
		{
			for (float x = -X; x <= X; x += stepX*2.0)
			{
				mVertices[index++] = glm::vec3(x, y, z);
			}
		}
	}

	glBindVertexArray(vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, vboVerticesID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3)*mVertices.size(), &mVertices.at(0));

	mShader.Use();
	glUniform3fv(mShader("sizeRatio"), 1, glm::value_ptr(glm::vec3(X, Y, Z)));
	glUniform3fv(mShader("texOffsetMap"), 8, texOffset);
	mShader.UnUse();
}

void MarchingCubesActor::updatePixelClamp(short minValue, short maxValue)
{
	mShader.Use();
	glUniform1f(mShader("mingray"), (float)minValue / 32768.0f);
	glUniform1f(mShader("maxgray"), (float)maxValue / 32768.0f);
	glUniform1f(mShader("isovalue"), (float)mIsoValue/32768.0f);
	glUniform1f(mShader("deviation"), (float)10.0f / 32768.0f);
	mShader.UnUse();
}

void MarchingCubesActor::setIsoValue(short level)
{
	mIsoValue = level;
	mShader.Use();
	glUniform1f(mShader("isovalue"), (float)mIsoValue / 32768.0f);
	mShader.UnUse();
}

/********************************************************/
void MarchingCubesActor::prepareVertices()
{
	float stepX = 1.0 / (float)mSteps[0];
	float stepY = 1.0 / (float)mSteps[0];
	float stepZ = 1.0 / (float)mSteps[0];
	for (float z = -1.0; z <= 1.0; z += stepZ*2.0)
	{
		for (float y = -1.0; y <= 1.0; y += stepY*2.0)
		{
			for (float x = -1.0; x <= 1.0; x += stepX*2.0)
			{
				mVertices.push_back(glm::vec3(x, y, z));
			}
		}
	}
}
