#include "FluidSimulator.h"
#include "FluidSimulationShader.h"

#include <iostream>

FluidSimulator::FluidSimulator(Area &area, const std::vector<Fan> &fanList)
{
    mResolution = area.getResolution();
    mVoxelCount = area.getVoxelCount();

    mStateVolume = area.getStateVolumeHandle();
    mLookupVolume = area.getLookupVolumeHandle();

    mSimulationArea = &area;

    mEdgeLenght = 1.f;
    mRelaxationSteps = 5;
	mFanCount = (int)fanList.size();

    prepareMaterialSSBO(area.getMaterialList());
    prepareFansSSBO(fanList);
    prepareShader();
}

FluidSimulator::~FluidSimulator()
{
    // Delete shader
    glDeleteProgram(mFluidSimulationProgram);
}

void FluidSimulator::prepareShader()
{
    mFluidSimulationProgram = glCreateProgram();
    GLint fluidSimulationCS = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(fluidSimulationCS, 1, &fluidSimComputeShader, NULL);
    glCompileShader(fluidSimulationCS);

    // Get length of compiling log
    GLint log_length = 0;
    glGetShaderiv(fluidSimulationCS, GL_INFO_LOG_LENGTH, &log_length);

    if (log_length > 1)
    {
        // Copy log to chars
        GLchar *log = new GLchar[log_length];
        glGetShaderInfoLog(fluidSimulationCS, log_length, NULL, log);

        // Print it
        std::cout << log << std::endl;

        // Delete chars
        delete[] log;
    }

    glAttachShader(mFluidSimulationProgram, fluidSimulationCS);
    glLinkProgram(mFluidSimulationProgram);
    glDetachShader(mFluidSimulationProgram, fluidSimulationCS);
    glDeleteShader(fluidSimulationCS);

    mStateVolumeLocation = glGetUniformLocation(mFluidSimulationProgram, "stateVolume");
    mTimestepLocation = glGetUniformLocation(mFluidSimulationProgram, "timeStep");
    mRelaxationStepsLocation = glGetUniformLocation(mFluidSimulationProgram, "relaxationSteps");
    mEdgeLengthLocation = glGetUniformLocation(mFluidSimulationProgram, "edgeLength");
	mFanCountLocation = glGetUniformLocation(mFluidSimulationProgram, "fanCount");
    mLookupVolumeLocation = glGetUniformLocation(mFluidSimulationProgram, "lookupVolume");
}

void FluidSimulator::nextStep(float dt)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mMaterialsSSBO);

	if (mFanCount > 0)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mFansSSBO);
	}

    glUseProgram(mFluidSimulationProgram);

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

    // update volume texture <-> unit location
    glUniform1i(mStateVolumeLocation, 0);
    glUniform1i(mLookupVolumeLocation, 1);

    // fill uniforms
    glUniform1f(mTimestepLocation, dt);
    glUniform1f(mEdgeLengthLocation, mEdgeLenght);
    glUniform1i(mRelaxationStepsLocation, mRelaxationSteps);
	glUniform1i(mFanCountLocation, mFanCount);

    glDispatchCompute(mResolution / 8, mResolution / 8, mResolution / 8);

    glUseProgram(0);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}


void FluidSimulator::prepareMaterialSSBO(const std::vector<Materialtype> &materialList)
{
	// Create list of materials
	std::vector<Material> materials;
	for (const Materialtype& type : materialList)
	{
		materials.push_back(mSimulationArea->determineMaterial(type));
	}

	// Copy it to the shader storage buffer object
	glGenBuffers(1, &mMaterialsSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mMaterialsSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Material) * materials.size(), materials.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void FluidSimulator::prepareFansSSBO(const std::vector<Fan> &fanList)
{
	if (fanList.size() > 0)
	{
		// Create structs with fans
		struct fanStruct
		{
			glm::vec3 position;
			float speed;
			glm::vec3 direction;
			float distance;

			fanStruct(glm::vec3 position,
			float speed,
			glm::vec3 direction,
			float distance)
			{
				this->position = position;
				this->speed = speed;
				this->direction = direction;
				this->distance = distance;
			}
		};

		std::vector<fanStruct> fanStructs;
		for (const Fan& fan : fanList)
		{
			fanStructs.push_back(fanStruct(
				fan.getPosition(),
				fan.getSpeed(),
				fan.getDirection(),	
				fan.getDistance()));
		}

		// Fill into ssbo
		glGenBuffers(1, &mFansSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, mFansSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(fanStruct) * fanStructs.size(), fanStructs.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
}


float FluidSimulator::getMEdgeLenght() {
    return mEdgeLenght;
}

const
void FluidSimulator::setMEdgeLenght(float edgeLenghth)
{
    if (edgeLenghth > 0.f)
        mEdgeLenght = edgeLenghth;
    else
        mEdgeLenght = 1.f;
}

void FluidSimulator::setRelaxationSteps(int steps)
{
    if (steps > 0)
        mRelaxationSteps = steps;
    else
        mRelaxationSteps = 5;
}

int FluidSimulator::getRelaxationSteps()
{
    return mRelaxationSteps;
}
