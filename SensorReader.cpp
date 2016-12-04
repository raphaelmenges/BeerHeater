#include "SensorReader.h"

#include "externals/GLM/glm/gtc/type_ptr.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

const int MAX_SENSOR_COUNT = 16;
const float RENDER_HALF_SCALE = 0.05f;

const char* sensorReaderShaderSource =
"#version 430 core\n"

// Structs
"struct SensorStruct{\n"
"	vec3 position;\n"
"	float temperature;\n"
"};\n"

// Layouts
"layout(local_size_x=4, local_size_y=1, local_size_z=1) in;\n"
"layout(rgba32f, location = 0) uniform image3D stateVolume;\n"
"layout(std430, binding=0) buffer Sensor\n"
"{\n"
"       SensorStruct sensors[];\n"
"};\n"

// Uniforms
"uniform int sensorCount;\n"

// Main
"void main()\n"
"{\n"
"	unsigned int index = gl_GlobalInvocationID.x;\n"
"	if(index < sensorCount)\n"
"	{\n"
"		ivec3 coords = ivec3(sensors[index].position * 128);\n" // TODO: Voxel count guessed
"		sensors[index].temperature = imageLoad(stateVolume, coords).x;\n"
"	}\n"
"}\n";

const char* sensorVertexShaderSource =
"#version 430 core\n"
"in vec4 positionAttribute;\n"
"uniform mat4 uniformModel;\n"
"uniform mat4 uniformView;\n"
"uniform mat4 uniformProjection;\n"
"void main()\n"
"{\n"
"	gl_Position = uniformProjection * uniformView * uniformModel * positionAttribute;\n" // Output for rasterization
"}";

const char* sensorFragmentShaderSource =
"#version 430 core\n"
"out vec4 fragmentColor;\n"
"void main()\n"
"{\n"
"	fragmentColor = vec4(1,1,1,1);\n" // Output
"}";

SensorReader::SensorReader(GLuint stateVolumeHandle, std::vector<Sensor> sensors)
{
	mSensors = sensors;
	mStateVolume = stateVolumeHandle;

	if (mSensors.size() > 0)
	{
		std::vector<Sensor::SensorStruct> tmp;

		for(int i=0; i<sensors.size();i++)
		{
			tmp.push_back(sensors.at(i).getSensor());
		}

		// Copy sensors to the shader storage buffer object
		glGenBuffers(1, &mSensorsSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSensorsSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Sensor::SensorStruct) * mSensors.size(), tmp.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		// Create compute shader
		mSensorReaderProgram = glCreateProgram();
		GLint sensorReaderCS = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(sensorReaderCS, 1, &sensorReaderShaderSource, NULL);
		glCompileShader(sensorReaderCS);

		// Get length of compiling log
		GLint log_length = 0;
		glGetShaderiv(sensorReaderCS, GL_INFO_LOG_LENGTH, &log_length);

		if (log_length > 1)
		{
			// Copy log to chars
			GLchar *log = new GLchar[log_length];
			glGetShaderInfoLog(sensorReaderCS, log_length, NULL, log);

			// Print it
			std::cout << log << std::endl;

			// Delete chars
			delete[] log;
		}

		glAttachShader(mSensorReaderProgram, sensorReaderCS);
		glLinkProgram(mSensorReaderProgram);
		glDetachShader(mSensorReaderProgram, sensorReaderCS);
		glDeleteShader(sensorReaderCS);

		// Get locations in shader
		mStateVolumeLocation = glGetUniformLocation(mSensorReaderProgram, "stateVolume");
		mSensorCountLocation = glGetUniformLocation(mSensorReaderProgram, "sensorCount");

		// Vertex shader
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &sensorVertexShaderSource, NULL);
		glCompileShader(vertexShader);

		// Get length of compiling log
		log_length = 0;
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
		glShaderSource(fragmentShader, 1, &sensorFragmentShaderSource, NULL);
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

		for (Sensor sensor : mSensors)
		{
			vertices.push_back(sensor.getSensor().position.x - RENDER_HALF_SCALE);
			vertices.push_back(sensor.getSensor().position.y);
			vertices.push_back(sensor.getSensor().position.z);
			vertices.push_back(sensor.getSensor().position.x + RENDER_HALF_SCALE);
			vertices.push_back(sensor.getSensor().position.y);
			vertices.push_back(sensor.getSensor().position.z);

			vertices.push_back(sensor.getSensor().position.x);
			vertices.push_back(sensor.getSensor().position.y - RENDER_HALF_SCALE);
			vertices.push_back(sensor.getSensor().position.z);
			vertices.push_back(sensor.getSensor().position.x);
			vertices.push_back(sensor.getSensor().position.y + RENDER_HALF_SCALE);
			vertices.push_back(sensor.getSensor().position.z);

			vertices.push_back(sensor.getSensor().position.x);
			vertices.push_back(sensor.getSensor().position.y);
			vertices.push_back(sensor.getSensor().position.z - RENDER_HALF_SCALE);
			vertices.push_back(sensor.getSensor().position.x);
			vertices.push_back(sensor.getSensor().position.y);
			vertices.push_back(sensor.getSensor().position.z + RENDER_HALF_SCALE);
		}

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
}

SensorReader::~SensorReader()
{
	glDeleteBuffers(1, &mSensorsSSBO);
	glDeleteProgram(mShaderProgram);
	glDeleteVertexArrays(1, &mVertexArrayObject);
	glDeleteBuffers(1, &mVertexBuffer);
}

std::vector<std::string> SensorReader::updateAndDraw(const glm::mat4& uniformView, const glm::mat4& uniformProjection) const
{
	std::vector<std::string> values;
	if (mSensors.size() > 0)
	{
		// Bind rendering shader
		glUseProgram(mShaderProgram);

		// Bind vertex array object
		glBindVertexArray(mVertexArrayObject);

		// Set updated uniforms in shader
		glUniformMatrix4fv(mUniformModelHandle, 1, GL_FALSE, glm::value_ptr(mUniformModel));
		glUniformMatrix4fv(mUniformViewHandle, 1, GL_FALSE, glm::value_ptr(uniformView));
		glUniformMatrix4fv(mUniformProjectionHandle, 1, GL_FALSE, glm::value_ptr(uniformProjection));

		// Drawing
		glDrawArrays(GL_LINES, 0, mVertexCount);

		// Use reader program
		glUseProgram(mSensorReaderProgram);

		// Bind ssbo
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSensorsSSBO);

		// Bind image
		glBindImageTexture(0,
			mStateVolume,
			0,
			GL_TRUE,
			0,
			GL_READ_ONLY,
			GL_RGBA32F);

		// Fill uniforms
		glUniform1i(mStateVolumeLocation, 0);
		glUniform1i(mSensorCountLocation, (GLint)mSensors.size());

		// Dispatch
		glDispatchCompute(MAX_SENSOR_COUNT/4, 1, 1);

		// Barrier
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		// Collects informations
	
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSensorsSSBO);
		Sensor::SensorStruct* ptr = (Sensor::SensorStruct*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		for (int i = 0; i < mSensors.size(); i++)
		{
			std::stringstream temperature; 
			temperature << std::setfill('0') << std::fixed << std::setw(6) << std::setprecision(3) << ptr[i].temperature;
			values.push_back(mSensors.at(i).getName() + " = " + temperature.str());
		}
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	// Return strings for output
	return values;
}