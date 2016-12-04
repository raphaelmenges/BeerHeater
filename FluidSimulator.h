#ifndef FLUIDSIMULATOR_H_
#define FLUIDSIMULATOR_H_

#include "Material.h"
#include "Area.h"
#include "Fan.h"
#include "externals/OpenGLLoader/gl_core_4_3.h"
#include <vector>

class FluidSimulator
{
public:
    FluidSimulator(Area &area, const std::vector<Fan> &fanList);
    ~FluidSimulator();

    void nextStep(float dt);
    float getMEdgeLenght(); const
    void setMEdgeLenght(float edgeLenght);
    void setRelaxationSteps(int steps);
    int getRelaxationSteps();

private:
    GLuint mFluidSimulationProgram;
    GLuint mStateVolume;
    GLuint mLookupVolume;
    GLuint mMaterialsSSBO;
    GLuint mFansSSBO;

    int mStateVolumeLocation;
    int mTimestepLocation;
    int mRelaxationStepsLocation;
    int mEdgeLengthLocation;
	int mFanCountLocation;
    int mLookupVolumeLocation;
    float mEdgeLenght;
    int mRelaxationSteps;
	int mFanCount;

    Area* mSimulationArea;

    void prepareShader();
    void prepareMaterialSSBO(const std::vector<Materialtype> &materialList);
    void prepareFansSSBO(const std::vector<Fan> &fanList);

    int mResolution;
    int mVoxelCount;
};

#endif // FLUIDSIMULATOR_H_
