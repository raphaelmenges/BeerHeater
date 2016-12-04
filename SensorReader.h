#ifndef SENSOR_READER_H_
#define SENSOR_READER_H_

#include "Sensor.h"
#include "externals/OpenGLLoader/gl_core_4_3.h"
#include <vector>
#include <string>

class SensorReader
{
public:
	SensorReader(GLuint stateVolumeHandle, std::vector<Sensor> sensors);
	~SensorReader();
	std::vector<std::string> updateAndDraw(const glm::mat4& uniformView, const glm::mat4& uniformProjection) const;

private:
	std::vector<Sensor> mSensors;
	GLuint mSensorReaderProgram;
	GLuint mStateVolume;
	GLuint mSensorsSSBO;
	int mStateVolumeLocation;
	int mSensorCountLocation;
	GLuint mUniformModelHandle;
	GLuint mUniformViewHandle;
	GLuint mUniformProjectionHandle;
	GLuint mShaderProgram;
	GLuint mVertexArrayObject;
	GLuint mVertexBuffer;
	GLuint mVertexCount;
	glm::mat4 mUniformModel;
};

#endif // SENSOR_READER_H_