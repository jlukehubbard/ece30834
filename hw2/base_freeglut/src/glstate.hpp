#ifndef GLSTATE_HPP
#define GLSTATE_HPP

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "gl_core_3_3.h"
#include "mesh.hpp"
#include "light.hpp"

// Manages OpenGL state, e.g. camera transform, objects, shaders
class GLState {
public:
	GLState();
	~GLState();
	// Disallow copy, move, & assignment
	GLState(const GLState& other) = delete;
	GLState& operator=(const GLState& other) = delete;
	GLState(GLState&& other) = delete;
	GLState& operator=(GLState&& other) = delete;

	// Callbacks
	void initializeGL();
	void paintGL();
	void resizeGL(int w, int h);

	// Enums
	enum NormalMode {
		NORMALMODE_FACE = 0,		// Flat normals
		NORMALMODE_SMOOTH = 1,		// Smooth normals
	};
	enum ShadingMode {
		SHADINGMODE_NORMALS = 0,	// View normals as colors
		SHADINGMODE_PHONG = 1,		// Use Phong shading and illumination
		SHADINGMODE_GOURAUD = 2,	// Use Gouraud shading
	};

	bool isInit() const { return init; }
	void readConfig(std::string filename);	// Read from a config file

	// Drawing modes
	NormalMode getNormalMode() const { return normalMode; }
	ShadingMode getShadingMode() const { return shadingMode; }
	void setNormalMode(NormalMode nm);
	void setShadingMode(ShadingMode sm);

	// Object properties
	float getAmbientStrength() const;
	float getDiffuseStrength() const;
	float getSpecularStrength() const;
	float getSpecularExponent() const;
	glm::vec3 getObjectColor() const;
	void setAmbientStrength(float ambStr);
	void setDiffuseStrength(float diffStr);
	void setSpecularStrength(float specStr);
	void setSpecularExponent(float specExp);
	void setObjectColor(glm::vec3 color);

	// Mesh & Light access
	std::string getMeshFilename() const { return meshFilename; }
	unsigned int getNumLights() const { return (unsigned int)lights.size(); }
	Light& getLight(int index) { return lights.at(index); }
	const Light& getLight(int index) const { return lights[index]; }

	// Camera control
	bool isCamRotating() const { return camRotating; }
	void beginCameraRotate(glm::vec2 mousePos);
	void endCameraRotate();
	void rotateCamera(glm::vec2 mousePos);
	void offsetCamera(float offset);

	// Set object to display
	void showObjFile(const std::string& filename);

protected:
	bool init;						// Whether we've been initialized yet

	// Initialization
	void initShaders();

	// Drawing modes
	NormalMode normalMode;
	ShadingMode shadingMode;

	// Camera state
	int width, height;		// Width and height of the window
	float fovy;				// Vertical field of view in degrees
	glm::vec3 camCoords;	// Camera spherical coordinates
	bool camRotating;		// Whether camera is currently rotating
	glm::vec2 initCamRot;	// Initial camera rotation on click
	glm::vec2 initMousePos;	// Initial mouse position on click

	// Mesh and lights
	std::string meshFilename;		// Name of the obj file being shown
	std::unique_ptr<Mesh> mesh;		// Pointer to mesh object
	std::vector<Light> lights;		// Lights

	// Shader state
	GLuint shader;			// GPU shader program
	GLuint modelMatLoc;		// Model-to-world matrix location
	GLuint viewProjMatLoc;	// World-to-clip matrix location
	GLuint normalModeLoc;	// Normal mode location
	GLuint shadingModeLoc;	// Shading mode location
	GLuint camPosLoc;		// Camera position location
	GLuint objColorLoc;		// Object color
	GLuint ambStrLoc;		// Ambient strength location
	GLuint diffStrLoc;		// Diffuse strength location
	GLuint specStrLoc;		// Specular strength location
	GLuint specExpLoc;		// Specular exponent location
};

#endif
