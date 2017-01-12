#ifndef DATALOADER_H
#define DATALOADER_H

#include <GL/glew.h>

class DataLoader
{
public:
    static DataLoader* instance();
    virtual ~DataLoader();

    bool isEnable() const {return mEnable;}

    void loadData(const char* path=NULL);

    void createCubeData();

    void getDimension(int& w, int& h, int& d);

    void getPixelRange(short& minv, short& maxv);

    GLuint textureId() const {return mTextureId;}

private:
    DataLoader();

    bool updateTexture();

private:
    static DataLoader* m_instance;
    bool   mEnable;

    int    mDimenRange[3];//X,Y,Z
    short  mValueRange[2];//minimum value, maximum value
    short* mData;

    GLuint mTextureId;
};

#endif // DATALOADER_H
