#include <GL/glew.h>

#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#pragma warning(pop, 0)

#include <glutils.hpp>
#include <camera.hpp>
#include "renderer.hpp"

// Custom includes
#include "Palms.hpp"
#include "Desert.hpp"

BEGIN_VISUALIZER_NAMESPACE

bool Renderer::Initialize()
{
    GL_CALL(glCreateBuffers, 1, &m_UBO);
    GL_CALL(glNamedBufferStorage, m_UBO, sizeof(glm::mat4), glm::value_ptr(m_Camera->GetViewProjectionMatrix()), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

    m_UBOData = GL_CALL_REINTERPRET_CAST_RETURN_VALUE(glm::mat4*, glMapNamedBufferRange, m_UBO, 0, sizeof(glm::mat4), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

    // Add entities to list
    entities.push_back(std::make_shared<Palms>());
    entities.push_back(std::make_shared<Desert>());
    // Load entities
    for (auto it : entities) {
        if (!it->Load())
            return false;
    }

    // Set clear color to something easier to distinguish
    glClearColor(0.1f, 0.1f, 0.3f, 1.f);
    return true;
}

void Renderer::Render()
{
    GL_CALL(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw all entities (using same uniform in 0)
    GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, 0, m_UBO);
    for (auto it : entities) {
        it->Draw();
    }
    GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, 0, 0);
}

void Renderer::Cleanup()
{
    m_UBOData = nullptr;

    GL_CALL(glUnmapNamedBuffer, m_UBO);

    GL_CALL(glDeleteBuffers, 1, &m_UBO);

    entities.clear();
}

void Renderer::UpdateViewport(uint32_t width, uint32_t height)
{
    m_ViewportWidth = width;
    m_ViewportHeight = height;

    glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
    m_Camera->ComputeProjection(m_ViewportWidth, m_ViewportHeight);

    std::memcpy(m_UBOData, glm::value_ptr(m_Camera->GetViewProjectionMatrix()), sizeof(glm::mat4));
    GL_CALL(glFlushMappedNamedBufferRange, m_UBO, 0, sizeof(glm::mat4));
}

void Renderer::UpdateCamera()
{
    std::memcpy(m_UBOData, glm::value_ptr(m_Camera->GetViewProjectionMatrix()), sizeof(glm::mat4));
    GL_CALL(glFlushMappedNamedBufferRange, m_UBO, 0, sizeof(glm::mat4));
}

END_VISUALIZER_NAMESPACE
