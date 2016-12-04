#ifndef CAMERA_H_
#define CAMERA_H_

#include "externals/OpenGLLoader/gl_core_4_3.h"
#include "externals/GLFW/include/GLFW/glfw3.h"
#include "externals/GLM/glm/glm.hpp"
#include "externals/GLM/glm/gtc/matrix_transform.hpp"
#include "externals/GLM/glm/gtc/type_ptr.hpp"

const GLfloat CAMERA_BETA_BIAS = 0.0001f;

class Camera
{
public:
    Camera(glm::vec3 center, GLfloat alpha, GLfloat beta, GLfloat radius, GLfloat minRadius, GLfloat maxRadius);
    ~Camera();
    void reset(glm::vec3 center, GLfloat alpha, GLfloat beta, GLfloat radius);
    void setCenter(glm::vec3 center);
    void setAlpha(GLfloat alpha);
    void setBeta(GLfloat beta);
    void setRadius(GLfloat radius);

    glm::mat4 getViewMatrix();
    glm::vec3 getPosition();
    glm::vec3 getCenter() const;
    GLfloat getAlpha() const;
    GLfloat getBeta() const;
    GLfloat getRadius() const;

protected:
    void calcPosition();
    void clampValues();
    void clampAlpha();
    void clampBeta();
    void clampRadius();

    GLfloat alpha;
    GLfloat beta;
    GLfloat radius;
    GLfloat minRadius;
    GLfloat maxRadius;
    glm::vec3 center;
    glm::vec3 position;
};

#endif
