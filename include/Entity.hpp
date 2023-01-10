#pragma once

class Entity
{
public:
    virtual ~Entity() = default;
    virtual bool Load() { return true; }
    virtual void Destroy() {};
    virtual void Draw() {};
};