#define NOMINMAX
#include <fstream>
#include <sstream>
#include "glstate.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "util.hpp"

// Constructor
GLState::GLState() :
	normalMode(NORMALMODE_SMOOTH),
	shadingMode(SHADINGMODE_PHONG),
	width(1), height(1),
	fovy(45.0f),
	camCoords(0.0f, 0.0f, 1.5f),
	camRotating(false),
	shader(0),
	modelMatLoc(0),
	viewProjMatLoc(0),
	normalModeLoc(0),
	shadingModeLoc(0),
	camPosLoc(0),
	objColorLoc(0),
	ambStrLoc(0),
	diffStrLoc(0),
	specStrLoc(0),
	specExpLoc(0) {}

// Destructor
GLState::~GLState() {
	// Release OpenGL resources
	if (shader)	glDeleteProgram(shader);
}

// Called when OpenGL context is created (some time after construction)
void GLState::initializeGL() {
	// General settings
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Initialize OpenGL state
	initShaders();

	// Set drawing state
	setNormalMode(NORMALMODE_SMOOTH);
	setShadingMode(SHADINGMODE_PHONG);

	// Create lights
	lights.resize(Light::MAX_LIGHTS);

	// Set initialized state
	init = true;
}

// Called when window requests a screen redraw
void GLState::paintGL() {
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set shader to draw with
	glUseProgram(shader);

	// Construct a transformation matrix for the camera
	glm::mat4 viewProjMat(1.0f);
	// Perspective projection
	float aspect = (float)width / (float)height;
	glm::mat4 proj = glm::perspective(glm::radians(fovy), aspect, 0.1f, 100.0f);
	// Camera viewpoint
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -camCoords.z));
	view = glm::rotate(view, glm::radians(camCoords.y), glm::vec3(1.0f, 0.0f, 0.0f));
	view = glm::rotate(view, glm::radians(camCoords.x), glm::vec3(0.0f, 1.0f, 0.0f));
	// Combine transformations
	viewProjMat = proj * view;

	if (mesh) {
		// Scale and center mesh using its bounding box
		auto meshBB = mesh->boundingBox();
		glm::mat4 modelMat = glm::scale(glm::mat4(1.0f),
			glm::vec3(1.0f / glm::length(meshBB.second - meshBB.first)));
		modelMat = glm::translate(modelMat, -(meshBB.first + meshBB.second) / 2.0f);
		// Upload transform matrices to shader
		glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
		glUniformMatrix4fv(viewProjMatLoc, 1, GL_FALSE, glm::value_ptr(viewProjMat));

		// Get camera position and upload to shader
		glm::vec3 camPos = glm::vec3(glm::inverse(view)[3]);
		glUniform3fv(camPosLoc, 1, glm::value_ptr(camPos));

		// Draw the mesh
		mesh->draw();
	}

	glUseProgram(0);

	// Draw enabled light icons (if in lighting mode)
	if (shadingMode != SHADINGMODE_NORMALS)
		for (auto& l : lights)
			if (l.getEnabled()) l.drawIcon(viewProjMat);
}

// Called when window is resized
void GLState::resizeGL(int w, int h) {
	// Tell OpenGL the new dimensions of the window
	width = w;
	height = h;
	glViewport(0, 0, w, h);
}

// Set the normal mode (face or smooth)
void GLState::setNormalMode(NormalMode nm) {
	normalMode = nm;

	// Update mode in shader
	glUseProgram(shader);
	glUniform1i(normalModeLoc, (int)normalMode);
	glUseProgram(0);
}

// Set the shading mode (normals or lighting)
void GLState::setShadingMode(ShadingMode sm) {
	shadingMode = sm;

	// Update mode in shader
	glUseProgram(shader);
	glUniform1i(shadingModeLoc, (int)shadingMode);
	glUseProgram(0);
}

// Get object color
glm::vec3 GLState::getObjectColor() const {
	glm::vec3 objColor;
	glGetUniformfv(shader, objColorLoc, glm::value_ptr(objColor));
	return objColor;
}

// Get ambient strength
float GLState::getAmbientStrength() const {
	float ambStr;
	glGetUniformfv(shader, ambStrLoc, &ambStr);
	return ambStr;
}

// Get diffuse strength
float GLState::getDiffuseStrength() const {
	float diffStr;
	glGetUniformfv(shader, diffStrLoc, &diffStr);
	return diffStr;
}

// Get specular strength
float GLState::getSpecularStrength() const {
	float specStr;
	glGetUniformfv(shader, specStrLoc, &specStr);
	return specStr;
}

// Get specular exponent
float GLState::getSpecularExponent() const {
	float specExp;
	glGetUniformfv(shader, specExpLoc, &specExp);
	return specExp;
}

// Set object color
void GLState::setObjectColor(glm::vec3 color) {
	// Update value in shader
	glUseProgram(shader);
	glUniform3fv(objColorLoc, 1, glm::value_ptr(color));
	glUseProgram(0);
}

// Set ambient strength
void GLState::setAmbientStrength(float ambStr) {
	// Update value in shader
	glUseProgram(shader);
	glUniform1f(ambStrLoc, ambStr);
	glUseProgram(0);
}

// Set diffuse strength
void GLState::setDiffuseStrength(float diffStr) {
	// Update value in shader
	glUseProgram(shader);
	glUniform1f(diffStrLoc, diffStr);
	glUseProgram(0);
}

// Set specular strength
void GLState::setSpecularStrength(float specStr) {
	// Update value in shader
	glUseProgram(shader);
	glUniform1f(specStrLoc, specStr);
	glUseProgram(0);
}

// Set specular exponent
void GLState::setSpecularExponent(float specExp) {
	// Update value in shader
	glUseProgram(shader);
	glUniform1f(specExpLoc, specExp);
	glUseProgram(0);
}

// Start rotating the camera (click + drag)
void GLState::beginCameraRotate(glm::vec2 mousePos) {
	camRotating = true;
	initCamRot = glm::vec2(camCoords);
	initMousePos = mousePos;
}

// Stop rotating the camera (mouse button is released)
void GLState::endCameraRotate() {
	camRotating = false;
}

// Use mouse delta to determine new camera rotation
void GLState::rotateCamera(glm::vec2 mousePos) {
	if (camRotating) {
		float rotScale = glm::min(width / 450.0f, height / 270.0f);
		glm::vec2 mouseDelta = mousePos - initMousePos;
		glm::vec2 newAngle = initCamRot + mouseDelta / rotScale;
		newAngle.y = glm::clamp(newAngle.y, -90.0f, 90.0f);
		while (newAngle.x > 180.0f) newAngle.x -= 360.0f;
		while (newAngle.x < -180.0f) newAngle.x += 360.0f;
		if (glm::length(newAngle - glm::vec2(camCoords)) > FLT_EPSILON) {
			camCoords.x = newAngle.x;
			camCoords.y = newAngle.y;
		}
	}
}

// Moves the camera toward / away from the origin (scroll wheel)
void GLState::offsetCamera(float offset) {
	camCoords.z = glm::clamp(camCoords.z + offset, 0.1f, 10.0f);
}

// Display a given .obj file
void GLState::showObjFile(const std::string& filename) {
	// Load the .obj file if it's not already loaded
	if (!mesh || meshFilename != filename) {
		mesh = std::unique_ptr<Mesh>(new Mesh(filename));
		meshFilename = filename;
	}
}

// Create shaders and associated state
void GLState::initShaders() {
	// Compile and link shader files
	std::vector<GLuint> shaders;
	shaders.push_back(compileShader(GL_VERTEX_SHADER, "shaders/v.glsl"));
	shaders.push_back(compileShader(GL_FRAGMENT_SHADER, "shaders/f.glsl"));
	shader = linkProgram(shaders);
	// Cleanup extra state
	for (auto s : shaders)
		glDeleteShader(s);
	shaders.clear();

	// Get uniform locations
	modelMatLoc = glGetUniformLocation(shader, "modelMat");
	viewProjMatLoc = glGetUniformLocation(shader, "viewProjMat");
	normalModeLoc = glGetUniformLocation(shader, "normalMode");
	shadingModeLoc = glGetUniformLocation(shader, "shadingMode");
	camPosLoc = glGetUniformLocation(shader, "camPos");
	objColorLoc = glGetUniformLocation(shader, "objColor");
	ambStrLoc = glGetUniformLocation(shader, "ambStr");
	diffStrLoc = glGetUniformLocation(shader, "diffStr");
	specStrLoc = glGetUniformLocation(shader, "specStr");
	specExpLoc = glGetUniformLocation(shader, "specExp");

	// Bind lights uniform block to binding index
	glUseProgram(shader);
	GLuint lightBlockIndex = glGetUniformBlockIndex(shader, "LightBlock");
	glUniformBlockBinding(shader, lightBlockIndex, Light::BIND_PT);
	glUseProgram(0);
}


// Trim leading and trailing whitespace from a line
std::string trim(const std::string& line) {
	const std::string whitespace = " \t\r\n";
	auto first = line.find_first_not_of(whitespace);
	if (first == std::string::npos)
		return "";
	auto last = line.find_last_not_of(whitespace);
	auto range = last - first + 1;
	return line.substr(first, range);
}

// Reads lines from istream, stripping whitespace and comments,
// until it finds a line with content in it
std::string getNextLine(std::istream& istr) {
	const std::string comment = "#";
	std::string line = "";
	while (line == "") {
		std::getline(istr, line);
		// Skip comments and empty lines
		auto found = line.find(comment);
		if (found != std::string::npos)
			line = line.substr(0, found);
		line = trim(line);
	}
	return line;
}

// Preprocess the file to remove empty lines and comments
std::string preprocessFile(std::string filename) {
	std::ifstream file;
	file.exceptions(std::ios::badbit | std::ios::failbit);
	file.open(filename);

	std::stringstream ss;
	try {
		// Read each line until the end of the file
		while (true) {
			std::string line = getNextLine(file);
			ss << line << std::endl;
		}
	} catch (const std::exception&) {}

	return ss.str();
}

// Read config file
void GLState::readConfig(std::string filename) {
	try {
		// Read the file contents into a string stream
		std::stringstream ss;
		ss.str(preprocessFile(filename));
		ss.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);

		// Read .obj filename
		std::string objName;
		std::getline(ss, objName);
		showObjFile(objName);

		// Read material properties
		float ambStr, diffStr, specStr, specExp;
		glm::vec3 objColor;
		ss >> ambStr >> diffStr >> specStr >> specExp;
		ss >> objColor.r >> objColor.g >> objColor.b;
		objColor /= 255.0f;
		// Set material properties
		setAmbientStrength(ambStr);
		setDiffuseStrength(diffStr);
		setSpecularStrength(specStr);
		setSpecularExponent(specExp);
		setObjectColor(objColor);

		// Read number of lights
		unsigned int numLights;
		ss >> numLights;
		if (numLights == 0)
			throw std::runtime_error("Must have at least 1 light");
		if (numLights >= Light::MAX_LIGHTS)
			throw std::runtime_error("Cannot create more than "
				+ std::to_string(Light::MAX_LIGHTS) + " lights");

		for (unsigned int i = 0; i < lights.size(); i++) {
			// Read properties of each light
			if (i < numLights) {
				int enabled, type;
				glm::vec3 lightColor, lightPos;
				ss >> enabled >> type;
				ss >> lightColor.r >> lightColor.g >> lightColor.b;
				ss >> lightPos.x >> lightPos.y >> lightPos.z;
				lightColor /= 255.0f;
				lights[i].setEnabled((bool)enabled);
				lights[i].setType((Light::LightType)type);
				lights[i].setColor(lightColor);
				lights[i].setPos(lightPos);

			// Disable all other lights
			} else
				lights[i].setEnabled(false);
		}

	} catch (const std::exception& e) {
		// Construct an error message and throw again
		std::stringstream ss;
		ss << "Failed to read config file " << filename << ": " << e.what();
		throw std::runtime_error(ss.str());
	}
}
