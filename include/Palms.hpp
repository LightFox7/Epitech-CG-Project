#pragma once

#include "Entity.hpp"
#include "Shader.hpp"

class Palms : public Entity
{
public:
    Palms() {
        this->VAO = 0;
        this->VAO2 = 0;
        this->VBO = 0;
        this->VBO2 = 0;
        this->IBO = 0;
        this->IBO2 = 0;
        this->SSBO = 0;
        this->SSBO2 = 0;
        this->indexCount = 0;
        this->palmCount = 0;
        this->sphereIndicesCount = 0;
        this->shader = Shader();
        this->shadowShader = Shader();
        this->computeShader = Shader();
        this->wireframeShader = Shader();
    }
    ~Palms() {}
    bool Load() override;
    void Destroy() override;
    void Render() override;
    void RenderShadows() override;

private:
    // SSBO for transforms, SSBO2 for culling
    GLuint VAO, VAO2, VBO, VBO2, IBO, IBO2, SSBO, SSBO2, indirectBuffer;
    uint32_t indexCount, palmCount, sphereIndicesCount;
    Shader shader;
    // Shader for shadow mapping
    Shader shadowShader;
    Shader computeShader;
    Shader wireframeShader;
};