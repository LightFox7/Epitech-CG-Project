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
    Frustum createFrustumFromCamera(const Camera& cam)
    {
        Frustum frustum;
        const float aspect = cam.GetAspectRatio();
        const float fovY = cam.GetFOV();
        const float zNear = cam.GetNear();
        const float zFar = cam.GetFar();
        const glm::vec3 dir = glm::normalize(cam.GetDirection());
        const glm::vec3 right = glm::normalize(cam.GetRight());
        const glm::vec3 up = glm::normalize(cam.GetUp());
        const glm::vec3 pos = cam.GetPosition();
        const float halfVSide = zFar * tanf(fovY * .5f);
        const float halfHSide = halfVSide * aspect;
        const glm::vec3 frontMultFar = zFar * dir;
        frustum.nearFace = glm::vec4(dir, glm::length(pos + zNear * dir));
        frustum.farFace = glm::vec4(-dir, glm::length(pos + frontMultFar));
        frustum.rightFace = glm::vec4(glm::cross(frontMultFar - right * halfHSide, up), glm::length(pos));
        frustum.leftFace = glm::vec4(glm::cross(up, frontMultFar + right * halfHSide), glm::length(pos));
        frustum.topFace = glm::vec4(glm::cross(right, frontMultFar - up * halfVSide), glm::length(pos));
        frustum.bottomFace = glm::vec4(glm::cross(frontMultFar + up * halfVSide, right), glm::length(pos));
        return frustum;
    }

private:
    std::shared_ptr<Camera> m_Camera;
    uint32_t m_ViewportWidth, m_ViewportHeight;

    // Custom variables
    GLuint m_UBO, m_LightUBO, m_depthMapFBO, m_depthMapTexture, frustumUBO;
    UBOData* m_UBOData;
    LightUBOData* m_LightUBOData;
    Frustum* frustumUBOData;
 
    // Entities to render
    std::vector<std::shared_ptr<Entity>> entities;
};

END_VISUALIZER_NAMESPACE

#endif // !RENDERER_HPP
