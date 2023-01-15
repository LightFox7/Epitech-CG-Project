#pragma once

#include "Entity.hpp"
#include "Shader.hpp"

class Desert : public Entity
{
public:
    Desert()
    {
        this->VAO = 0;
        this->VBO = 0;
        this->IBO = 0;
        this->SSBO = 0;
        this->shader = Shader();
        this->shadowShader = Shader();
        this->indexCount = 0;
        this->palmCount = 0;
    }
    ~Desert() {}
    bool Load() override;
    void Destroy() override;
    void Render() override;
    void RenderShadows() override;

private:
    GLuint VAO, VBO, IBO, SSBO;
    uint32_t indexCount, palmCount;
    Shader shader;
    // Shader for shadow mapping
    Shader shadowShader;
};