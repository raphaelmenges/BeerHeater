#include "externals/OpenGLLoader/gl_core_4_3.h"
#include "externals/GLFW/include/GLFW/glfw3.h"
#include "externals/GLM/glm/glm.hpp"
#include "externals/GLM/glm/gtc/matrix_transform.hpp"
#include "externals/picoPNG/picopng.h"
#include "Camera.h"
#include "Raycaster.h"
#include "HeatSimulator.h"
#include "FluidSimulator.h"
#include "SensorReader.h"
#include "Setup.h"
#include <sstream>
#include <iomanip>

// GLM for math
// picoPNG for loading PNGs

// ########### SETUP SETTINGS ###########
const SetupType SETUP = SetupType::COOLER_COMPARSION;
// ######################################

// Global variables
GLboolean buttonPressed = GL_FALSE;
GLfloat cursorX = 0;
GLfloat cursorY = 0;
GLfloat scrollOffsetY = 0;
std::unique_ptr<Raycaster> upRaycaster;

// GLFW callback for errors
static void errorCallback(int error, const char* description)
{
    std::cout << error << " " << description << std::endl;
}

// GLFW callback for keys
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    else if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        upRaycaster->toggleRenderEnvironment();
    }
    else if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        upRaycaster->toggleRenderTemperature();
    }
    else if (key == GLFW_KEY_V && action == GLFW_PRESS)
    {
        upRaycaster->toggleRenderVelocity();
    }
}

// GLFW callback for cursor
static void cursorCallback(GLFWwindow* pWindow, GLdouble xpos, GLdouble ypos)
{
    cursorX = (GLfloat)xpos;
    cursorY = (GLfloat)ypos;
}

// GLFW callback for mouse buttons
static void buttonsCallback(GLFWwindow* pWindow, GLint button, GLint action, GLint mods)
{
    if (button == GLFW_MOUSE_BUTTON_1)
    {
        if (action == GLFW_PRESS)
        {
            buttonPressed = GL_TRUE;
        }
        else if (action == GLFW_RELEASE)
        {
            buttonPressed = GL_FALSE;
        }
    }
}

// GLFW callback for scrolling
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    scrollOffsetY = (float) yoffset;
}

// Main
int main(void)
{
    // Tutorial
    std::cout << "Welcome to Air Simulation by Nils Hoehner and Raphael Menges!" << std::endl;
    std::cout << "Following keys can be used for controlling:" << std::endl;
    std::cout << "E: Show / hide environment" << std::endl;
    std::cout << "T: Show / hide temperature" << std::endl;
    std::cout << "V: Show / hide velocity" << std::endl;

    // Initialize GLFW and OpenGL
    GLFWwindow* pWindow;
    glfwSetErrorCallback(errorCallback);
    if (!glfwInit())
        exit(EXIT_FAILURE);

    pWindow = glfwCreateWindow(1024, 768, "BeerHeater", NULL, NULL);
    if (!pWindow)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(pWindow);
    ogl_LoadFunctions();

    // GLFW callbacks
    glfwSetKeyCallback(pWindow, keyCallback);
    glfwSetCursorPosCallback(pWindow, cursorCallback);
    glfwSetMouseButtonCallback(pWindow, buttonsCallback);
    glfwSetScrollCallback(pWindow, scrollCallback);

    // OpenGL initialization
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1);
    glEnable(GL_TEXTURE_1D);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_3D);
    glEnable(GL_CULL_FACE);

    // Initialize camera
    Camera camera(glm::vec3(0.5f), glm::radians(-135.0f), glm::radians(80.0f), 2, 0.1f, 5);

    // Variables for camera control
    GLfloat prevCursorX = cursorX;
    GLfloat prevCursorY = cursorY;

    // Fans
    std::vector<Fan> fans;

    // Sensors
    std::vector<Sensor> sensors;

    // Area
    std::unique_ptr<Area> upArea = std::move(createSetup(SETUP, fans, sensors));

    // Raycaster
    upRaycaster = std::unique_ptr<Raycaster>(new Raycaster(upArea->getColorVolumeHandle(), upArea->getStateVolumeHandle()));

    // Fluid simulator
    FluidSimulator fluidSimulator(*(upArea.get()), fans);
    fluidSimulator.setMEdgeLenght(0.1f);

    // Heat simulator
    HeatSimulator heatSimulator(*(upArea.get()));
    heatSimulator.setMEdgeLenght(0.1f);

    // Sensor reader
    SensorReader sensorReader(upArea->getStateVolumeHandle(), sensors);

    // Variables for the loop
    GLfloat prevTime = (GLfloat)glfwGetTime();
    GLfloat deltaTime;
    GLint prevWidth = 0;
    GLint prevHeight = 0;

    glm::mat4 uniformView;
    glm::mat4 uniformProjection;

    // Loop
    while (!glfwWindowShouldClose(pWindow))
    {
        // Clear buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get window resolution
        GLint width, height;
        glfwGetWindowSize(pWindow, &width, &height);

        // Give OpenGL the window resolution
        if (width != prevWidth || height != prevHeight)
        {
            glViewport(0, 0, width, height);
            prevWidth = width;
            prevHeight = height;
        }

        // Calc time per frame
        GLfloat currentTime = (GLfloat)glfwGetTime();
        deltaTime = currentTime - prevTime;
        prevTime = currentTime;

        // Calculate cursor movement
        GLfloat cursorDeltaX = cursorX - prevCursorX;
        GLfloat cursorDeltaY = cursorY - prevCursorY;
        prevCursorX = cursorX;
        prevCursorY = cursorY;

        // View matrix
        if (buttonPressed)
        {
            camera.setAlpha(camera.getAlpha() + 0.005f * cursorDeltaX);
            camera.setBeta(camera.getBeta() - 0.005f * cursorDeltaY);
        }
        camera.setRadius(camera.getRadius() + 0.1f * scrollOffsetY);
        scrollOffsetY = 0;
        uniformView = camera.getViewMatrix();

        // Projection matrix
        uniformProjection = glm::perspective(glm::radians(35.0f), ((GLfloat)width / (GLfloat)height), 0.1f, 100.f);

        // Simulate (TODO: real time steps. at the moment depending on frame time)
        fluidSimulator.nextStep(0.5);
        heatSimulator.nextStep(0.5);

        // Draw raycaster
        upRaycaster->draw(uniformView, uniformProjection, camera.getPosition());

        // Draw fans (TODO: Does not work)
        for(const Fan& fan : fans)
        {
            fan.draw(uniformView, uniformProjection);
        }

        // Update and draw sensors
        std::vector<std::string> sensorOutputs = sensorReader.updateAndDraw(uniformView, uniformProjection);

        // Prepare next frame
        glfwSwapBuffers(pWindow);
        glfwPollEvents();

        // Printing
		std::stringstream fps;
		fps << std::setfill('0') << std::setw(3) << (int)(1.0f / deltaTime);
		std::string sensorPrint = "";
		if (!sensorOutputs.empty())
		{
			sensorPrint = " | Sensors: ";
			for (int i = 0; i < sensorOutputs.size(); i++)
			{
				sensorPrint += sensorOutputs[i];
				if (i < sensorOutputs.size() - 1)
				{
					sensorPrint += ", ";
				}
			}
		}
		std::cout << "\r" << "FPS: " << fps.str() << sensorPrint;
    }

    // Termination
    glfwDestroyWindow(pWindow);
    glfwTerminate();

	return 0;
}
