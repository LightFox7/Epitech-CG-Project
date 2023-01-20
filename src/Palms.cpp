
#pragma once
#include "Palms.hpp"

#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#pragma warning(pop, 0)

#include "OBJ_Loader.hpp"

struct PalmVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uvs;
};

bool Palms::Load()
{
	// Load shaders
	if (!shader.LoadShader("../../res/palm.vert.glsl", GL_VERTEX_SHADER) ||
		!shader.LoadShader("../../res/palm.frag.glsl", GL_FRAGMENT_SHADER) ||
		!shader.LinkProgram())
		return false;
	if (!shadowShader.LoadShader("../../res/palm_shadow.vert.glsl", GL_VERTEX_SHADER) ||
		!shadowShader.LoadShader("../../res/palm_shadow.frag.glsl", GL_FRAGMENT_SHADER) ||
		!shadowShader.LinkProgram())
		return false;
	if (!computeShader.LoadShader("../../res/palm_culling.comp.glsl", GL_COMPUTE_SHADER) ||
		!computeShader.LinkProgram())
		return false;
	// Load obj file
	objl::Loader loader;
	loader.LoadFile("../../res/palm.obj");
	if (loader.LoadedMeshes.size() < 1)
		return false;
	objl::Mesh mesh = loader.LoadedMeshes[0];

	// Fill vertices
	std::vector<PalmVertex> vertices;
	std::vector<uint32_t> indices;
	for (auto const& it : mesh.Vertices) {
		vertices.push_back({
			glm::vec3(it.Position.X, it.Position.Y, it.Position.Z),
			glm::vec3(it.Normal.X, it.Normal.Y, it.Normal.Z),
			glm::vec2(it.TextureCoordinate.X, it.TextureCoordinate.Y),
		});
	}
	for (auto const& it : mesh.Indices) {
		indices.push_back(it);
	}
	this->indexCount = (uint32_t)indices.size();

	// Load transforms
	std::vector<glm::vec4> transforms;
	std::ifstream transfoFile;
	std::string line;
	transfoFile.exceptions(std::ifstream::badbit);
	try {
		// Open files
		transfoFile.open("../../res/palmTransfo.txt");
		// Read file's buffer contents into streams
		std::getline(transfoFile, line);
		std::istringstream ss(line);
		ss >> palmCount;
		while (std::getline(transfoFile, line)) {
			ss = std::istringstream(line);
			glm::vec4 vec;
			ss >> vec.x >> vec.y >> vec.z >> vec.w;
			transforms.push_back(vec);
		}
		// close file handler
		transfoFile.close();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::TRANSFORMS::FILE_NOT_SUCCESFULLY_READ: " << "palmTransfo.txt" << std::endl;
		return false;
	}

	transformsSize = sizeof(glm::vec4) * transforms.size();

	// Allocate and init buffers
	glCreateBuffers(1, &VBO);
	glNamedBufferStorage(VBO, sizeof(PalmVertex) * vertices.size(), &vertices.front(), 0);
	glCreateBuffers(1, &IBO);
	glNamedBufferStorage(IBO, sizeof(uint32_t) * indices.size(), &indices.front(), 0);
	glCreateBuffers(1, &SSBO);
	glNamedBufferStorage(SSBO, sizeof(glm::vec4) * (transforms.size() + 1), nullptr, GL_DYNAMIC_STORAGE_BIT);
	glm::ivec4 palmCountV4 = glm::ivec4(palmCount, 0, 0, 0);
	glNamedBufferSubData(SSBO, 0, sizeof(glm::vec4), &palmCountV4);
	glNamedBufferSubData(SSBO, sizeof(glm::vec4), sizeof(glm::vec4) * transforms.size(), &transforms.front());
	// Set data to VAO
   // Init VAO
	glCreateVertexArrays(1, &VAO);
	// Assign VBO
	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(PalmVertex));
	// Assign IBO
	glVertexArrayElementBuffer(VAO, IBO);
	// Define Attributes
	glEnableVertexArrayAttrib(VAO, 0);
	glEnableVertexArrayAttrib(VAO, 1);
	glEnableVertexArrayAttrib(VAO, 2);
	glVertexArrayAttribBinding(VAO, 0, 0);
	glVertexArrayAttribBinding(VAO, 1, 0);
	glVertexArrayAttribBinding(VAO, 2, 0);
	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3));
	glVertexArrayAttribFormat(VAO, 2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2);
	return true;
}

void Palms::Destroy()
{
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IBO);
	glDeleteBuffers(1, &SSBO);
}

void Palms::Render()
{
	// Culling pass
	this->computeShader.Use();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
	glDispatchCompute(palmCount, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glm::ivec4 vec;
	glGetNamedBufferSubData(SSBO, 0, sizeof(glm::vec4), &vec);
	std::cout << vec.x << " " << vec.y << " " << vec.z << " " << vec.w << std::endl;
	glUseProgram(0);
	// Render
	this->shader.Use();
	glDisable(GL_CULL_FACE);
	glBindVertexArray(VAO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
	glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, palmCount);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	glEnable(GL_CULL_FACE);
}

void Palms::RenderShadows()
{
	this->shadowShader.Use();
	glBindVertexArray(VAO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
	glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, palmCount);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

