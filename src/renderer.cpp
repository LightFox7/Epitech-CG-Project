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

glm::vec3 lightPos = glm::vec3(5.0f, 10.0f, -2.5f);

bool Renderer::Initialize()
{
    // Init UBOData
    UBOData uboData;
    uboData.viewProjectionMatrix = m_Camera->GetViewProjectionMatrix();
    uboData.projectionMatrix = m_Camera->GetProjectionMatrix();
    uboData.viewMatrix = m_Camera->GetViewMatrix();
    uboData.lightDir = glm::normalize(glm::vec4(-lightPos, 0));
    uboData.lightDirViewSpace = uboData.viewMatrix * uboData.lightDir;
    uboData.ambiant = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    uboData.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    glCreateBuffers(1, &m_UBO);
    glNamedBufferStorage(m_UBO, sizeof(UBOData), &uboData, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
    m_UBOData = (UBOData*)glMapNamedBufferRange(m_UBO, 0, sizeof(UBOData),
        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

    LightUBOData lightUboData;
    lightUboData.lightProjectionMatrix = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, -50.0f, 50.0f);
    lightUboData.lightViewMatrix = glm::lookAt(
        m_Camera->GetPosition(),
        m_Camera->GetPosition() + m_UBOData->lightDir.xyz,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    lightUboData.lightViewProjectionMatrix = lightUboData.lightProjectionMatrix * lightUboData.lightViewMatrix;

    glCreateBuffers(1, &m_LightUBO);
    glNamedBufferStorage(m_LightUBO, sizeof(LightUBOData), &lightUboData, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
    m_LightUBOData = (LightUBOData*)glMapNamedBufferRange(m_LightUBO, 0, sizeof(LightUBOData),
        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

    Frustum frustum = GenerateFrustumFromMainCam();
    glCreateBuffers(1, &m_frustumUBO);
    glNamedBufferStorage(m_frustumUBO, sizeof(Frustum), &frustum, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
    m_frustumUBOData = (Frustum*)glMapNamedBufferRange(m_frustumUBO, 0, sizeof(Frustum),
        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

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

    // Create hdr texture
    glCreateTextures(GL_TEXTURE_2D, 1, &m_HDRTexture);
    glTextureStorage2D(m_HDRTexture, 1, GL_RGBA16F, m_ViewportWidth, m_ViewportHeight);
    // Create bright texture
    glCreateTextures(GL_TEXTURE_2D, 1, &m_BrightTexture);
    glTextureStorage2D(m_BrightTexture, 1, GL_RGBA16F, m_ViewportWidth, m_ViewportHeight);
    // Create depth render buffer
    glCreateRenderbuffers(1, &m_depthRBO);
    glNamedRenderbufferStorage(m_depthRBO, GL_DEPTH_COMPONENT, m_ViewportWidth, m_ViewportHeight);
    // Create hdr frame buffer
    glCreateFramebuffers(1, &m_HDRFBO);
    glNamedFramebufferTexture(m_HDRFBO, GL_COLOR_ATTACHMENT0, m_HDRTexture, 0);
    glNamedFramebufferTexture(m_HDRFBO, GL_COLOR_ATTACHMENT1, m_BrightTexture, 0);
    glNamedFramebufferRenderbuffer(m_HDRFBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRBO);
    // Create buffers & textures for blur
    glCreateTextures(GL_TEXTURE_2D, 2, m_PingPongTextures);
    glTextureStorage2D(m_PingPongTextures[0], 1, GL_RGBA16F, m_ViewportWidth, m_ViewportHeight);
    glTextureStorage2D(m_PingPongTextures[1], 1, GL_RGBA16F, m_ViewportWidth, m_ViewportHeight);
    glCreateFramebuffers(2, m_PingPongFBOs);
    glNamedFramebufferTexture(m_PingPongFBOs[0], GL_COLOR_ATTACHMENT0, m_PingPongTextures[0], 0);
    glNamedFramebufferTexture(m_PingPongFBOs[1], GL_COLOR_ATTACHMENT0, m_PingPongTextures[1], 0);

    // Init VAO for quad (no data because vertices are in shader)
    glCreateVertexArrays(1, &m_VAO);

    // Load hdr shader
    hdrShader = std::make_shared<Shader>();
    if (!hdrShader->LoadShader("../../res/quad.vert.glsl", GL_VERTEX_SHADER) ||
        !hdrShader->LoadShader("../../res/hdr.frag.glsl", GL_FRAGMENT_SHADER) ||
        !hdrShader->LinkProgram())
        return false;
    // Load separate bright colors shader
    separateShader = std::make_shared<Shader>();
    if (!separateShader->LoadShader("../../res/quad.vert.glsl", GL_VERTEX_SHADER) ||
        !separateShader->LoadShader("../../res/separate_bright.frag.glsl", GL_FRAGMENT_SHADER) ||
        !separateShader->LinkProgram())
        return false;
    // Load blur shader
    blurShader = std::make_shared<Shader>();
    if (!blurShader->LoadShader("../../res/quad.vert.glsl", GL_VERTEX_SHADER) ||
        !blurShader->LoadShader("../../res/blur.frag.glsl", GL_FRAGMENT_SHADER) ||
        !blurShader->LinkProgram())
        return false;

    // Add entities to list
    entities.push_back(std::make_shared<Palms>());
    entities.push_back(std::make_shared<Desert>());
    // Load entities
    for (auto it : entities) {
        if (!it->Load())
            return false;
    }

    return true;
}

void Renderer::ApplyBloom()
{
    bool horizontal = true, first_iteration = true;
    blurShader->Use();
    for (unsigned int i = 0; i < blur_iterations; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_PingPongFBOs[horizontal]);
        blurShader->SetInt("horizontal", horizontal);
        glBindTextureUnit(0, first_iteration ? m_BrightTexture : m_PingPongTextures[!horizontal]);
        RenderQuad();
        glBindTextureUnit(0, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }
    glUseProgram(0);
}

void Renderer::RenderQuad()
{
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Renderer::Render()
{
    // Render to depth map
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    // Bindings
    GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, 0, m_UBO);
    GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, 1, m_LightUBO);
    GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, 2, m_frustumUBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    for (auto it : entities) {
        it->RenderShadows();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Normal render
    glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, m_HDRFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindTextureUnit(0, m_depthMapTexture);
    // Draw all entities
    for (auto it : entities) {
        it->Render();
    }
    glBindTextureUnit(0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // Unbind uniforms
    GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, 0, 0);
    GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, 1, 0);
    GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, 2, 0);

    // Separate bright colors
    // Draw to 2 color attachments
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glNamedFramebufferDrawBuffers(m_HDRFBO, 2, attachments);
    glBindFramebuffer(GL_FRAMEBUFFER, m_HDRFBO);
    separateShader->Use();
    glBindTextureUnit(0, m_HDRTexture);
    RenderQuad();
    glBindTextureUnit(0, 0);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // Reset to draw only on attachment 0
    glNamedFramebufferDrawBuffers(m_HDRFBO, 1, attachments);

    ApplyBloom();

    // Render quad with hdr texture
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    hdrShader->Use();
    glBindTextureUnit(0, m_HDRTexture);
    glBindTextureUnit(1, m_PingPongTextures[blur_iterations % 2]);
    RenderQuad();
    glBindTextureUnit(0, 0);
    glBindTextureUnit(1, 0);
    glUseProgram(0);
}

void Renderer::Cleanup()
{
    m_UBOData = nullptr;
    m_LightUBOData = nullptr;
    m_frustumUBOData = nullptr;

    // Unmap UBOS
    GL_CALL(glUnmapNamedBuffer, m_UBO);
    GL_CALL(glUnmapNamedBuffer, m_LightUBO);
    GL_CALL(glUnmapNamedBuffer, m_frustumUBO);

    // Delete vertex array
    GL_CALL(glDeleteVertexArrays, 1, &m_VAO);

    // Delete buffers
    GL_CALL(glDeleteBuffers, 1, &m_VAO);
    GL_CALL(glDeleteBuffers, 1, &m_UBO);
    GL_CALL(glDeleteBuffers, 1, &m_LightUBO);
    GL_CALL(glDeleteBuffers, 1, &m_frustumUBO);
    GL_CALL(glDeleteBuffers, 1, &m_depthMapFBO);
    GL_CALL(glDeleteBuffers, 1, &m_HDRFBO);
    GL_CALL(glDeleteBuffers, 1, &m_depthRBO);
    GL_CALL(glDeleteBuffers, 2, m_PingPongFBOs);

    // Delete textures
    GL_CALL(glDeleteTextures, 1, &m_depthMapTexture);
    GL_CALL(glDeleteTextures, 1, &m_HDRTexture);
    GL_CALL(glDeleteTextures, 1, &m_BrightTexture);
    GL_CALL(glDeleteTextures, 2, m_PingPongTextures);

    // Clear entities
    entities.clear();
}

void Renderer::UpdateViewport(uint32_t width, uint32_t height)
{
    m_ViewportWidth = width;
    m_ViewportHeight = height;

    glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
    m_Camera->ComputeProjection(m_ViewportWidth, m_ViewportHeight);

    UpdateCamera();
}

void Renderer::UpdateCamera()
{
    m_UBOData->viewProjectionMatrix = m_Camera->GetViewProjectionMatrix();
    m_UBOData->viewMatrix = m_Camera->GetViewMatrix();
    m_UBOData->projectionMatrix = m_Camera->GetProjectionMatrix();
    m_UBOData->lightDirViewSpace = m_UBOData->viewMatrix * m_UBOData->lightDir;

    glFlushMappedNamedBufferRange(m_UBO, 0, sizeof(UBOData));

    // Update light UBO
    m_LightUBOData->lightViewMatrix = glm::lookAt(
        m_Camera->GetPosition(),
        m_Camera->GetPosition() + m_UBOData->lightDir.xyz,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    m_LightUBOData->lightViewProjectionMatrix = m_LightUBOData->lightProjectionMatrix * m_LightUBOData->lightViewMatrix;
    glFlushMappedNamedBufferRange(m_LightUBO, 0, sizeof(LightUBOData));

    *m_frustumUBOData = GenerateFrustumFromMainCam();
    glFlushMappedNamedBufferRange(m_frustumUBO, 0, sizeof(Frustum));
}

END_VISUALIZER_NAMESPACE
