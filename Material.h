#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "externals/GLM/glm/glm.hpp"

enum class Materialtype
{
    AIR, IRON, HEATER, COPPER, ISOLATOR, BEER, GLASS, FOAM, DIAMOND, ZINC
};

struct Material
{
    glm::vec4 color;
    glm::vec4 cisf; // conductivity, internalHeatGeneration, specificHeat, fluidity
    glm::vec4 dppp; // density
};

namespace material
{

    const Material iron =
    {
        glm::vec4(120.0f / 255, 50.0f / 255, 46.0f / 255, 1), // color
        glm::vec4(
            80.f, // conductivity
            0.0f, // internalHeatGeneration
            450.f, // specificHeat
            0.f), // fluidity
        glm::vec4(7870.f,0,0,0) // density
    };

    const Material copper =
    {
        glm::vec4(150.0f/255, 100.0f/255, 56.0f/255, 1), // color
        glm::vec4(
            385.f, // conductivity
            0.0f, // internalHeatGeneration
            390.f, // specificHeat
            0.f), // fluidity
        glm::vec4(8960.f,0,0,0) // density
    };

    const Material air =
    {
        glm::vec4(171.0f/255, 227.0f/255, 214.0f/255, 8.0f/255), // color
        glm::vec4(
            0.024f, // conductivity
            0.f, // internalHeatGeneration
            1012.f, // specificHeat
            1.205f), // fluidity
        glm::vec4(
            1.1839f, 0, 0, 0) // density
    };

    const Material heater =
    {
        glm::vec4(255.0f/255, 200.0f/255, 100.0f/255, 1), // color
        glm::vec4(
            0.2f, // conductivity
            100.f, // internalHeatGeneration
            750.f, // specificHeat
            0.f), // fluidity
        glm::vec4(
            2800.f, 0, 0, 0) // density
    };

    const Material isolator =
    {
        glm::vec4(64.0f/255, 255.0f/255, 120.0f/255, 30.0f/255), // color
        glm::vec4(
            0.f, // conductivity
            0.0f, // internalHeatGeneration
            450.f, // specificHeat
            0.f), // fluidity
        glm::vec4(2000.f,0,0,0) // density
    };

    const Material beer =
    {
        glm::vec4(255.0f/255, 200.0f/255, 20.0f/255, 200.0f/255), // color
        glm::vec4(
            0.635f, // conductivity
            0.0f, // internalHeatGeneration
            4157.f, // specificHeat
            0.5f), // fluidity (TODO: no value found)
        glm::vec4(1008.f,0,0,0) // density
    };

    const Material glass =
    {
        glm::vec4(200.0f/255, 230.0f/255, 255.0f/255, 40.0f/255), // color
        glm::vec4(
            0.8f, // conductivity
            0.0f, // internalHeatGeneration
            800.f, // specificHeat
            0.f), // fluidity
        glm::vec4(2500.f,0,0,0) // density
    };

    const Material foam =
    {
        glm::vec4(255.0f/255, 255.0f/255, 255.0f/255, 150.0f/255), // color
        glm::vec4(
            0.045f, // conductivity (TODO: no value found)
            0.0f, // internalHeatGeneration
            4157.f, // specificHeat (TODO: no value found)
            0.f), // fluidity
        glm::vec4(500.f,0,0,0) // density (TODO: no value found)
    };

    const Material diamond =
    {
            glm::vec4(1,1,1,0.7), // color
            glm::vec4(
                    1000.f, // conductivity
                    0.0f, // internalHeatGeneration
                    630.f, // specificHeat
                    0.f), // fluidity
            glm::vec4(3500.f,0,0,0) // density
    };

    const Material zinc =
    {
            glm::vec4(80.0f / 255, 70.0f / 255, 66.0f / 255, 1), // color
            glm::vec4(
                    113.f, // conductivity
                    0.0f, // internalHeatGeneration
                    388.f, // specificHeat
                    0.f), // fluidity
            glm::vec4(7140.f,0,0,0) // density
    };
}

#endif // MATERIAL_H_
