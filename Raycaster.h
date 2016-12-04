#ifndef RAYCASTER_H_
#define RAYCASTER_H_

#include "Material.h"
#include "Area.h"
#include "externals/OpenGLLoader/gl_core_4_3.h"
#include "externals/GLM/glm/glm.hpp"
#include <iostream>
#include <fstream>
#include <vector>

class Raycaster
{
public:

    Raycaster(GLuint colorVolumeHandle, GLuint stateVolumeHandle);
    virtual ~Raycaster();

    void draw(const glm::mat4& uniformView, const glm::mat4& uniformProjection, const glm::vec3& cameraPosition) const;

    void toggleRenderEnvironment();
    void toggleRenderTemperature();
    void toggleRenderVelocity();

private:

    void recompileShader();

    GLuint mUniformModelHandle;
    GLuint mUniformViewHandle;
    GLuint mUniformProjectionHandle;
    GLuint mUniformCameraPositionHandle;
    GLint mColorTextureLocation;
    GLint mStateTextureLocation;
    GLuint mVertexCount;
    GLuint mColorVolumeHandle;
    GLuint mStateVolumeHandle;
    GLuint mShaderProgram;
    GLuint mVertexArrayObject;

    bool mShaderInitialized;
    bool mRenderEnvironment;
    bool mRenderTemperature;
    bool mRenderVelocity;
};

#endif // RAYCASTER_H_
