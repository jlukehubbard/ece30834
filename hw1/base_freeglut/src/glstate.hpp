#ifndef GLSTATE_HPP
#define GLSTATE_HPP

#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "gl_core_3_3.h"
#include "camera.hpp"
#include "scene.hpp"

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

	// Camera access
	inline CameraType getCamType() { return whichCam; }
	inline Camera& getCamera(const CameraType camType = GROUND_VIEW) {
		return (camType == GROUND_VIEW) ? camGround : camOverhead;
	}
	inline void switchCam() {  // switch between the two cameras
		whichCam = (whichCam == GROUND_VIEW) ? OVERHEAD_VIEW : GROUND_VIEW;
	}

protected:
	// Initialization
	void initShaders();

	std::unique_ptr<Scene> scene;	// Pointer to the scene object

	// OpenGL state
	GLuint shader;		// GPU shader program
	GLuint xformLoc;	// Transformation matrix location

	// cameras:
	Camera camGround, camOverhead;
	CameraType whichCam = OVERHEAD_VIEW;  // which camera is active currently
};

#endif
