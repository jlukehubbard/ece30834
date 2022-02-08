#define NOMINMAX
#include <iostream>
#include "camera.hpp"
#include "scene.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;

static GLdouble camRot(0.0);
static glm::vec3 camPos(0.0, 10.0, 0.0);
static glm::vec3 camDir(0.0, 0.0, -1.0);

static void updateDir(void) {
	camDir = glm::vec3(glm::sin(glm::radians(camRot)), 0.0f, glm::cos(glm::radians(camRot)));
}

static void updateRot(GLdouble step) {
	camRot += step;
	updateDir();
}

// constructor
Camera::Camera(const CameraType cType) :
	camType(cType),
	width(1), height(1),
	fovy(45.0f),
	view(glm::mat4(1.0f)), proj(glm::mat4(1.0f)) {

	// *********************************** TODO: this line below is hard-coded to help you get a better initial view of the scene, REMOVE it afterwards
	
	updateViewProj();
}

void Camera::turnLeft() {
	// *********************************** TODO
	updateRot(-(this -> rotStep));
	updateViewProj();
}

void Camera::turnRight() {
	// *********************************** TODO
	updateRot(this -> rotStep);
	updateViewProj();
}

void Camera::moveForward() {
	// *********************************** TODO
	camPos += glm::vec3(camDir.x * moveStep, camDir.y * moveStep, camDir.z * moveStep);
	updateViewProj();
}

void Camera::moveBackward() {
	// *********************************** TODO
	camPos -= glm::vec3(camDir.x * moveStep, camDir.y * moveStep, camDir.z * moveStep);
	updateViewProj();
}

#define GLDFHALF(num) ((GLdouble) num) / ((GLdouble) 2.0f)

void Camera::updateViewProj() {
	// *********************************** TODO
	GLdouble aspect = ((GLdouble) this->width)/ ((GLdouble) this->height);


	switch (this -> camType) {
	case GROUND_VIEW:
	case OVERHEAD_VIEW:
		this -> proj = glm::perspective<GLdouble>(fovy, aspect, 0.1, 150.0);
		//this -> view = ViewPitchYaw(camPos, 45.0f, camRot);
		//break;
		//this -> proj = glm::ortho<GLdouble>(-GLDFHALF(width), GLDFHALF(width), -GLDFHALF(height), GLDFHALF(height), 0.1, 1000.0);
		this -> view = LookAt(camPos, camPos + camDir, glm::normalize(camDir + glm::vec3(0.0f, 10.0f, 0.0f)));
	}


}

glm::mat4 Camera::LookAt(glm::vec3 eye, glm::vec3 at, glm::vec3 up) {
	/*
	 * Based on: https://www.geertarien.com/blog/2017/07/30/breakdown-of-the-lookAt-function-in-OpenGL/ 
	 */

	
	//zhat direction is the direction the camera is pointing (forget the sign for now)
	glm::vec3 zhat = glm::normalize(at - eye);
	//xhat direction is perpendicular to up and to zhat
	glm::vec3 xhat = glm::normalize(glm::cross(zhat, up));
	//yhat is perpendicular to xhat and zhat.
	glm::vec3 yhat = glm::cross(xhat, zhat);

	//negate zhat to convert from LH to RH coordinates
	zhat = -zhat;

	//This matrix combines the world-camera transformation and the translation to account for camera position
	glm::mat4 viewmat = {
		glm::vec4(xhat.x, xhat.y, xhat.z, -glm::dot(xhat, eye)),
		glm::vec4(yhat.x, yhat.y, yhat.z, -glm::dot(yhat, eye)),
		glm::vec4(zhat.x, zhat.y, zhat.z, -glm::dot(zhat, eye)),
		glm::vec4(0, 0, 0, 1)
	};

/*	
	glm::mat4 viewmat = {
		glm::vec4(xhat.x, yhat.x, zhat.x, 0.0f),
		glm::vec4(xhat.x, yhat.x, zhat.x, 0.0f),
		glm::vec4(xhat.x, yhat.x, zhat.x, 0.0f),
		glm::vec4(-glm::dot(xhat, eye), -glm::dot(yhat, eye), -glm::dot(zhat, eye), 1.0f)
	};
*/
	//viewmat = glm::transpose(viewmat);

	return viewmat;
}

glm::mat4 Camera::ViewPitchYaw(glm::vec3 eye, GLdouble pitch, GLdouble yaw) {
	/*
	 * Based on: https://www.3dgep.com/understanding-the-view-matrix/#The_View_Matrix
	 */
	
	// assume p+y are in degrees because I'm the one writing the software and I feel like it
	GLdouble cosPitch = glm::cos(glm::radians(pitch));
	GLdouble sinPitch = glm::sin(glm::radians(pitch));
	GLdouble cosYaw = glm::cos(glm::radians(yaw));
	GLdouble sinYaw = glm::sin(glm::radians(yaw));

	glm::vec3 xhat(cosYaw, 0.0f, -sinYaw);
	glm::vec3 yhat(sinYaw * sinPitch, cosPitch, cosYaw * sinPitch);
	glm::vec3 zhat(sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw);
	
	glm::mat4 viewmat = {
		glm::vec4(xhat.x, xhat.y, xhat.z, -glm::dot(xhat, eye)),
		glm::vec4(yhat.x, yhat.y, yhat.z, -glm::dot(yhat, eye)),
		glm::vec4(zhat.x, zhat.y, zhat.z, -glm::dot(zhat, eye)),
		glm::vec4(0, 0, 0, 1)
	};

	return viewmat;
}
















