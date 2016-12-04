#ifndef FAN_H_
#define FAN_H_

#include "externals/OpenGLLoader/gl_core_4_3.h"
#include "externals/GLM/glm/glm.hpp"

const float FAN_SIZE = 0.05f;

class Fan
{
public:
    Fan(
        glm::vec3 position,
		float speed,
        glm::vec3 direction,
        float distance);
    ~Fan();
    void draw(const glm::mat4& uniformView, const glm::mat4& uniformProjection) const;
    glm::vec3 getPosition() const;
	float getSpeed() const;
    glm::vec3 getDirection() const;
    float getDistance() const;

private:
    glm::vec3 mPosition;
	float mSpeed;
    glm::vec3 mDirection;
    float mDistance;
    GLuint mUniformModelHandle;
    GLuint mUniformViewHandle;
    GLuint mUniformProjectionHandle;
    GLuint mShaderProgram;
    GLuint mVertexArrayObject;
    GLuint mVertexBuffer;
    GLuint mVertexCount;
    glm::mat4 mUniformModel;
};

#endif // FAN_H_
