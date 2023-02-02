#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "Entity.hpp"

BEGIN_VISUALIZER_NAMESPACE

class Camera;

struct UBOData {
    glm::mat4 viewProjectionMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::vec4 lightDir;
    glm::vec4 lightDirViewSpace;
    glm::vec4 ambiant;
    glm::vec4 diffuse;
};

// Light data for shadow mapping
struct LightUBOData {
    glm::mat4 lightProjectionMatrix;
    glm::mat4 lightViewMatrix;
    glm::mat4 lightViewProjectionMatrix;
};

struct Frustum
{
    glm::vec4 topFace;
    glm::vec4 bottomFace;
    glm::vec4 rightFace;
    glm::vec4 leftFace;
    glm::vec4 farFace;
    glm::vec4 nearFace;
};

class Renderer
{
public:
    Renderer(uint32_t width, uint32_t height, const std::shared_ptr<Camera>& camera)
        : m_ViewportWidth(width)
        , m_ViewportHeight(height)
        , m_Camera(camera)
    {}

    Renderer() = delete;
    ~Renderer() = default;
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    bool Initialize();
    void Render();
    void Cleanup();

    void UpdateViewport(uint32_t width, uint32_t height);
    void UpdateCamera();

    void ApplyBloom();
    void RenderQuad();

    Frustum GenerateFrustumFromMainCam()
    {
        Frustum frustum;
        const glm::vec3 farPlaneCenter = m_Camera->GetFar() * m_Camera->GetDirection();
        const float halfVSide = m_Camera->GetFar() * glm::tan(glm::radians(m_Camera->GetFOV()) / 2.0f);
        const float halfHSide = halfVSide * m_Camera->GetAspectRatio();
        frustum.nearFace = glm::vec4(m_Camera->GetDirection(), 0);
        frustum.nearFace.w = glm::dot(glm::vec3(frustum.nearFace.xyz), m_Camera->GetPosition() + m_Camera->GetNear() * m_Camera->GetDirection());
        frustum.farFace = glm::vec4(-m_Camera->GetDirection(), 0);
        frustum.farFace.w = glm::dot(glm::vec3(frustum.farFace.xyz), m_Camera->GetPosition() + farPlaneCenter);
        frustum.rightFace = glm::vec4(glm::cross(m_Camera->GetUp(), glm::normalize(farPlaneCenter + m_Camera->GetRight() * halfHSide)), 0);
        frustum.rightFace.w = glm::dot(glm::vec3(frustum.rightFace.xyz), m_Camera->GetPosition());
        frustum.leftFace = glm::vec4(glm::cross(glm::normalize(farPlaneCenter - m_Camera->GetRight() * halfHSide), m_Camera->GetUp()), 0);
        frustum.leftFace.w = glm::dot(glm::vec3(frustum.leftFace.xyz), m_Camera->GetPosition());
        frustum.topFace = glm::vec4(glm::cross(m_Camera->GetRight(), glm::normalize(farPlaneCenter - m_Camera->GetUp() * halfVSide)), 0);
        frustum.topFace.w = glm::dot(glm::vec3(frustum.topFace.xyz), m_Camera->GetPosition());
        frustum.bottomFace = glm::vec4(glm::cross(glm::normalize(farPlaneCenter + m_Camera->GetUp() * halfVSide), m_Camera->GetRight()), 0);
        frustum.bottomFace.w = glm::dot(glm::vec3(frustum.bottomFace.xyz), m_Camera->GetPosition());
        return frustum;
    }

private:
    std::shared_ptr<Camera> m_Camera;
    uint32_t m_ViewportWidth, m_ViewportHeight;

    // Custom variables
    // UBOs
    GLuint m_UBO, m_LightUBO, m_frustumUBO;
    UBOData* m_UBOData;
    LightUBOData* m_LightUBOData;
    Frustum* m_frustumUBOData;
    // FBOs & RBO
    GLuint m_depthMapFBO, m_HDRFBO, m_depthRBO;
    GLuint m_PingPongFBOs[2];
    const unsigned int blur_iterations = 20;
    // Textures
    GLuint m_depthMapTexture, m_HDRTexture, m_BrightTexture;
    GLuint m_PingPongTextures[2];
    // VAO
    GLuint m_VAO;

    std::shared_ptr<Shader> separateShader;
    std::shared_ptr<Shader> blurShader;
    std::shared_ptr<Shader> hdrShader;

    // Entities to render
    std::vector<std::shared_ptr<Entity>> entities;
};

END_VISUALIZER_NAMESPACE

#endif // !RENDERER_HPP
