#ifndef SETUP_H_
#define SETUP_H_

#include "Fan.h"
#include "Area.h"
#include "Sensor.h"
#include <vector>
#include <memory>

enum class SetupType
{
    TEST, SIMPLE_COOLER, COOLER_COMPARSION, FANS, BEER, CHANDELIER
};

static std::unique_ptr<Area> createSetup(SetupType type, std::vector<Fan>& rFans, std::vector<Sensor>& rSensors)
{
    std::unique_ptr<Area> upArea = std::unique_ptr<Area>(new Area(128, Materialtype::AIR));

    switch (type)
    {
    case SetupType::TEST:
        upArea->setBlock(Materialtype::HEATER, 22, 20, 20, 5, 1, 5);
        upArea->setBlock(Materialtype::HEATER, 52, 21, 20, 5, 1, 5);
        upArea->setBlock(Materialtype::COPPER, 20, 22, 20, 70, 3, 70);
        upArea->setBlock(Materialtype::ISOLATOR, 0, 0, 0, 128, 2, 128); // Bottom
        upArea->setBlock(Materialtype::ISOLATOR, 0, 0, 0, 128, 128, 2); // Side
        upArea->setBlock(Materialtype::ISOLATOR, 0, 126, 0, 128, 2, 128); // Top

        rFans.push_back(Fan(glm::vec3(0.3f, 0.2f, 0.1f), 0.1f, glm::vec3(1, 0.9f, 1), 0.2f));
        rFans.push_back(Fan(glm::vec3(0.5f, 0.4f, 0.3f), 0.2f, glm::vec3(-1, 0.9f, -1), 0.4f));

        rSensors.push_back(Sensor(glm::vec3(0.18f, 0.16f, 0.17f), "SensorA"));
        rSensors.push_back(Sensor(glm::vec3(0.3f, 0.5f, 0.13f), "SensorB"));
        break;
    case SetupType::SIMPLE_COOLER:
        upArea->setBlock(Materialtype::ISOLATOR, 38, 20, 18, 52, 48, 2);
        upArea->setBlock(Materialtype::COPPER, 38, 20, 20, 52, 48, 6);
        upArea->setBlock(Materialtype::HEATER, 50, 30, 20, 28, 28, 4);
        upArea->setBlock(Materialtype::COPPER, 38, 20, 26, 4, 48, 48);
        upArea->setBlock(Materialtype::COPPER, 46, 20, 26, 4, 48, 48);
        upArea->setBlock(Materialtype::COPPER, 54, 20, 26, 4, 48, 48);
        upArea->setBlock(Materialtype::COPPER, 62, 20, 26, 4, 48, 48);
        upArea->setBlock(Materialtype::COPPER, 70, 20, 26, 4, 48, 48);
        upArea->setBlock(Materialtype::COPPER, 78, 20, 26, 4, 48, 48);
        upArea->setBlock(Materialtype::COPPER, 86, 20, 26, 4, 48, 48);
        break;
    case SetupType::COOLER_COMPARSION:
        upArea->setBlock(Materialtype::ISOLATOR, 20, 20, 18, 24, 24, 20);
        upArea->setBlock(Materialtype::HEATER, 22, 22, 20, 20, 20, 4);
        upArea->setBlock(Materialtype::COPPER, 26, 26, 24, 12, 12, 14);
        upArea->setBlock(Materialtype::COPPER, 20, 20, 38, 24, 24, 2);

        upArea->setBlock(Materialtype::ISOLATOR, 84, 20, 18, 24, 24, 20);
        upArea->setBlock(Materialtype::HEATER, 86, 22, 20, 20, 20, 4);
        upArea->setBlock(Materialtype::COPPER, 90, 26, 24, 12, 12, 14);
        upArea->setBlock(Materialtype::COPPER, 84, 20, 38, 24, 24, 2);
        upArea->setBlock(Materialtype::COPPER, 84, 20, 38, 1, 24, 20);
        upArea->setBlock(Materialtype::COPPER, 86, 20, 38, 1, 24, 20);
        upArea->setBlock(Materialtype::COPPER, 88, 20, 38, 1, 24, 20);
        upArea->setBlock(Materialtype::COPPER, 90, 20, 38, 1, 24, 20);
        upArea->setBlock(Materialtype::COPPER, 92, 20, 38, 1, 24, 20);
        upArea->setBlock(Materialtype::COPPER, 94, 20, 38, 1, 24, 20);
        upArea->setBlock(Materialtype::COPPER, 96, 20, 38, 1, 24, 20);
        upArea->setBlock(Materialtype::COPPER, 98, 20, 38, 1, 24, 20);
        upArea->setBlock(Materialtype::COPPER, 100, 20, 38, 1, 24, 20);
        upArea->setBlock(Materialtype::COPPER, 102, 20, 38, 1, 24, 20);
        upArea->setBlock(Materialtype::COPPER, 104, 20, 38, 1, 24, 20);
        upArea->setBlock(Materialtype::COPPER, 106, 20, 38, 1, 24, 20);

        rSensors.push_back(Sensor(glm::vec3(0.26, 0.26, 0.3), "CoolerA"));
        rSensors.push_back(Sensor(glm::vec3(0.76, 0.26, 0.3), "CoolerB"));
        break;
    case SetupType::FANS:
        upArea->setBlock(Materialtype::HEATER, 50, 20, 50, 12, 4, 28);

        rFans.push_back(Fan(glm::vec3(0.35f, 0.3f, 0.5f), 0.2f, glm::vec3(1, 1, 0), 0.4f));
        rFans.push_back(Fan(glm::vec3(0.5f, 0.5f, 0.5f), 0.2f, glm::vec3(1, 0.4f, 0.3f), 0.4f));
        break;
    case SetupType::BEER:
        upArea->setBlock(Materialtype::GLASS, 50, 20, 50, 28, 60, 28);
        upArea->setBlock(Materialtype::BEER, 52, 24, 52, 24, 48, 24);
        upArea->setBlock(Materialtype::FOAM, 52, 72, 52, 24, 4, 24);
        upArea->setBlock(Materialtype::AIR, 52, 76, 52, 24, 4, 24);
        upArea->setBlock(Materialtype::COPPER, 40, 12, 40, 48, 8, 48);
        upArea->setBlock(Materialtype::HEATER, 54, 12, 54, 20, 4, 20);
        upArea->setBlock(Materialtype::ISOLATOR, 40, 10, 40, 48, 2, 48);

        rSensors.push_back(Sensor(glm::vec3(0.5, 0.3, 0.5), "Beer"));
        break;
    case SetupType::CHANDELIER:
        upArea->setBlock(Materialtype::ISOLATOR,60,60,60,8,8,8 );
        upArea->setBlock(Materialtype::ISOLATOR, 68,60,60,26,8,8);
        upArea->setBlock(Materialtype::ISOLATOR, 34,60,60,26,8,8);
        upArea->setBlock(Materialtype::ISOLATOR, 60,60,68,8,8,26);
        upArea->setBlock(Materialtype::ISOLATOR, 60,60,34,8,8,26);
        upArea->setBlock(Materialtype::HEATER, 62,62,62,4,4,4);
        upArea->setBlock(Materialtype::COPPER, 66,62,62,30,4,4);
        upArea->setBlock(Materialtype::IRON, 32,62,62,30,4,4);
        upArea->setBlock(Materialtype::ZINC, 62,62,66,4,4,30);
        upArea->setBlock(Materialtype::DIAMOND, 62,62,32,4,4,30);

        rSensors.push_back(Sensor(glm::vec3(0.75f, 0.5f, 0.5f), "Copper"));
        rSensors.push_back(Sensor(glm::vec3(0.25f, 0.5f, 0.5f), "Iron"));
        rSensors.push_back(Sensor(glm::vec3(0.5f, 0.5f, 0.75f), "Zinc"));
        rSensors.push_back(Sensor(glm::vec3(0.5f, 0.5f, 0.25f), "Diamond"));
        break;
    }

    return std::move(upArea);
}

#endif // SETUP_H_
