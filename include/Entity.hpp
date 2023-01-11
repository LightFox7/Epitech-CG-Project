#pragma once

class Entity
{
public:
    virtual ~Entity() = default;
    virtual bool Load() = 0;
    virtual void Destroy() = 0;
    virtual void Draw() = 0;
};