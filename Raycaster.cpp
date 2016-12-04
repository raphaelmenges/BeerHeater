#include "Raycaster.h"
#include "RaycastingShader.h"

#include "externals/GLM/glm/gtc/type_ptr.hpp"

Raycaster::Raycaster(GLuint colorVolumeHandle, GLuint stateVolumeHandle)
{
    // Save member
    mColorVolumeHandle = colorVolumeHandle;
    mStateVolumeHandle = stateVolumeHandle;

    // Initialize members
    mShaderInitialized = false;
    mRenderEnvironment = true;
    mRenderTemperature = true;
    mRenderVelocity = false;

    // First compilation of shader
    recompileShader();

    // Vertex buffer
    GLuint vertexBuffer = 0;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    mVertexCount = (sizeof(cubeVertices) / sizeof(GLfloat)) / 3;
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Vertex array object
    mVertexArrayObject = 0;
    glGenVertexArrays(1, &mVertexArrayObject);
    glBindVertexArray(mVertexArrayObject);

    // Vertices
    GLuint vertexAttrib = glGetAttribLocation(mShaderProgram, "positionAttribute");
    glEnableVertexAttribArray(vertexAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(vertexAttrib, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Unbind vertex array object
    glBindVertexArray(0);
}

Raycaster::~Raycaster()
{
    // Delete shader program
    glDeleteProgram(mShaderProgram);
}

void Raycaster::draw(const glm::mat4& uniformView, const glm::mat4& uniformProjection, const glm::vec3& cameraPosition) const
{
    // Bind shader
    glUseProgram(mShaderProgram);

    // Bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, mColorVolumeHandle);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, mStateVolumeHandle);

    if(mColorTextureLocation >= 0)
    {
        glUniform1i(mColorTextureLocation, 0);
    }
    if(mStateTextureLocation >= 0)
    {
        glUniform1i(mStateTextureLocation, 1);
    }

    // Bind vertex array object
    glBindVertexArray(mVertexArrayObject);

    // Set updated uniforms in shader
    glUniformMatrix4fv(mUniformModelHandle, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
    glUniformMatrix4fv(mUniformViewHandle, 1, GL_FALSE, glm::value_ptr(uniformView));
    glUniformMatrix4fv(mUniformProjectionHandle, 1, GL_FALSE, glm::value_ptr(uniformProjection));
    glUniform3fv(mUniformCameraPositionHandle, 1, glm::value_ptr(cameraPosition));

    // Drawing
    glDrawArrays(GL_QUADS, 0, mVertexCount);
}

void Raycaster::toggleRenderEnvironment()
{
    mRenderEnvironment = !mRenderEnvironment;
    recompileShader();
}

void Raycaster::toggleRenderTemperature()
{
    mRenderTemperature = !mRenderTemperature;
    recompileShader();
}

void Raycaster::toggleRenderVelocity()
{
    mRenderVelocity = !mRenderVelocity;
    recompileShader();
}

void Raycaster::recompileShader()
{
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &pRaycastingVertexShader, NULL);
    glCompileShader(vertexShader);

    // Get length of compiling log
    GLint log_length = 0;
    glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &log_length);

    if (log_length > 1)
    {
        // Copy log to chars
        GLchar *log = new GLchar[log_length];
        glGetShaderInfoLog(vertexShader, log_length, NULL, log);

        // Print it
        std::cout << log << std::endl;

        // Delete chars
        delete[] log;
    }

    // Fragment shader
    std::string fragmentShaderSource = std::string(pRaycastingFragmentShader);
    fragmentShaderSource = std::string("\n") + fragmentShaderSource;
    if (mRenderEnvironment)
    {
        fragmentShaderSource = std::string("#define RENDER_ENVIRONMENT\n") + fragmentShaderSource;
    }
    if (mRenderTemperature)
    {
        fragmentShaderSource = std::string("#define RENDER_TEMPERATURE\n") + fragmentShaderSource;
    }
    if (mRenderVelocity)
    {
        fragmentShaderSource = std::string("#define RENDER_VELOCITY\n") + fragmentShaderSource;
    }

    // Version must be first line
    fragmentShaderSource = std::string("#version 430 core\n\n") + fragmentShaderSource;

    // Compile fragment shader
    GLchar *cstr = new GLchar[fragmentShaderSource.length() + 1];
    strcpy(cstr, fragmentShaderSource.c_str());
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &cstr, NULL);
    glCompileShader(fragmentShader);
    delete[] cstr;

    // Get length of compiling log
    log_length = 0;
    glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &log_length);

    if (log_length > 1)
    {
        // Copy log to chars
        GLchar *log = new GLchar[log_length];
        glGetShaderInfoLog(fragmentShader, log_length, NULL, log);

        // Print it
        std::cout << log << std::endl;

        // Delete chars
        delete[] log;
    }

    // Create program
    if(mShaderInitialized)
    {
        glDeleteProgram(mShaderProgram);
    }
    mShaderProgram = glCreateProgram();
    mShaderInitialized = true;

    // Link program
    glAttachShader(mShaderProgram, vertexShader);
    glAttachShader(mShaderProgram, fragmentShader);
    glLinkProgram(mShaderProgram);

    // Delete shader
    glDetachShader(mShaderProgram, fragmentShader);
    glDetachShader(mShaderProgram, vertexShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Fetch uniforms from shader program
    mUniformModelHandle = glGetUniformLocation(mShaderProgram, "uniformModel");
    mUniformViewHandle = glGetUniformLocation(mShaderProgram, "uniformView");
    mUniformProjectionHandle = glGetUniformLocation(mShaderProgram, "uniformProjection");
    mUniformCameraPositionHandle = glGetUniformLocation(mShaderProgram, "uniformCameraPosition");
    mColorTextureLocation = glGetUniformLocation(mShaderProgram, "uniformColorVolume");
    mStateTextureLocation = glGetUniformLocation(mShaderProgram, "uniformStateVolume");
}
