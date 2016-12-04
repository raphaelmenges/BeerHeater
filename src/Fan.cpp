#include "Fan.h"

#include "externals/GLM/glm/gtc/matrix_transform.hpp"
#include "externals/GLM/glm/gtc/type_ptr.hpp"
#include <iostream>
#include <vector>

const char* fanVertexShaderSource =
    "#version 430 core\n"
    "in vec4 positionAttribute;\n"
	"out float falloff;\n"
    "uniform mat4 uniformModel;\n"
    "uniform mat4 uniformView;\n"
    "uniform mat4 uniformProjection;\n"
    "void main()\n"
    "{\n"
    "	gl_Position = uniformProjection * uniformView * uniformModel * positionAttribute;\n" // Output for rasterization
	"	falloff = positionAttribute.x + 0.5;\n"
    "}";

const char* fanFragmentShaderSource =
    "#version 430 core\n"
    "out vec4 fragmentColor;\n"
	"in float falloff;\n"
    "void main()\n"
    "{\n"
    "	fragmentColor = vec4(0,1*falloff,1,1);\n" // Output
    "}";

Fan::Fan(glm::vec3 position, float speed, glm::vec3 direction, float distance)
{
    mPosition = position;
	mSpeed = speed;
    mDirection = glm::normalize(direction);
    mDistance = distance;

    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &fanVertexShaderSource, NULL);
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
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fanFragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Get length of compiling log
    log_length = 0;
    glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &log_length);

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

    // Program
    mShaderProgram = glCreateProgram();

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

	// Create vertices
	std::vector<float> vertices;
	vertices.push_back(mPosition.x);
	vertices.push_back(mPosition.y);
	vertices.push_back(mPosition.z);
	vertices.push_back(mPosition.x + mDistance * mDirection.x);
	vertices.push_back(mPosition.y + mDistance * mDirection.y);
	vertices.push_back(mPosition.z + mDistance * mDirection.z);
	
	glm::vec3 base = 0.5f * glm::cross(mDirection, glm::vec3(0, 1, 0));
	vertices.push_back(mPosition.x - mSpeed * base.x);
	vertices.push_back(mPosition.y - mSpeed * base.y);
	vertices.push_back(mPosition.z - mSpeed * base.z);
	vertices.push_back(mPosition.x + mSpeed * base.x);
	vertices.push_back(mPosition.y + mSpeed * base.y);
	vertices.push_back(mPosition.z + mSpeed * base.z);

    // Vertex buffer
	mVertexBuffer = 0;
    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    mVertexCount = (GLuint)vertices.size() / 3;
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Vertex array object
    mVertexArrayObject = 0;
    glGenVertexArrays(1, &mVertexArrayObject);
    glBindVertexArray(mVertexArrayObject);

    // Vertices
    GLuint vertexAttrib = glGetAttribLocation(mShaderProgram, "positionAttribute");
    glEnableVertexAttribArray(vertexAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glVertexAttribPointer(vertexAttrib, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Unbind vertex array object
    glBindVertexArray(0);

    // Create model matrix
    mUniformModel = glm::mat4(1.0f);
}

Fan::~Fan()
{
	glDeleteProgram(mShaderProgram);
	glDeleteVertexArrays(1, &mVertexArrayObject);
	glDeleteBuffers(1, &mVertexBuffer);
}

glm::vec3 Fan::getPosition() const
{
    return mPosition;
}

float Fan::getSpeed() const
{
	return mSpeed;
}

glm::vec3 Fan::getDirection() const
{
    return mDirection;
}

float Fan::getDistance() const
{
    return mDistance;
}

void Fan::draw(const glm::mat4& uniformView, const glm::mat4& uniformProjection) const
{
    // Bind shader
    glUseProgram(mShaderProgram);

    // Bind vertex array object
    glBindVertexArray(mVertexArrayObject);

    // Set updated uniforms in shader
    glUniformMatrix4fv(mUniformModelHandle, 1, GL_FALSE, glm::value_ptr(mUniformModel));
    glUniformMatrix4fv(mUniformViewHandle, 1, GL_FALSE, glm::value_ptr(uniformView));
    glUniformMatrix4fv(mUniformProjectionHandle, 1, GL_FALSE, glm::value_ptr(uniformProjection));

    // Drawing
    glDrawArrays(GL_LINES, 0, mVertexCount);
}
