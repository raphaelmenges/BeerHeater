#ifndef FLUIDSIMULATIONSHADER_H_
#define FLUIDSIMULATIONSHADER_H_

const char* fluidSimComputeShader =
"#version 430 core\n"

// Structs
"struct Mat{\n"
"	vec4 color;\n"
"	vec4 cisf;\n"
"	vec4 dppp;\n"
"};\n"
"struct FanStruct{\n"
"   vec3 position;\n"
"   float speed;\n"
"   vec3 direction;\n"
"   float distance;\n"
"};\n"

// Workgroup settings
"layout(local_size_x=8, local_size_y=8, local_size_z=8) in;\n"

// SSBOs
"layout(std430, binding = 0) buffer Material\n"
"{\n"
"	Mat m[];\n"
"};\n"
"layout(std430, binding = 1) buffer Fan\n"
"{\n"
"	FanStruct fans[];\n"
"};\n"

// Uniforms
"layout(rgba32f, location = 0) uniform image3D stateVolume;\n"
"layout(r32f, location = 1) uniform image3D lookupVolume;\n"
"uniform float timeStep;\n"
"uniform float edgeLength;\n"
"uniform int relaxationSteps;\n"
"uniform int fanCount;\n"

// Consts
"const float gravity = -0;\n" // Not set
"const float thermalExpansionCoefficient = 0.00025;\n"
"const float viscosity = 0.0001568f;\n"
"const float limitation = 0.07;\n"

// Global variables
"vec4 myState;\n"
"float inverseVoxelEdgeArea;\n"

// Is fluid
"bool isFluid(ivec3 coords)"
"{\n"
"	float fluid = m[int(imageLoad(lookupVolume, coords).x)].cisf.w;\n"
"	return bool(fluid > 0);\n"
"}\n"

// Get state
"vec4 getState(ivec3 coords)"
"{\n"
"	return imageLoad(stateVolume, coords);\n"
"}\n"

// Wind
"void wind(ivec3 coords)\n"
"{\n"
"	for(int i = 0; i < fanCount; i++)\n"
"	{\n"
"		vec3 relCoords = coords;\n"
"		relCoords /= 128.0;\n" // TODO: Voxel count guessed...
"		float inFront = max(0,sign(dot(-fans[i].position+relCoords, fans[i].direction)));\n" // Figure out, whether voxel is in front of fan
"		float distanceFalloff = 1.0 - clamp(abs(length(relCoords - fans[i].position)) / 0.2, 0, 1);\n" // Falloff by distance
"		vec3 wind = distanceFalloff * inFront * fans[i].direction * fans[i].speed;\n"
"		myState.yzw = length(wind) > length(myState.yzw) ? wind : myState.yzw;\n" // Just the wind if it is strong enough
"	}\n"
"	imageStore(stateVolume, coords, myState);\n" // Save changed state for other programs
"	barrier();\n" // Necessary for storing?
"};\n"

// Buoyancy
"void buoyancy(ivec3 coords)\n"
"{\n"
"	float downForce = gravity * timeStep;\n" // Down (should be negative)
"	float upForce = thermalExpansionCoefficient * timeStep;\n" // Up
"   float averageTemperature = 0;\n"  // TODO
"	myState.z -= (downForce-upForce) * myState.x + upForce * averageTemperature;\n"
//  f[i][j] += (g - b) * t[i][j] + b * t0;
"	imageStore(stateVolume, coords, myState);\n" // Save changed state for other programs
"	barrier();\n" // Necessary for storing?
"};\n"

// Diffuse
"void diffuse(ivec3 coords)"
"{\n"
"	vec4 myInitialState = myState;\n"
"	float h = timeStep * viscosity * inverseVoxelEdgeArea;\n"
"	float normalization = 1 / (1 + 2 * (h + h));\n" // TODO: Normalization but in 3D (formula still 2D...)
//  float dn = 1f / (1 + 2 * (hx + hy));
"	for(int i = 0; i < relaxationSteps; i++)\n"
"	{\n" // Doing diffusion in all three directions at once per step
"       vec4 leftState = getState(coords+ivec3(1,0,0));\n"
"       vec4 rightState = getState(coords+ivec3(-1,0,0));\n"
"       vec4 topState = getState(coords+ivec3(0,1,0));\n"
"       vec4 downState = getState(coords+ivec3(0,-1,0));\n"
"       vec4 frontState = getState(coords+ivec3(0,0,1));\n"
"       vec4 backState = getState(coords+ivec3(0,0,-1));\n"
"		myState.y"
"           = (myInitialState.y "
"           + h * (leftState.y + rightState.y)"
"           + h * (topState.y + downState.y)"
"           + h * (frontState.y + backState.y))"
"           * normalization;\n"
"		myState.z"
"           = (myInitialState.z "
"           + h * (leftState.z + rightState.z)"
"           + h * (topState.z + downState.z)"
"           + h * (frontState.z + backState.z))"
"           * normalization;\n"
"		myState.w"
"           = (myInitialState.w "
"           + h * (leftState.w + rightState.w)"
"           + h * (topState.w + downState.w)"
"           + h * (frontState.w + backState.w))"
"           * normalization;\n"
//		f[i][j] = (f0[i][j] + hx * (f[i - 1][j] + f[i + 1][j]) + hy * (f[i][j - 1] + f[i][j + 1])) * dn;
"		imageStore(stateVolume, coords, myState);\n"
"       barrier();\n"
"	}\n"
"}\n"

// Conserve
"void conserve(ivec3 coords)"
"{\n"
    // Grab values from neighbors
"   vec4 leftState = getState(coords+ivec3(1,0,0));\n"
"   vec4 rightState = getState(coords+ivec3(-1,0,0));\n"
"   vec4 topState = getState(coords+ivec3(0,1,0));\n"
"   vec4 downState = getState(coords+ivec3(0,-1,0));\n"
"   vec4 frontState = getState(coords+ivec3(0,0,1));\n"
"   vec4 backState = getState(coords+ivec3(0,0,-1));\n"
    // Use volume to save phi and div
"   float normalization = 0.5 / edgeLength;\n"
"   vec4 value = vec4(0,0,0,0);\n"
"   value.x = 0;\n"
"   value.y "
"   = ((leftState.y - rightState.y)"
"   + (topState.z - downState.z)"
"   + (frontState.w - backState.w)) * normalization;\n"
//  div[i][j] = (u[i + 1][j] - u[i - 1][j]) * i2dx + (v[i][j + 1] - v[i][j - 1]) * i2dy;
//	phi[i][j] = 0;
"   imageStore(stateVolume, coords, value);\n" // Abuse state volume for temporaly value storing
"   barrier();\n"
//  Relaxation
"   float idsq = (1.0 / (2.0 * edgeLength));\n"
"   normalization = 0.5 / (2.0 * idsq);\n"
"	for(int i = 0; i < relaxationSteps; i++)\n"
"	{\n"
"       vec4 leftValue = getState(coords+ivec3(1,0,0));\n"
"       vec4 rightValue = getState(coords+ivec3(-1,0,0));\n"
"       vec4 topValue = getState(coords+ivec3(0,1,0));\n"
"       vec4 downValue = getState(coords+ivec3(0,-1,0));\n"
"       vec4 frontValue = getState(coords+ivec3(0,0,1));\n"
"       vec4 backValue = getState(coords+ivec3(0,0,-1));\n"
"       value.x "
"       = normalization "
"       * (((leftValue.x + rightValue.x)"
"       + (topValue.x + downValue.x)"
"       + (frontValue.x + backValue.x)) "
"       * idsq)"
"       - value.y;\n"
//      phi[i][j] = s * ((phi[i - 1][j] + phi[i + 1][j]) * idxsq + (phi[i][j - 1] + phi[i][j + 1]) * idysq - div[i][j]);
"		imageStore(stateVolume, coords, value);\n" // Save changed state for neighbors
"       barrier();\n" // Neighbors must update before next cycle
"   }\n"
"   vec4 leftValue = getState(coords+ivec3(1,0,0));\n"
"   vec4 rightValue = getState(coords+ivec3(-1,0,0));\n"
"   vec4 topValue = getState(coords+ivec3(0,1,0));\n"
"   vec4 downValue = getState(coords+ivec3(0,-1,0));\n"
"   vec4 frontValue = getState(coords+ivec3(0,0,1));\n"
"   vec4 backValue = getState(coords+ivec3(0,0,-1));\n"
"   float i2d = 0.5 / edgeLength;\n" // Correct?
"   myState.y -= (leftValue.x - rightValue.x) * i2d;\n"
"   myState.z -= (topValue.x - downValue.x) * i2d;\n"
"   myState.w -= (frontValue.x - backValue.x) * i2d;\n"
"   imageStore(stateVolume, coords, myState);\n" // Save changed state for neighbors
"   barrier();"
"}\n"

// Advect
"void advect(ivec3 coords)"
"{\n"
"   vec4 myOldState = myState;"
"   float normalization = 0.5 * timeStep / edgeLength;\n"
"   vec4 leftState = getState(coords+ivec3(1,0,0));\n"
"   vec4 rightState = getState(coords+ivec3(-1,0,0));\n"
"   vec4 topState = getState(coords+ivec3(0,1,0));\n"
"   vec4 downState = getState(coords+ivec3(0,-1,0));\n"
"   vec4 frontState = getState(coords+ivec3(0,0,1));\n"
"   vec4 backState = getState(coords+ivec3(0,0,-1));\n"
//  Reduce own state by difference of squared neihgbors' values multiplied with some normalization
"   myState.y"
"   = myState.y"
"   - normalization * (leftState.y*leftState.y - rightState.y*rightState.y)"
"   - normalization * (topState.y*topState.y - downState.y*downState.y)"
"   - normalization * (frontState.y*frontState.y - backState.y*backState.y);"
"   myState.z"
"   = myState.z"
"   - normalization * (leftState.z*leftState.z - rightState.z*rightState.z)"
"   - normalization * (topState.z*topState.z - downState.z*downState.z)"
"   - normalization * (frontState.z*frontState.z - backState.z*backState.z);"
"   myState.w"
"   = myState.w"
"   - normalization * (leftState.w*leftState.w - rightState.w*rightState.w)"
"   - normalization * (topState.w*topState.w - downState.w*downState.w)"
"   - normalization * (frontState.w*frontState.w - backState.w*backState.w);"
//  f[i][j] = f0[i][j] - tx * (u0[i + 1][j] * f0[i + 1][j] - u0[i - 1][j] * f0[i - 1][j]) - ty * (v0[i][j + 1] * f0[i][j + 1] - v0[i][j - 1] * f0[i][j - 1]);
"   imageStore(stateVolume, coords, myState);\n"
"   barrier();\n"
"   leftState = getState(coords+ivec3(1,0,0));\n"
"   rightState = getState(coords+ivec3(-1,0,0));\n"
"   topState = getState(coords+ivec3(0,1,0));\n"
"   downState = getState(coords+ivec3(0,-1,0));\n"
"   frontState = getState(coords+ivec3(0,0,1));\n"
"   backState = getState(coords+ivec3(0,0,-1));\n"
//  Weight own state with state at beginning of function and differences of neighbors' values
"   myState.y"
"   = 0.5 * (myOldState.y + myState.y)\n"
"   - 0.5 * normalization * myOldState.y * (leftState.y - rightState.y)\n"
"   - 0.5 * normalization * myOldState.y * (topState.y - downState.y)\n"
"   - 0.5 * normalization * myOldState.y * (frontState.y - backState.y);\n"
"   myState.z"
"   = 0.5 * (myOldState.z + myState.z)\n"
"   - 0.5 * normalization * myOldState.z * (leftState.z - rightState.z)\n"
"   - 0.5 * normalization * myOldState.z * (topState.z - downState.z)\n"
"   - 0.5 * normalization * myOldState.z * (frontState.z - backState.z);\n"
"   myState.w"
"   = 0.5 * (myOldState.w + myState.w)\n"
"   - 0.5 * normalization * myOldState.w * (leftState.w - rightState.w)\n"
"   - 0.5 * normalization * myOldState.w * (topState.w - downState.w)\n"
"   - 0.5 * normalization * myOldState.w * (frontState.w - backState.w);\n"
//  f0[i][j] = 0.5f * (f0[i][j] + f[i][j])
//  - 0.5f * tx * u0[i][j] * (f[i + 1][j] - f[i - 1][j])
//  - 0.5f * ty * v0[i][j] * (f[i][j + 1] - f[i][j - 1]);
"   imageStore(stateVolume, coords, myState);\n"
"   barrier();\n"
"}\n"

// Collide with static environment
"void collide(ivec3 coords)"
"{\n"
"   bool fluidLeft = 0 < m[int(imageLoad(lookupVolume, coords+ivec3(1,0,0)).x)].cisf.w;\n"
"   bool fluidRight = 0 < m[int(imageLoad(lookupVolume, coords+ivec3(-1,0,0)).x)].cisf.w;\n"
"   bool fluidTop = 0 < m[int(imageLoad(lookupVolume, coords+ivec3(0,1,0)).x)].cisf.w;\n"
"   bool fluidDown = 0 < m[int(imageLoad(lookupVolume, coords+ivec3(0,-1,0)).x)].cisf.w;\n"
"   bool fluidFront = 0 < m[int(imageLoad(lookupVolume, coords+ivec3(0,0,1)).x)].cisf.w;\n"
"   bool fluidBack = 0 < m[int(imageLoad(lookupVolume, coords+ivec3(0,0,-1)).x)].cisf.w;\n"
"   vec4 leftState = getState(coords+ivec3(1,0,0));\n"
"   vec4 rightState = getState(coords+ivec3(-1,0,0));\n"
"   vec4 topState = getState(coords+ivec3(0,1,0));\n"
"   vec4 downState = getState(coords+ivec3(0,-1,0));\n"
"   vec4 frontState = getState(coords+ivec3(0,0,1));\n"
"   vec4 backState = getState(coords+ivec3(0,0,-1));\n"
"   myState.yzw = vec3(0,0,0);\n"
"   if(fluidRight)\n"
"	{\n"
"       myState.y = -rightState.y;\n"
"       myState.z = rightState.z;\n"
"       myState.w = rightState.w;\n"
"	}\n"
"   else if(fluidLeft)\n"
"	{\n"
"       myState.y = -leftState.y;\n"
"       myState.z = leftState.z;\n"
"       myState.w = leftState.w;\n"
"	}\n"
"   if(fluidTop)\n"
"	{\n"
"       myState.y = topState.y;\n"
"       myState.z = -topState.z;\n"
"       myState.w = topState.w;\n"
"	}\n"
"   else if(fluidDown)\n"
"	{\n"
"       myState.y = downState.y;\n"
"       myState.z = -downState.z;\n"
"       myState.w = downState.w;\n"
"	}\n"
"   if(fluidFront)\n"
"	{\n"
"       myState.y = frontState.y;\n"
"       myState.z = frontState.z;\n"
"       myState.w = -frontState.w;\n"
"	}\n"
"   else if(fluidBack)\n"
"	{\n"
"       myState.y = backState.y;\n"
"       myState.z = backState.z;\n"
"       myState.w = -backState.w;\n"
"	}\n"
"   imageStore(stateVolume, coords, myState);\n"
"   barrier();\n"
"}\n"

// Limitation of speed (not physically correct...)
"void limit(ivec3 coords)\n"
"{\n"
"   myState.y = max(min(myState.y, limitation), -limitation);\n"
"   myState.z = max(min(myState.z, limitation), -limitation);\n"
"   myState.w = max(min(myState.w, limitation), -limitation);\n"
"   imageStore(stateVolume, coords, myState);\n"
"   barrier();\n"
"}\n"

// Main
"void main()\n"
"{\n"
"   bool fluid = isFluid(ivec3(gl_GlobalInvocationID));\n"
"   myState = imageLoad(stateVolume, ivec3(gl_GlobalInvocationID));\n" // Get initial state
"   vec4 oldState = myState;\n"
"	inverseVoxelEdgeArea = 1.0 / edgeLength * edgeLength;\n"
"   wind(ivec3(gl_GlobalInvocationID));\n" // Just the fans overwritting the velocities
"	buoyancy(ivec3(gl_GlobalInvocationID));\n" // Upthrust depending on average temperature
"	diffuse(ivec3(gl_GlobalInvocationID));\n" // Do diffusion which is much like smoothing
"	advect(ivec3(gl_GlobalInvocationID));\n" // Difference of temperature and velocity of neighbors used
"   // conserve(ivec3(gl_GlobalInvocationID));\n" // Does not work :( and no exact idea what it is doing
"	limit(ivec3(gl_GlobalInvocationID));\n" // More or less simple replacement for conserve
"	if(!fluid)\n"
"	{\n"
"		myState = oldState;\n"
"		collide(ivec3(gl_GlobalInvocationID));\n"
"		imageStore(stateVolume, ivec3(gl_GlobalInvocationID), myState);\n" // Has to be done this way, becaue barrier not allowed inside of an if
"	}\n"
"}\n";

#endif // FLUIDSIMULATIONSHADER_H_
