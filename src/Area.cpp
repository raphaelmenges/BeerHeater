#include "Area.h"

#include <iostream>
#include <algorithm>

Area::Area(int resolution, Materialtype materialtype, State startState) : mIsInitialised(false)
{
    // Save resolutioin
    mResolution = resolution;
    mVoxelCount = resolution * resolution * resolution;

    // Initialize volumes and lookup data
    mpMaterials = new Material[mVoxelCount];
    mStartState = new State[mVoxelCount];
    mLookupArray = new float[mVoxelCount];

    // Prepare data
    std::fill_n(mpMaterials, mVoxelCount, determineMaterial(materialtype));
    std::fill_n(mStartState, mVoxelCount, startState);
    std::fill_n(mLookupArray, mVoxelCount, static_cast<float>(materialtype));

    // Initialize texture
    glGenTextures(1, &mColorVolumeHandle);
    glGenTextures(1, &mStateVolumeHandle);
    glGenTextures(1, &mLookupVolume);

    // Prepare list of available materials
    mMaterialList.push_back(Materialtype::AIR);
    mMaterialList.push_back(Materialtype::IRON);
    mMaterialList.push_back(Materialtype::HEATER);
    mMaterialList.push_back(Materialtype::COPPER);
    mMaterialList.push_back(Materialtype::ISOLATOR);
    mMaterialList.push_back(Materialtype::BEER);
    mMaterialList.push_back(Materialtype::GLASS);
    mMaterialList.push_back(Materialtype::FOAM);
    mMaterialList.push_back(Materialtype::DIAMOND);
    mMaterialList.push_back(Materialtype::ZINC);
}

Area::~Area()
{
    // Delete data
    glDeleteTextures(1, &mColorVolumeHandle);
    glDeleteTextures(1, &mStateVolumeHandle);
    glDeleteFramebuffers(1, &mLookupVolume);

    delete[] mpMaterials;
    delete[] mStartState;
    delete[] mLookupArray;
}

void Area::setBlock(Materialtype materialtype, int x, int y, int z, int width, int height, int depth)
{
    for(int i = 0; i < width; i++)
    {
        for(int j = 0; j < height; j++)
        {
            for(int k = 0; k < depth; k++)
            {
                mpMaterials[(x+i) + (y+j) * mResolution + (z+k) * mResolution * mResolution] = determineMaterial(materialtype);
                mLookupArray[(x+i) + (y+j) * mResolution + (z+k) * mResolution * mResolution] = static_cast<float>(materialtype);
            }
        }
    }
}

void Area::printColors() const
{
    for(int i = 0; i < mVoxelCount; i++)
    {
        std::cout << "(" << mpMaterials[i].color.r << ", " << mpMaterials[i].color.g << ", " << mpMaterials[i].color.b << ", " << mpMaterials[i].color.a << ")" << std::endl;
    }
}

 int Area::getResolution() const
 {
    return mResolution;
 }

 int Area::getVoxelCount() const
 {
    return mVoxelCount;
 }

 GLuint Area::getColorVolumeHandle() const
 {
    updateColorVolume();
    return mColorVolumeHandle;
 }

GLuint Area::getLookupVolumeHandle() const
{
    updateLookupVolume();
    return mLookupVolume;
}

GLuint Area::getStateVolumeHandle()
{
    if(!mIsInitialised)
        setInitialState();

    return mStateVolumeHandle;
}

 void Area::updateColorVolume() const
 {
    // Copy color information to new array
    unsigned char* pColorData = new unsigned char[mVoxelCount * 4];
    for(int i = 0; i < mVoxelCount; i++)
    {
        pColorData[4*i] = (unsigned char) (255 * mpMaterials[i].color.r);
        pColorData[4*i+1] = (unsigned char)(255* mpMaterials[i].color.g);
        pColorData[4*i+2] = (unsigned char) (255 *  mpMaterials[i].color.b);
        pColorData[4*i+3] = (unsigned char) (255 * mpMaterials[i].color.a);
    }

    // Create volume 3D texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, mColorVolumeHandle);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);  // Border is assumpted to zero
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, mResolution, mResolution, mResolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, pColorData);
    glBindTexture(GL_TEXTURE_3D, 0);

    // Get rid of array
    delete[] pColorData;
}

void Area::setInitialState(float startTemperatur)
{
    float * stateData = new float[mVoxelCount * 4];

    for(int i=0;i<mVoxelCount;i++)
    {
        stateData[4*i]   = mStartState[i].temperature;
        stateData[4*i+1] = mStartState[i].velocityX;
        stateData[4*i+2] = mStartState[i].velocityY;
        stateData[4*i+3] = mStartState[i].velocityZ;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, mStateVolumeHandle);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D,0,GL_RGBA32F,mResolution,mResolution,mResolution,0,GL_RGBA,GL_FLOAT,stateData);
    glBindTexture(GL_TEXTURE_3D,0);

    delete []stateData;

    mIsInitialised = true;
}

void Area::updateLookupVolume() const
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, mLookupVolume);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage3D(GL_TEXTURE_3D,0,GL_R32F,mResolution,mResolution,mResolution,0, GL_RED, GL_FLOAT, mLookupArray);
    glBindTexture(GL_TEXTURE_3D,0);
}

Material* Area::getMaterialData()
{
    return mpMaterials;
}

Material Area::determineMaterial(const Materialtype &materialtype)
{
    switch (materialtype)
    {
    case Materialtype::AIR:
        return material::air;
    case Materialtype::HEATER:
        return material::heater;
    case Materialtype::COPPER:
        return material::copper;
    case Materialtype::IRON:
        return material::iron;
    case Materialtype::ISOLATOR:
        return material::isolator;
    case Materialtype::BEER:
        return material::beer;
    case Materialtype::GLASS:
        return material::glass;
    case Materialtype::FOAM:
        return material::foam;
        case Materialtype::ZINC:
        return material::zinc;
    case Materialtype::DIAMOND:
        return  material::diamond;
    default:
        return material::iron;
    }
}

const std::vector<Materialtype>& Area::getMaterialList() const
{
    return mMaterialList;
}