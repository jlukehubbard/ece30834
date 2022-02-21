#define NOMINMAX
#include <iostream>
#include <memory>
#include <filesystem>
#include <algorithm>
#include "glstate.hpp"
#include <GL/freeglut.h>
namespace fs = std::filesystem;

// Menu identifiers
const int MENU_OBJBASE = 64;				// Select object to view
const int MENU_NORMALS_FLAT = 2;			// Normals mode
const int MENU_NORMALS_SMOOTH = 3;
const int MENU_SHADING_PHONG = 4;			// Shading mode
const int MENU_SHADING_GOURAUD = 5;
const int MENU_SHADING_NORMALS = 6;
const int MENU_PRESETS_GOLD = 7;			// Presets
const int MENU_PRESETS_OBSIDIAN = 8;
const int MENU_PRESETS_PEARL = 9;
const int MENU_EXIT = 1;					// Exit application
std::vector<std::string> meshFilenames;		// Paths to .obj files to load

// OpenGL state
int width, height;
std::unique_ptr<GLState> glState;
unsigned int activeLight = 0;

// Initialization functions
void initGLUT(int* argc, char** argv);
void initMenu();
void findObjFiles();

// Callback functions
void display();
void reshape(GLint width, GLint height);
void keyPress(unsigned char key, int x, int y);
void keyRelease(unsigned char key, int x, int y);
void mouseBtn(int button, int state, int x, int y);
void mouseMove(int x, int y);
void idle();
void menu(int cmd);
void cleanup();

// Program entry point
int main(int argc, char** argv) {
	std::string configFile = "config.txt";
	if (argc > 1)
		configFile = std::string(argv[1]);

	try {
		// Create the window and menu
		initGLUT(&argc, argv);
		initMenu();
		// Initialize OpenGL (buffers, shaders, etc.)
		glState = std::unique_ptr<GLState>(new GLState());
		glState->initializeGL();
		glState->readConfig(configFile);

	} catch (const std::exception& e) {
		// Handle any errors
		std::cerr << "Fatal error: " << e.what() << std::endl;
		cleanup();
		return -1;
	}

	std::cout << "Mouse controls:" << std::endl;
	std::cout << "  Left click + drag to rotate camera" << std::endl;
	std::cout << "  Scroll wheel to zoom in/out" << std::endl;
	std::cout << "  SHIFT + left click + drag to rotate active light source" << std::endl;
	std::cout << "  SHIFT + scroll wheel to change active light distance" << std::endl;
	std::cout << "Keyboard controls:" << std::endl;
	std::cout << "  1-8:  Change active light source" << std::endl;
	std::cout << "  e:    Enable/disable active light source" << std::endl;
	std::cout << "  t:    Toggle active light type (point vs. directional)" << std::endl;
	std::cout << "  a,A:  Decrease/increase ambient strength" << std::endl;
	std::cout << "  d,D:  Decrease/increase diffuse strength" << std::endl;
	std::cout << "  s,S:  Decrease/increase specular strength" << std::endl;
	std::cout << "  x,X:  Decrease/increase specular exponent" << std::endl;
	std::cout << "  n:    Toggle normals type (flat vs. smooth)" << std::endl;
	std::cout << "  l,L:  Toggle shading type (Phong vs. Gouraud vs. colored normals)" << std::endl;
	std::cout << std::endl;
	std::cout << "Active light: " << activeLight+1 << std::endl;

	// Execute main loop
	glutMainLoop();

	return 0;
}

// Setup window and callbacks
void initGLUT(int* argc, char** argv) {
	// Set window and context settings
	width = 800; height = 600;
	glutInit(argc, argv);
	glutInitWindowSize(width, height);
	//glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	// Create the window
	glutCreateWindow("FreeGLUT Window");

	// Create a menu

	// GLUT callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyPress);
	glutKeyboardUpFunc(keyRelease);
	glutMouseFunc(mouseBtn);
	glutMotionFunc(mouseMove);
	glutIdleFunc(idle);
	glutCloseFunc(cleanup);
}

void initMenu() {
	// Create a submenu with all the objects you can view
	findObjFiles();
	int objMenu = glutCreateMenu(menu);
	for (int i = 0; i < meshFilenames.size(); i++) {
		glutAddMenuEntry(meshFilenames[i].c_str(), MENU_OBJBASE + i);
	}

	int normalsMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Flat", MENU_NORMALS_FLAT);
	glutAddMenuEntry("Smooth", MENU_NORMALS_SMOOTH);

	int shadingMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Phong", MENU_SHADING_PHONG);
	glutAddMenuEntry("Gouraud", MENU_SHADING_GOURAUD);
	glutAddMenuEntry("Normals", MENU_SHADING_NORMALS);

	int presetsMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Gold", MENU_PRESETS_GOLD);
	glutAddMenuEntry("Obsidian", MENU_PRESETS_OBSIDIAN);
	glutAddMenuEntry("Pearl", MENU_PRESETS_PEARL);

	// Create the main menu, adding the objects menu as a submenu
	glutCreateMenu(menu);
	glutAddSubMenu("View object", objMenu);
	glutAddSubMenu("Normals", normalsMenu);
	glutAddSubMenu("Shading", shadingMenu);
	glutAddSubMenu("Presets", presetsMenu);
	glutAddMenuEntry("Exit", MENU_EXIT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

}

void findObjFiles() {
	// Search the models/ directory for any file ending in .obj
	fs::path modelsDir = "models";
	for (auto& di : fs::directory_iterator(modelsDir)) {
		if (di.is_regular_file() && di.path().extension() == ".obj")
			meshFilenames.push_back(di.path().string());
	}
	std::sort(meshFilenames.begin(), meshFilenames.end());
}

// Called whenever a screen redraw is requested
void display() {
	// Tell the GLState to render the scene
	glState->paintGL();

	// Scene is rendered to the back buffer, so swap the buffers to display it
	glutSwapBuffers();
}

// Called when the window is resized
void reshape(GLint w, GLint h) {
	// Tell OpenGL the new window size
	width = w; height = h;
	glState->resizeGL(width, height);
}

// Called when a key is pressed
void keyPress(unsigned char key, int x, int y) {
	switch (key) {
	// Toggle normals type (flat vs smooth)
	case 'n':
	case 'N': {
		GLState::NormalMode nm = glState->getNormalMode();
		if (nm == GLState::NORMALMODE_FACE) {
			glState->setNormalMode(GLState::NORMALMODE_SMOOTH);
			std::cout << "Showing smoothed normals" << std::endl;
		} else if (nm == GLState::NORMALMODE_SMOOTH) {
			glState->setNormalMode(GLState::NORMALMODE_FACE);
			std::cout << "Showing flat normals" << std::endl;
		}
		glutPostRedisplay();
		break; }
	// Toggle shading mode (normals vs Phong)
	case 'l': {
		GLState::ShadingMode sm = glState->getShadingMode();
		if (sm == GLState::SHADINGMODE_NORMALS) {
			glState->setShadingMode(GLState::SHADINGMODE_PHONG);
			std::cout << "Showing Phong shading & illumination" << std::endl;
		} else if (sm == GLState::SHADINGMODE_PHONG) {
			glState->setShadingMode(GLState::SHADINGMODE_GOURAUD);
			std::cout << "Showing Gouraud shading" << std::endl;
		} else if (sm == GLState::SHADINGMODE_GOURAUD) {
			glState->setShadingMode(GLState::SHADINGMODE_NORMALS);
			std::cout << "Showing normals as colors" << std::endl;
		}
		glutPostRedisplay();
		break; }
	// Toggle shading mode (normals vs Phong)
	case 'L': {
		GLState::ShadingMode sm = glState->getShadingMode();
		if (sm == GLState::SHADINGMODE_NORMALS) {
			glState->setShadingMode(GLState::SHADINGMODE_GOURAUD);
			std::cout << "Showing Gouraud shading" << std::endl;
		} else if (sm == GLState::SHADINGMODE_PHONG) {
			glState->setShadingMode(GLState::SHADINGMODE_NORMALS);
			std::cout << "Showing normals as colors" << std::endl;
		} else if (sm == GLState::SHADINGMODE_GOURAUD) {
			glState->setShadingMode(GLState::SHADINGMODE_PHONG);
			std::cout << "Showing Phong shading & illumination" << std::endl;
		}
		glutPostRedisplay();
		break; }
	// Decrease ambient lighting
	case 'a': {
		float ambStr = glState->getAmbientStrength();
		ambStr = glm::max(0.0f, ambStr - 0.02f);
		glState->setAmbientStrength(ambStr);
		std::cout << "Set ambient strength to " << ambStr << std::endl;
		glutPostRedisplay();
		break; }
	// Increase ambient lighting
	case 'A': {
		float ambStr = glState->getAmbientStrength();
		ambStr = glm::min(1.0f, ambStr + 0.02f);
		glState->setAmbientStrength(ambStr);
		std::cout << "Set ambient strength to " << ambStr << std::endl;
		glutPostRedisplay();
		break; }
	// Decrease diffuse lighting
	case 'd': {
		float diffStr = glState->getDiffuseStrength();
		diffStr = glm::max(0.0f, diffStr - 0.1f);
		glState->setDiffuseStrength(diffStr);
		std::cout << "Set diffuse strength to " << diffStr << std::endl;
		glutPostRedisplay();
		break; }
	// Increase diffuse lighting
	case 'D': {
		float diffStr = glState->getDiffuseStrength();
		diffStr = glm::min(1.0f, diffStr + 0.1f);
		glState->setDiffuseStrength(diffStr);
		std::cout << "Set diffuse strength to " << diffStr << std::endl;
		glutPostRedisplay();
		break; }
	// Decrease specular lighting
	case 's': {
		float specStr = glState->getSpecularStrength();
		specStr = glm::max(0.0f, specStr - 0.1f);
		glState->setSpecularStrength(specStr);
		std::cout << "Set specular strength to " << specStr << std::endl;
		glutPostRedisplay();
		break; }
	// Increase specular lighting
	case 'S': {
		float specStr = glState->getSpecularStrength();
		specStr = glm::min(1.0f, specStr + 0.1f);
		glState->setSpecularStrength(specStr);
		std::cout << "Set specular strength to " << specStr << std::endl;
		glutPostRedisplay();
		break; }
	// Decrease specular exponent
	case 'x': {
		float specExp = glState->getSpecularExponent();
		specExp = glm::max(1.0f, specExp / 2.0f);
		glState->setSpecularExponent(specExp);
		std::cout << "Set specular exponent to " << specExp << std::endl;
		glutPostRedisplay();
		break; }
	// Increase specular exponent
	case 'X': {
		float specExp = glState->getSpecularExponent();
		specExp = glm::min(1024.0f, specExp * 2.0f);
		glState->setSpecularExponent(specExp);
		std::cout << "Set specular exponent to " << specExp << std::endl;
		glutPostRedisplay();
		break; }
	// Switch active light source
	case '1':
		if (glState->getNumLights() > 0) {
			activeLight = 0;
			std::cout << "Active light: " << activeLight+1 << std::endl;
		} break;
	case '2':
		if (glState->getNumLights() > 1) {
			activeLight = 1;
			std::cout << "Active light: " << activeLight+1 << std::endl;
		} break;
	case '3':
		if (glState->getNumLights() > 2) {
			activeLight = 2;
			std::cout << "Active light: " << activeLight+1 << std::endl;
		} break;
	case '4':
		if (glState->getNumLights() > 3) {
			activeLight = 3;
			std::cout << "Active light: " << activeLight+1 << std::endl;
		} break;
	case '5':
		if (glState->getNumLights() > 4) {
			activeLight = 4;
			std::cout << "Active light: " << activeLight+1 << std::endl;
		} break;
	case '6':
		if (glState->getNumLights() > 5) {
			activeLight = 5;
			std::cout << "Active light: " << activeLight+1 << std::endl;
		} break;
	case '7':
		if (glState->getNumLights() > 6) {
			activeLight = 6;
			std::cout << "Active light: " << activeLight+1 << std::endl;
		} break;
	case '8':
		if (glState->getNumLights() > 7) {
			activeLight = 7;
			std::cout << "Active light: " << activeLight+1 << std::endl;
		} break;
	// Enable / disable active light
	case 'e':
	case 'E': {
		bool enabled = glState->getLight(activeLight).getEnabled();
		enabled = !enabled;
		glState->getLight(activeLight).setEnabled(enabled);
		std::cout << (enabled ? "Enabled" : "Disabled") << " light " << activeLight+1 << std::endl;
		glutPostRedisplay();
		break; }
	// Toggle active light type
	case 't':
	case 'T': {
		Light::LightType type = glState->getLight(activeLight).getType();
		if (type == Light::POINT) {
			glState->getLight(activeLight).setType(Light::DIRECTIONAL);
			std::cout << "Set light " << activeLight+1 << " to directional light" << std::endl;
		} else if (type == Light::DIRECTIONAL) {
			glState->getLight(activeLight).setType(Light::POINT);
			std::cout << "Set light " << activeLight+1 << " to point light" << std::endl;
		}
		glutPostRedisplay();
		break; }
	default:
		break;
	}
}

// Called when a key is released
void keyRelease(unsigned char key, int x, int y) {
	switch (key) {
	case 27:	// Escape key
		menu(MENU_EXIT);
		break;
	}
}

// Called when a mouse button is pressed or released
void mouseBtn(int button, int state, int x, int y) {
	int modifiers = glutGetModifiers();

	// Press left mouse button
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
		// Start rotating the active light if holding shift
		if (modifiers & GLUT_ACTIVE_SHIFT) {
			float scale = glm::min((float)width, (float)height);
			glState->getLight(activeLight).beginRotate(
				glm::vec2(x / scale, y / scale));

		// Start rotating the camera otherwise
		} else
			glState->beginCameraRotate(glm::vec2(x, y));
	}
	// Release left mouse button
	if (state == GLUT_UP && button == GLUT_LEFT_BUTTON) {
		// Stop camera and light rotation
		glState->endCameraRotate();
		glState->getLight(activeLight).endRotate();
	}
	// Scroll wheel up
	if (button == 3) {
		// Offset the active light if holding shift
		if (modifiers & GLUT_ACTIVE_SHIFT)
			glState->getLight(activeLight).offsetLight(-0.05f);

		// "Zoom in" otherwise
		else
			glState->offsetCamera(-0.1f);
		glutPostRedisplay();
	}
	// Scroll wheel down
	if (button == 4) {
		// Offset the active light if holding shift
		if (modifiers & GLUT_ACTIVE_SHIFT)
			glState->getLight(activeLight).offsetLight(0.05f);

		// "Zoom out" otherwise
		else
			glState->offsetCamera(0.1f);
		glutPostRedisplay();
	}
}

// Called when the mouse moves
void mouseMove(int x, int y) {
	if (glState->isCamRotating()) {
		// Rotate the camera if currently rotating
		glState->rotateCamera(glm::vec2(x, y));
		glutPostRedisplay();	// Request redraw

	} else if (glState->getLight(activeLight).isRotating()) {
		float scale = glm::min((float)width, (float)height);
		glState->getLight(activeLight).rotateLight(
			glm::vec2(x / scale, y / scale));
		glutPostRedisplay();
	}
}

// Called when there are no events to process
void idle() {
	// Anything that happens every frame (e.g. movement) should be done here
	// Be sure to call glutPostRedisplay() if the screen needs to update as well
}

// Called when a menu button is pressed
void menu(int cmd) {
	switch (cmd) {
	// End the program
	case MENU_EXIT:
		glutLeaveMainLoop();
		break;

	// Show flat normals
	case MENU_NORMALS_FLAT:
		glState->setNormalMode(GLState::NORMALMODE_FACE);
		glutPostRedisplay();
		break;

	// Show smooth normals
	case MENU_NORMALS_SMOOTH:
		glState->setNormalMode(GLState::NORMALMODE_SMOOTH);
		glutPostRedisplay();
		break;

	// Show Phong shading & illumination
	case MENU_SHADING_PHONG:
		glState->setShadingMode(GLState::SHADINGMODE_PHONG);
		glutPostRedisplay();
		break;

	// Show Gouraud shading
	case MENU_SHADING_GOURAUD:
		glState->setShadingMode(GLState::SHADINGMODE_GOURAUD);
		glutPostRedisplay();
		break;

	// Show normals as colors
	case MENU_SHADING_NORMALS:
		glState->setShadingMode(GLState::SHADINGMODE_NORMALS);
		glutPostRedisplay();
		break;

	// Presets
	case MENU_PRESETS_GOLD:
		glState->readConfig("config_gold.txt");
		glutPostRedisplay();
		break;

	case MENU_PRESETS_OBSIDIAN:
		glState->readConfig("config_obsidian.txt");
		glutPostRedisplay();
		break;

	case MENU_PRESETS_PEARL:
		glState->readConfig("config_pearl.txt");
		glutPostRedisplay();
		break;

	default:
		// Show the other objects
		if (cmd >= MENU_OBJBASE) {
			try {
				glState->showObjFile(meshFilenames[cmd - MENU_OBJBASE]);
				glutPostRedisplay();	// Request redraw
			// Might fail to load object
			} catch (const std::exception& e) {
				std::cerr << e.what() << std::endl;
			}
		}
		break;
	}
}

// Called when the window is closed or the event loop is otherwise exited
void cleanup() {
	// Delete the GLState object, calling its destructor,
	// which releases the OpenGL objects
	glState.reset(nullptr);
}
