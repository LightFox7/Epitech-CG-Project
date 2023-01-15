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

const GLuint SHADOW_WIDTH = 8192, SHADOW_HEIGHT = 8192;

bool Renderer::Initialize()
{
    // Init UBOData
    UBOData uboData;
    uboData.viewProjectionMatrix = m_Camera->GetViewProjectionMatrix();
    uboData.projectionMatrix = m_Camera->GetProjectionMatrix();
    uboData.viewMatrix = m_Camera->GetViewMatrix();
    uboData.lightPos = glm::vec4(5.0f, 10.0f, -2.5f, 0.0f);
    uboData.lightDir = glm::normalize(-uboData.lightPos);
    uboData.lightDirViewSpace = uboData.viewMatrix * uboData.lightDir;
    uboData.ambiant = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    uboData.diffuse = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);

    glCreateBuffers(1, &m_UBO);
    glNamedBufferStorage(m_UBO, sizeof(UBOData), &uboData, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
    m_UBOData = (UBOData*)glMapNamedBufferRange(m_UBO, 0, sizeof(UBOData),
        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

    LightUBOData lightUboData;
    lightUboData.lightProjectionMatrix = glm::ortho(-260.0f, 260.0f, -260.0f, 260.0f, 1.0f, 100.0f);
    lightUboData.lightViewMatrix = glm::lookAt(
        glm::vec3(m_UBOData->lightPos.xyz),
        glm::vec3(0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    lightUboData.lightViewProjectionMatrix = lightUboData.lightProjectionMatrix * lightUboData.lightViewMatrix;

    glCreateBuffers(1, &m_LightUBO);
    glNamedBufferStorage(m_LightUBO, sizeof(LightUBOData), &lightUboData, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
    m_LightUBOData = (LightUBOData*)glMapNamedBufferRange(m_LightUBO, 0, sizeof(LightUBOData),
        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

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

    // Create depth map texture
    glCreateTextures(GL_TEXTURE_2D, 1, &m_depthMapTexture);
    glTextureStorage2D(m_depthMapTexture, 1, GL_DEPTH_COMPONENT32F, SHADOW_WIDTH, SHADOW_HEIGHT);
    glTextureParameteri(m_depthMapTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_depthMapTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_depthMapTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(m_depthMapTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTextureParameterfv(m_depthMapTexture, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Create depth map frame buffer
    glCreateFramebuffers(1, &m_depthMapFBO);
    glNamedFramebufferTexture(m_depthMapFBO, GL_DEPTH_ATTACHMENT, m_depthMapTexture, 0);
    glNamedFramebufferDrawBuffer(m_depthMapFBO, GL_NONE);
    glNamedFramebufferReadBuffer(m_depthMapFBO, GL_NONE);

    return true;
}

void Renderer::Render()
{
    // Bind uniforms
    GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, 0, m_UBO);
    GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, 1, m_LightUBO);
    // Render to depth map
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    for (auto it : entities) {
        it->RenderShadows();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Normal render
    glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
    GL_CALL(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindTextureUnit(0, m_depthMapTexture);
    // Draw all entities
    for (auto it : entities) {
        it->Render();
    }
    glBindTextureUnit(0, 0);

    // Unbind uniforms
    GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, 0, 0);
    GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, 1, 0);
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

    m_UBOData->viewProjectionMatrix = m_Camera->GetViewProjectionMatrix();
    m_UBOData->viewMatrix = m_Camera->GetViewMatrix();
    m_UBOData->projectionMatrix = m_Camera->GetProjectionMatrix();

    GL_CALL(glFlushMappedNamedBufferRange, m_UBO, 0, sizeof(UBOData));
}

void Renderer::UpdateCamera()
{
    m_UBOData->viewProjectionMatrix = m_Camera->GetViewProjectionMatrix();
    m_UBOData->viewMatrix = m_Camera->GetViewMatrix();
    m_UBOData->projectionMatrix = m_Camera->GetProjectionMatrix();

    GL_CALL(glFlushMappedNamedBufferRange, m_UBO, 0, sizeof(UBOData));
}

END_VISUALIZER_NAMESPACE
