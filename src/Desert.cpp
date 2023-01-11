
#include "Desert.hpp"

#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#pragma warning(pop, 0)

#include "OBJ_Loader.hpp"

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
};

bool Desert::Load()
{
	// Load obj file
	objl::Loader loader;
	loader.LoadFile("../../res/desert.obj");
	if (loader.LoadedMeshes.size() < 1)
		return false;
	objl::Mesh mesh = loader.LoadedMeshes[0];
	std::cout << "loaded desert with " << mesh.Vertices.size() << " vertices and " << mesh.Indices.size() << " indices " << std::endl;

	// Fill vertices
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	for (auto const& it : mesh.Vertices) {
		vertices.push_back({ glm::vec3(it.Position.X, it.Position.Y, it.Position.Z), glm::vec3(it.Normal.X, it.Normal.Y, it.Normal.Z) });
	}
	for (auto const& it : mesh.Indices) {
		indices.push_back(it);
	}
	this->indexCount = (uint32_t)indices.size();

	// Allocate and init buffers
	glCreateBuffers(1, &VBO);
	glNamedBufferStorage(VBO, sizeof(Vertex) * vertices.size(), &vertices.front(), 0);
	glCreateBuffers(1, &IBO);
	glNamedBufferStorage(IBO, sizeof(uint32_t) * indices.size(), &indices.front(), 0);

	// Set data to VAO
   // Init VAO
	glCreateVertexArrays(1, &VAO);
	// Assign VBO
	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(Vertex));
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
	if (!shader.LoadShader("../../res/desert.vert.glsl", GL_VERTEX_SHADER))
		return false;
	if (!shader.LoadShader("../../res/desert.frag.glsl", GL_FRAGMENT_SHADER))
		return false;
	if (shader.LinkProgram() == false)
		return false;
	return true;
}

void Desert::Destroy()
{
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IBO);
	glDeleteBuffers(1, &SSBO);
}

void Desert::Draw()
{
	shader.Use();
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
	glUseProgram(0);
}
