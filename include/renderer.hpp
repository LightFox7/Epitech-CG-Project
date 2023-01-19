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

private:
    std::shared_ptr<Camera> m_Camera;
    uint32_t m_ViewportWidth, m_ViewportHeight;

    // Custom variables
    GLuint m_UBO, m_LightUBO, m_depthMapFBO, m_depthMapTexture;
    UBOData* m_UBOData;
    LightUBOData* m_LightUBOData;
 
    // Entities to render
    std::vector<std::shared_ptr<Entity>> entities;
};

END_VISUALIZER_NAMESPACE

#endif // !RENDERER_HPP
