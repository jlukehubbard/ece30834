#define NOMINMAX
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "scene.hpp"
using namespace std;
namespace fs = std::filesystem;

std::string trim(const std::string& line);

void Scene::parseScene() {  // TODO: rewrite
	fs::path modelsDir = "models";					// Models directory
	fs::path sceneFile = modelsDir / "scene.txt";	// Scene file
	ifstream istr;
	istr.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		// Read the number of objects in the scene
		istr.open(sceneFile);
		istr >> nObj;

		while (objects.size() < nObj) {
			// Skip any lines that don't contain ".obj" (e.g. whitespace)
			string line;
			std::size_t found = std::string::npos;
			while (found == std::string::npos) {
				getline(istr, line);
				found = line.find(".obj");
			}

			// Load the mesh
			string objFilename = trim(line);
			auto mesh = std::shared_ptr<Mesh>(new Mesh((modelsDir / objFilename).string()));  // construct the mesh
			objects.push_back(mesh);  // store the mesh

			// TODO: read the rotation and translation of the mesh
			
			glm::mat4 model(1.0f);

			for (int row = 0; row < 3; row++) {
				for (int col = 0; col < 3; col++) {
					istr >> model[col][row];
				}
			}
			for (int row = 0; row < 3; row++) {
				istr >> model[3][row];
			}
			(*mesh).setModelMat(model);
		}
	}
	catch (const std::exception& e) {
		// Construct an error message and throw again
		std::stringstream ss;
		ss << "Failed to read " << sceneFile << ": " << e.what();
		throw std::runtime_error(ss.str());
	}
}

void Scene::printMat3(const glm::mat3 mat) {
	for (int i = 0; i < 3; i++) {  // copy the rotation matrix
		for (int j = 0; j < 3; j++) {
			std::cout << mat[i][j] << " ";
		}
		std::cout << "\n";
	}
	std::cout << std::endl;
}

void Scene::printMat4(const glm::mat4 mat) {
	for (int i = 0; i < 4; i++) {  // copy the rotation matrix
		for (int j = 0; j < 4; j++) {
			std::cout << mat[i][j] << " ";
		}
		std::cout << "\n";
	}
	std::cout << std::endl;
}

// Trim leading and trailing whitespace
std::string trim(const std::string& line) {
	const std::string whitespace = " \t\r\n";
	auto first = line.find_first_not_of(whitespace);
	if (first == std::string::npos)
		return "";
	auto last = line.find_last_not_of(whitespace);
	auto range = last - first + 1;
	return line.substr(first, range);
}
