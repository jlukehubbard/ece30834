#version 330

const int NORMALMODE_FACE = 0;			// Flat normals
const int NORMALMODE_SMOOTH = 1;		// Smooth normals

const int SHADINGMODE_GOURAUD = 2;

layout(location = 0) in vec3 pos;			// Model-space position
layout(location = 1) in vec3 face_norm;		// Model-space face normal
layout(location = 2) in vec3 smooth_norm;	// Model-space smoothed normal

smooth out vec3 fragPos;	// Interpolated position in world-space
smooth out vec3 fragNorm;	// Interpolated normal in world-space

uniform mat4 modelMat;		// Model-to-world transform matrix
uniform mat4 viewProjMat;	// World-to-clip transform matrix
uniform int normalMode;		// Face normals or smooth normals

void main() {
	// Choose which normals to use
	vec3 norm;
	if (normalMode == NORMALMODE_FACE)
		norm = face_norm;
	else if (normalMode == NORMALMODE_SMOOTH)
		norm = smooth_norm;

	// Get world-space position and normal
	fragPos = vec3(modelMat * vec4(pos, 1.0));
	fragNorm = vec3(modelMat * vec4(norm, 0.0));

	// Output clip-space position
	gl_Position = viewProjMat * vec4(fragPos, 1.0);

	// TODO (Extra credit) =========================================================
	// Implement Gouraud shading
}
