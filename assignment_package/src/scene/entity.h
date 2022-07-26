#pragma once
#include "glm_includes.h"
#include "terrain.h"

struct InputBundle {
    bool wPressed, aPressed, sPressed, dPressed, qPressed, ePressed;
    bool spacePressed;
    float mouseX, mouseY;

    InputBundle()
        : wPressed(false), aPressed(false), sPressed(false),
          dPressed(false), qPressed(false), ePressed(false),
          spacePressed(false), mouseX(0.f), mouseY(0.f)
    {}
};

class Entity {
protected:
    // The origin of our local coordinate system
    // Degrees of rotation from default (vertical applied before horizontal)
    float horRotation, verRotation;

public:
    // Vectors that define the axes of our local coordinate system
    glm::vec3 m_forward, m_right, m_up;

    glm::vec3 m_position; // to do: remove uses of this
    // A readonly reference to position for external use
    const glm::vec3& mcr_position;

    // Various constructors
    Entity();
    Entity(glm::vec3 pos);
    Entity(const Entity &e);
    virtual ~Entity();

    // To be called by MyGL::tick()
    virtual void tick(float dT, InputBundle &input, float time) = 0;

    // Translate along the given vector
    virtual void moveAlongVector(glm::vec3 dir);

    // Rotate to given horizontal and vertical angles (vertical than horizontal)
    virtual void rotateToAngles(float hor, float ver);

    // Translate along one of our local axes
    virtual void moveForwardLocal(float amount);
    virtual void moveRightLocal(float amount);
    virtual void moveUpLocal(float amount);

    // Translate along one of the world axes
    virtual void moveForwardGlobal(float amount);
    virtual void moveRightGlobal(float amount);
    virtual void moveUpGlobal(float amount);

    // Rotate about one of our local axes
    virtual void rotateOnForwardLocal(float degrees);
    virtual void rotateOnRightLocal(float degrees);
    virtual void rotateOnUpLocal(float degrees);

    // Rotate about one of the world axes
    virtual void rotateOnForwardGlobal(float degrees);
    virtual void rotateOnRightGlobal(float degrees);
    virtual void rotateOnUpGlobal(float degrees);
};
