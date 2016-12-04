#include "HeatSimulator.h"
#include "HeatSimulationShader.h"

#include <iostream>

HeatSimulator::HeatSimulator(Area &area)
{
    mResolution = area.getResolution();
    mVoxelCount = area.getVoxelCount();

    mStateVolume = area.getStateVolumeHandle();
    mLookupVolume = area.getLookupVolumeHandle();

    mSimulationArea = &area;

    mEdgeLenght = 1.f;
    mRelaxationSteps = 5;

    prepareSSBO(area.getMaterialList());
    prepareShader();
}

HeatSimulator::~HeatSimulator()
{
    // Delete shader
    glDeleteProgram(mHeatSimulationProgram);
}

void HeatSimulator::prepareShader()
{
    mHeatSimulationProgram = glCreateProgram();
    GLint heatSimulationCS = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(heatSimulationCS, 1 , &heatSimComputeShader, NULL);
    glCompileShader(heatSimulationCS);

    // Get length of compiling log
    GLint log_length = 0;
    glGetShaderiv(heatSimulationCS, GL_INFO_LOG_LENGTH, &log_length);

    if (log_length > 1)
    {
        // Copy log to chars
        GLchar *log = new GLchar[log_length];
        glGetShaderInfoLog(heatSimulationCS, log_length, NULL, log);

        // Print it
        std::cout << log << std::endl;

        // Delete chars
        delete[] log;
    }

    glAttachShader(mHeatSimulationProgram, heatSimulationCS);
    glLinkProgram(mHeatSimulationProgram);
    glDetachShader(mHeatSimulationProgram, heatSimulationCS);
    glDeleteShader(heatSimulationCS);

    mStateVolumeLocation = glGetUniformLocation(mHeatSimulationProgram, "stateVolume");
    mTimestepLocation = glGetUniformLocation(mHeatSimulationProgram, "timeStep");
    mRelaxationStepsLocation = glGetUniformLocation(mHeatSimulationProgram, "relaxationSteps");
    mEdgeLengthLocation = glGetUniformLocation(mHeatSimulationProgram,"edgeLength");
    mLookupVolumeLocation = glGetUniformLocation(mHeatSimulationProgram, "lookupVolume");
}

void HeatSimulator::nextStep(float dt)
{
	glUseProgram(mHeatSimulationProgram);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mMaterialsSSBO);

    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, mStateVolume);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, mLookupVolume);

    glBindImageTexture(0,
                       mStateVolume,
                       0,
                       GL_TRUE,
                       0,
                       GL_READ_WRITE,
                       GL_RGBA32F);

    glBindImageTexture(1,
                       mLookupVolume,
                       0,
                       GL_TRUE,
                       0,
                       GL_READ_ONLY,
                       GL_R32F);

    // update volume texture<->unit location
    glUniform1i(mStateVolumeLocation, 0);
    glUniform1i(mLookupVolumeLocation, 1);

    // update time step location
    glUniform1f(mTimestepLocation, dt);
    glUniform1f(mEdgeLengthLocation, mEdgeLenght);
    glUniform1i(mRelaxationStepsLocation, mRelaxationSteps);

    glDispatchCompute(mResolution/4,mResolution/4,mResolution/4);

    glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

    glUseProgram(0);

    //glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glBindTexture(GL_TEXTURE_3D,0);
}


void HeatSimulator::prepareSSBO(const std::vector<Materialtype> &materialList)
{
	// Create list of materials
	std::vector<Material> materials;
	for(const Materialtype& type : materialList)
	{
		materials.push_back(mSimulationArea->determineMaterial(type));
	}

	// Copy it to the shader storage buffer object
	glGenBuffers(1, &mMaterialsSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mMaterialsSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Material) * materials.size(), materials.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

float HeatSimulator::getMEdgeLenght() {
    return mEdgeLenght;
}

const
void HeatSimulator::setMEdgeLenght(float edgeLenghth)
{
    if(edgeLenghth > 0.f)
        mEdgeLenght = edgeLenghth;
    else
        mEdgeLenght = 1.f;
}

void HeatSimulator::setRelaxationSteps(int steps)
{
    if(steps > 0)
        mRelaxationSteps = steps;
    else
        mRelaxationSteps = 5;
}

int HeatSimulator::getRelaxationSteps()
{
    return mRelaxationSteps;
}
