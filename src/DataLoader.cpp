#include "DataLoader.h"
#include <iostream>
#include <fstream>

using namespace std;

DataLoader* DataLoader::m_instance = NULL;

DataLoader::DataLoader()
    : mEnable(false)
    , mTextureId(-1)
{
}

DataLoader::~DataLoader()
{
}

DataLoader* DataLoader::instance()
{
    if(NULL==m_instance)
        m_instance = new DataLoader;
    return m_instance;
}

void DataLoader::createCubeData()
{
    short* buffer = new short[200 * 200 * 200]();
    for (int z = 0; z < 200; z++)
	{
		for (int y = 0; y < 200; y++)
		{
			for (int x = 0; x < 200; x++)
			{
			    if((z<50||z>150)||(x>75&&x<125))
                    buffer[z * 200 * 200 + y * 200 + x] = 800;
                else
                    buffer[z * 200 * 200 + y * 200 + x] = 100;

                int dist2 = (x - 100)*(x - 100) + (y - 100)*(y - 100) + (z - 100)*(z - 100);
                if(dist2<225)
                    buffer[z * 200 * 200 + y * 200 + x] = 1500;
			}
		}
	}

	mDimenRange[0] = 200;
	mDimenRange[1] = 200;
	mDimenRange[2] = 200;
	mValueRange[0] = 0;
	mValueRange[1] = 2000;
    mData = buffer;

    if(!updateTexture())
        std::cout<<"Failed to upload texture."<<std::endl;

    delete []buffer;
}

void DataLoader::getDimension(int& width, int& height, int& depth)
{
    width = this->mDimenRange[0];
    height = this->mDimenRange[1];
    depth = this->mDimenRange[2];
}

void DataLoader::getPixelRange(short& minv, short& maxv)
{
    minv = this->mValueRange[0];
    maxv = this->mValueRange[1];
}

/********************** private methods***************************************/
bool DataLoader::updateTexture()
{
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &mTextureId);
    glBindTexture(GL_TEXTURE_3D, mTextureId);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_R16_SNORM, mDimenRange[0], mDimenRange[1], mDimenRange[2], 0, GL_RED, GL_SHORT, (GLvoid*)mData);

    mEnable = true;
    return mEnable;
}
