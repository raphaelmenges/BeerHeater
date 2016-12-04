#ifndef AREA_H_
#define AREA_H_

#include "Material.h"
#include "State.h"
#include "externals/OpenGLLoader/gl_core_4_3.h"
#include <vector>

class Area
{
public:

    Area(int resolution, Materialtype materialtype, State startState = state::fallback);
    virtual ~Area();
    void setBlock(Materialtype material, int x, int y, int z, int width, int height, int depth);
    void printColors() const;
    int getResolution() const;
    int getVoxelCount() const;
    Material *getMaterialData();
    GLuint getColorVolumeHandle() const;
    GLuint getLookupVolumeHandle() const;
    GLuint getStateVolumeHandle();
    void setInitialState(float startTemperatur = 0.f);
    Material determineMaterial(const Materialtype &materialtype);
	const std::vector<Materialtype>& getMaterialList() const;

private:
    void updateColorVolume() const;
    void updateLookupVolume() const;

    Material* mpMaterials;
    State* mStartState;
    float* mLookupArray;
    int mResolution;
    int mVoxelCount;
    GLuint mColorVolumeHandle;
    GLuint mStateVolumeHandle;
    GLuint mLookupVolume;
    bool mIsInitialised;
	std::vector<Materialtype> mMaterialList;
};

#endif // AREA_H_
