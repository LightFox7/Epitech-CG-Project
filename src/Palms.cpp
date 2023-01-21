
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

typedef  struct {
	uint32_t  count;
	uint32_t  instanceCount;
	uint32_t  firstIndex;
	uint32_t  baseVertex;
	uint32_t  baseInstance;
} DrawElementsIndirectCommand;

void GenerateSphere(std::vector<PalmVertex>& vertices, std::vector<uint32_t>& indices, uint16_t stackCount, uint16_t sectorCount, float radius)
{
	std::vector<PalmVertex>().swap(vertices);
	std::vector<uint32_t>().swap(indices);
	float PI = 3.14f;

	float x, y, z, xy;                              // vertex position
	float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
	float s, t;                                     // vertex texCoord

	float sectorStep = 2 * PI / sectorCount;
	float stackStep = PI / stackCount;
	float sectorAngle, stackAngle;

	for (int i = 0; i <= stackCount; ++i)
	{
		stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
		xy = radius * cosf(stackAngle);             // r * cos(u)
		z = radius * sinf(stackAngle);              // r * sin(u)

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for (int j = 0; j <= sectorCount; ++j)
		{
			sectorAngle = j * sectorStep;           // starting from 0 to 2pi

			// vertex position (x, y, z)
			x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
			y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)

			// normalized vertex normal (nx, ny, nz)
			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;

			// vertex tex coord (s, t) range between [0, 1]
			s = (float)j / sectorCount;
			t = (float)i / stackCount;
			vertices.push_back({ glm::vec3(x, y, z), glm::vec3(nx, ny, nz), glm::vec2(s, t) });
		}
	}

	int k1, k2;
	for (int i = 0; i < stackCount; ++i)
	{
		k1 = i * (sectorCount + 1);
		k2 = k1 + sectorCount + 1;

		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
		{
			if (i != 0)
			{
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			if (i != (stackCount - 1))
			{
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
		}
	}
}

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
	if (!wireframeShader.LoadShader("../../res/palm_wireframe.vert.glsl", GL_VERTEX_SHADER) ||
		!wireframeShader.LoadShader("../../res/palm_wireframe.frag.glsl", GL_FRAGMENT_SHADER) ||
		!wireframeShader.LinkProgram())
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
	glCreateBuffers(1, &SSBO2);
	glNamedBufferStorage(SSBO2, sizeof(uint32_t) * ((uint32_t)palmCount + 1), nullptr, GL_DYNAMIC_STORAGE_BIT);
	glCreateBuffers(1, &indirectBuffer);
	glNamedBufferStorage(indirectBuffer, sizeof(DrawElementsIndirectCommand), nullptr, 0);
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

	// Load sphere VBO (Culling debug)
	std::vector<PalmVertex> sphereVertices;
	std::vector<uint32_t> sphereIndices;
	GenerateSphere(sphereVertices, sphereIndices, 32, 12, 2.5);
	sphereIndicesCount = (uint32_t)(sphereIndices.size());
	// Allocate and init buffers
	glCreateBuffers(1, &VBO2);
	glNamedBufferStorage(VBO2, sizeof(PalmVertex) * sphereVertices.size(), &sphereVertices.front(), 0);
	glCreateBuffers(1, &IBO2);
	glNamedBufferStorage(IBO2, sizeof(uint32_t) * sphereIndices.size(), &sphereIndices.front(), 0);

	// Set data to VAO
	// Init VAO
	glCreateVertexArrays(1, &VAO2);
	// Assign VBO
	glVertexArrayVertexBuffer(VAO2, 0, VBO2, 0, sizeof(PalmVertex));
	// Assign IBO
	glVertexArrayElementBuffer(VAO2, IBO2);
	// Define Attributes
	glEnableVertexArrayAttrib(VAO2, 0);
	glEnableVertexArrayAttrib(VAO2, 1);
	glEnableVertexArrayAttrib(VAO2, 2);
	glVertexArrayAttribBinding(VAO2, 0, 0);
	glVertexArrayAttribBinding(VAO2, 1, 0);
	glVertexArrayAttribBinding(VAO2, 2, 0);
	glVertexArrayAttribFormat(VAO2, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(VAO2, 1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3));
	glVertexArrayAttribFormat(VAO2, 2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2);
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
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, SSBO2);
	glDispatchCompute(64, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glUseProgram(0);

	// Render
	uint32_t count = 0;
	glGetNamedBufferSubData(SSBO2, 0, sizeof(uint32_t), &count);
	this->shader.Use();
	glDisable(GL_CULL_FACE);
	glBindVertexArray(VAO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, SSBO2);
	glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, count);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	glEnable(GL_CULL_FACE);

	// Display sphere
	this->wireframeShader.Use();
	glBindVertexArray(VAO2);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, SSBO2);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElementsInstanced(GL_TRIANGLES, sphereIndicesCount, GL_UNSIGNED_INT, nullptr, palmCount);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindVertexArray(0);
	glUseProgram(0);
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

