#ifndef CALCULATEAVERAGETEMPERATURESHADER_H_
#define CALCULATEAVERAGETEMPERATURESHADER_H_

const char* fluidSimComputeShader =
"#version 430 core\n"

// Workgroup settings
"layout(local_size_x=8, local_size_y=8, local_size_z=8) in;\n"

// Uniforms
"layout(rgba32f, location = 0) uniform image3D stateVolume;\n"

// Global variables
"vec4 myState;\n"

// Shared Memory
"shared float temperatureCache[8*8*8*]\n"

// Get state
"vec4 getState(ivec3 coords)"
"{\n"
"	return imageLoad(stateVolume, coords);\n"
"}\n"

// Main
"void main()\n"
"{\n"
    "int numberOfThreads = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;\n"
    "temperaturCache[gl_LocalInvocationIndex] = getState(ivec3(gl_GlobalInvocationID)).x;\n" // load temperature into shared memory
    "barrier();\n" // make sure all threads finished loading the temperature cache
    "\n"
"}\n";

#endif CALCULATEAVERAGETEMPERATURESHADER_H_
