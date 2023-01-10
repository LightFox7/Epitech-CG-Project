#pragma once

#include "Entity.hpp"
#include "Shader.hpp"

class Palms : public Entity
{
public:
    Palms() {
        this->VAO = 0;
        this->VBO = 0;
        this->shader = Shader();
        this->indexCount = 0;
    }
    ~Palms() {}
    bool Load() override;
    void Destroy() override;
    void Draw() override;

private:
    GLuint VAO, VBO, IBO, SSBO;
    uint32_t indexCount;
    Shader shader;
};