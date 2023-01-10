
#include "Palms.hpp"

#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#pragma warning(pop, 0)

#include "OBJ_Loader.h"

struct PalmVertex {
	glm::vec3 position;
	glm::vec3 normal;
};

bool Palms::Load()
{
	// Load obj file
	objl::Loader loader;
	loader.LoadFile("../../res/palm.obj");
	if (loader.LoadedMeshes.size() < 1)
		return false;
	objl::Mesh mesh = loader.LoadedMeshes[0];
	std::cout << "loaded palm with " << mesh.Vertices.size() << " vertices and " << mesh.Indices.size() << " indices " << std::endl;

	// Fill vertices
	std::vector<PalmVertex> vertices;
	std::vector<uint32_t> indices;
	for (auto it : mesh.Vertices) {
		vertices.push_back({ glm::vec3(it.Position.X, it.Position.Y, it.Position.Z), glm::vec3(it.Normal.X, it.Normal.Y, it.Normal.Z) });
	}
	for (auto it : mesh.Indices) {
		indices.push_back(it);
	}
	this->indexCount = (uint32_t)indices.size();

	// TODO Load transforms
	std::vector<glm::vec4> transforms;
	transforms.push_back(glm::vec4(25, 25, 25, 1));
	transforms.push_back(glm::vec4(0, 0, 0, 1));

	// Allocate and init buffers
	glCreateBuffers(1, &VBO);
	glNamedBufferStorage(VBO, sizeof(PalmVertex) * vertices.size(), &vertices.front(), 0);
	glCreateBuffers(1, &IBO);
	glNamedBufferStorage(IBO, sizeof(uint32_t) * indices.size(), &indices.front(), 0);
	glCreateBuffers(1, &SSBO);
	glNamedBufferStorage(SSBO, sizeof(glm::vec4) * transforms.size(), &transforms.front(), GL_DYNAMIC_STORAGE_BIT);

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
	glVertexArrayAttribBinding(VAO, 0, 0);
	glVertexArrayAttribBinding(VAO, 1, 0);
	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3));

	// Load shader
	if (!shader.LoadShader("../../res/palm.vert.glsl", GL_VERTEX_SHADER))
		return false;
	if (!shader.LoadShader("../../res/palm.frag.glsl", GL_FRAGMENT_SHADER))
		return false;
	if (shader.LinkProgram() == false)
		return false;
	return true;
}

void Palms::Destroy()
{
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IBO);
	glDeleteBuffers(1, &SSBO);
}

void Palms::Draw()
{
	glDisable(GL_CULL_FACE);
	shader.Use();
	glBindVertexArray(VAO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, SSBO);
	glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, 30);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	glEnable(GL_CULL_FACE);
}
