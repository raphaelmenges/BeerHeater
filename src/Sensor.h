#ifndef SENSOR_H_
#define SENSOR_H_

#include "externals/GLM/glm/glm.hpp"
#include <string>
#include <iostream>

class Sensor
{
public:
	Sensor(glm::vec3 position, std::string name): mStruct(position), mName(name){}

	struct SensorStruct {
		glm::vec3 position;
		float temperature = 0;

		SensorStruct(glm::vec3
		position)
		{
			this->position = position;
		}
	};
	std::string getName() const {return mName;}
	SensorStruct getSensor(){return mStruct;}
private:
	std::string mName;
	SensorStruct mStruct;
};

#endif // SENSOR_H_
