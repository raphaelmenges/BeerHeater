#ifndef HEATSIMULATOR_H_
#define HEATSIMULATOR_H_

#include "Material.h"
#include "Area.h"
#include "externals/OpenGLLoader/gl_core_4_3.h"
#include <vector>

class HeatSimulator
{
public:
    HeatSimulator(Area &area);
    ~HeatSimulator();

    void nextStep(float dt);
    float getMEdgeLenght(); const
    void setMEdgeLenght(float edgeLenght);
    void setRelaxationSteps(int steps);
    int getRelaxationSteps();

private:
	void prepareShader();
	void prepareSSBO(const std::vector<Materialtype> &materialList);

    GLuint mHeatSimulationProgram;
    GLuint mStateVolume;
    GLuint mLookupVolume;
    GLuint mMaterialsSSBO;
    int mStateVolumeLocation;
    int mTimestepLocation;
    int mRelaxationStepsLocation;
    int mEdgeLengthLocation;
    int mLookupVolumeLocation;
    float mEdgeLenght;
    int mRelaxationSteps;
    Area* mSimulationArea;
    int mResolution;
    int mVoxelCount;
};


#endif //HEATSIMULATOR_H_
