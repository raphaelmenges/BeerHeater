#ifndef RAYCASTING_SHADER_H_
#define RAYCASTING_SHADER_H_

const char* pRaycastingVertexShader =
    "#version 430 core\n"
    "in vec4 positionAttribute;\n"
    "out vec3 position;\n"
    "out vec3 direction;\n"
    "uniform mat4 uniformModel;\n"
    "uniform mat4 uniformView;\n"
    "uniform mat4 uniformProjection;\n"
    "uniform vec3 uniformCameraPosition;\n"
    "void main()\n"
    "{\n"
    "	position = positionAttribute.xyz;\n" // Startposition for rays
    "	direction = position - uniformCameraPosition;\n" // Direction in model space
    "	gl_Position = uniformProjection * uniformView * uniformModel * positionAttribute;\n" // Output for rasterization
    "}";

const char* pRaycastingFragmentShader =
    "in vec3 position;\n"
    "in vec3 direction;\n"
    "out vec4 fragmentColor;\n"
    "uniform sampler3D uniformColorVolume;\n"
    "uniform sampler3D uniformStateVolume;\n"
    "const float stepSize = 0.004;\n"
    "const float outerIterations = 100;\n"
    "const vec3 sun = vec3(0.5,-1,0.75);\n"
    "const vec4 blue = vec4(0,0,1,0.1);\n" // Blue part of celsius to color mapping
    "const vec4 green = vec4(1,1,0,0.4);\n" // Blue part of celsius to color mapping
    "const vec4 red = vec4(1,0,0,0.6);\n" // Blue part of celsius to color mapping
    "const vec4 white = vec4(1,1,1,0.75);\n" // Blue part of celsius to color mapping
    "vec3 dir;\n" // Basic raycasting variables (all vectors are in model space)
    "vec3 pos = position;\n"
    "vec4 src;\n"
    "vec3 normalizedSun;\n"
    "vec4 dst;\n"
    "float maxCelsius;\n"
    "\n"
    "vec4 calcNormal(vec3 pos, float offset)\n"
    "{\n"
    "   vec3 nrm = vec3(0,0,0);\n"
    "   float x1 = texture(uniformColorVolume, vec3(pos.x + offset, pos.y, pos.z)).a;\n"
    "   float x2 = texture(uniformColorVolume, vec3(pos.x - offset, pos.y, pos.z)).a;\n"
    "   float y1 = texture(uniformColorVolume, vec3(pos.x, pos.y + offset, pos.z)).a;\n"
    "   float y2 = texture(uniformColorVolume, vec3(pos.x, pos.y - offset, pos.z)).a;\n"
    "   float z1 = texture(uniformColorVolume, vec3(pos.x, pos.y, pos.z + offset)).a;\n"
    "   float z2 = texture(uniformColorVolume, vec3(pos.x, pos.y, pos.z - offset)).a;\n"
    "   nrm.x = x1 - x2;\n"
    "   nrm.y = y1 - y2;\n"
    "   nrm.z = z1 - z2;\n"
    "   float mag = length(nrm) / 1.7320508;\n"
    "   // nrm = normalize(nrm);\n"
    "   return vec4(nrm, mag);\n"
    "}\n"
    "\n"
    "vec4 celsiusToColor(float temperature)\n"
    "{\n"
    "   temperature /= 100;\n"
    "   float b = 1- clamp(temperature * 3, 0, 1);\n"
    "   float g = clamp(temperature * 3, 0, 1) * (1 - clamp((temperature * 3) - 1, 0, 1));\n"
    "   float r = clamp((temperature * 3) - 1, 0, 1) * (1 - clamp((temperature * 3) - 2, 0, 1));\n"
    "   float w = clamp((temperature * 3) - 2, 0, 1);\n"
    "   return b*blue + g*green + r*red + w*white;\n"
    "}\n"
    "\n"
    "void nextStep()\n"
    "{\n"
    "	src.rgba = vec4(0,0,0,0);\n"
    "	vec4 state = texture(uniformStateVolume, pos).rgba;\n"
    //	Render environment
    "	#ifdef RENDER_ENVIRONMENT\n"
    "		src.rgba = texture(uniformColorVolume, pos).rgba;\n" // Get color value from volume
    "		src.a = pow((src.a * 2), 2) / 4; \n" // Adjust alpha a little bit
    "		vec3 nrm = calcNormal(pos, 0.005).xyz;\n"
    "		src.rgb *= 0.7 + 0.3 * max(dot(normalizedSun, nrm),0);\n"
    "	#endif\n"
    //	Render temperature
    "	#ifdef RENDER_TEMPERATURE\n"
    "       float temperature = state.r;\n" // Get temperature value from volume
    "       maxCelsius = max(maxCelsius, temperature);\n"
    "	#endif\n"
    //	Render velocity
    "	#ifdef RENDER_VELOCITY\n"
    "		const float maxVelocity = 0.05f;\n"
    "		vec3 velocity = clamp(state.gba, vec3(-maxVelocity, -maxVelocity, -maxVelocity), vec3(maxVelocity, maxVelocity, maxVelocity));\n"
    "		float velocityAlpha = (state.r / 100) * (abs(velocity.r) + abs(velocity.g) + abs(velocity.b));\n"
    "		velocity += maxVelocity;\n"
    "		velocity *= 1.0 / (2*maxVelocity);\n"
    "       src.rgba += vec4(velocity, velocityAlpha);\n"
    "	#endif\n"
    //	Composition
    "	dst.rgb += (1.0 - dst.a) * src.rgb * src.a;\n" // Front-To-Back composition
    "	dst.a += (1.0 - dst.a) * src.a;\n"
    "	pos += dir*stepSize;\n" // Prepare for next sample
    "}\n"
    "\n"
    "void main()\n"
    "{\n"
    "   dir = normalize(direction);\n"
    "   normalizedSun = normalize(sun);\n"
    "   dst = vec4(0, 0, 0, 0);\n"
    "   maxCelsius = -1000;\n"
    "	for (int i = 0; i < outerIterations; i++)\n" // Sample along the ray
    "	{\n"
    "       nextStep();\n"
    "       nextStep();\n"
    "       nextStep();\n"
    "       nextStep();\n"
    "       nextStep();\n"
    "		if (pos.x > 1 || pos.y > 1 || pos.z > 1 ||\n"
    "			pos.x < 0 || pos.y < 0 || pos.z < 0 || dst.a >= 0.99)\n" // Check whether still in volume
    "		{\n"
    "			break;\n"
    "		}\n"
    "	}\n"
    "   vec4 temperature = celsiusToColor(maxCelsius);\n"
    "	fragmentColor = vec4(dst.rgb + temperature.rgb * temperature.a, 1);\n" // Output
    "}";

const char* pMinimalFragmentShader =
    "#version 430 core\n"
    "out vec4 fragmentColor;\n"
    "void main()\n"
    "{\n"
    "	fragmentColor = vec4(1,0,0,1);\n" // Output
    "}";

const float cubeVertices[] =
{
    0,0,0, 0,1,0, 1,1,0, 1,0,0,
    0,0,1, 1,0,1, 1,1,1, 0,1,1,
    0,1,0, 0,1,1, 1,1,1, 1,1,0,
    0,0,0, 1,0,0, 1,0,1, 0,0,1,
    1,0,0, 1,1,0, 1,1,1, 1,0,1,
    0,0,0, 0,0,1, 0,1,1, 0,1,0
};

#endif // RAYCASTING_SHADER_H_
