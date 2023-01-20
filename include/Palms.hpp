#pragma once

#include "Entity.hpp"
#include "Shader.hpp"

class Palms : public Entity
{
public:
    Palms() {
        this->VAO = 0;
        this->VBO = 0;
        this->IBO = 0;
        this->SSBO = 0;
        this->SSBO2 = 0;
        this->indexCount = 0;
        this->palmCount = 0;
        this->transformsSize = 0;
        this->shader = Shader();
        this->shadowShader = Shader();
        this->computeShader = Shader();
    }
    ~Palms() {}
    bool Load() override;
    void Destroy() override;
    void Render() override;
    void RenderShadows() override;

private:
    // SSBO for transforms, SSBO2 for culling
    GLuint VAO, VBO, IBO, SSBO, SSBO2;
    uint32_t indexCount, palmCount;
    std::size_t transformsSize;
    Shader shader;
    // Shader for shadow mapping
    Shader shadowShader;
    Shader computeShader;
};