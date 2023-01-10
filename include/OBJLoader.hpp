#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#pragma warning(pop, 0)

#include <GL/glew.h>

struct OBJData {
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> uvs;
	std::vector<glm::vec3> normals;
	std::vector<GLuint> indices;
};

class OBJLoader
{
public:
	static OBJData LoadFile(std::string path);
};

