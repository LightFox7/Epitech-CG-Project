#pragma once

#include "Shader.hpp"

class Entity
{
public:
    virtual ~Entity() = default;
    virtual bool Load() = 0;
    virtual void Destroy() = 0;
    virtual void Render() = 0;
    virtual void RenderShadows() = 0;
};