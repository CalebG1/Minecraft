#pragma once
#include "entity.h"
#include "camera.h"
#include "terrain.h"
#include "chunk.h"
#include "npc.h"
#include <QMap>
#include <QSoundEffect>

class Player : public Entity {
private:
    glm::vec3 m_velocity, m_acceleration;
    Terrain &mcr_terrain;
    bool flying;
    bool jumping;
    bool spaceWaterPressed;
    float alignTimer;

//    QMap<QString,QSoundEffect*> m_soundEf;


    void computePhysics(float dT, Terrain &terrain);
    void processInputs(InputBundle &inputs);

public:
    Camera m_camera;
    // Readonly public reference to our camera
    // for easy access from MyGL
    const Camera& mcr_camera;

    float m_prevMouseX;
    float m_prevMouseY;

    npc ship_npc;

    Player(glm::vec3 pos, Terrain &terrain);
    virtual ~Player() override;

    void setCameraWidthHeight(unsigned int w, unsigned int h); 

    void tick(float dT, InputBundle &input, float time) override;

    void alignVecs();
    void toggleFly();

    BlockType removeBlock(Terrain &terrain);
    bool addBlock(Terrain &terrain, BlockType t);

    void activateRedstone(Terrain &terrain);
    void switchSwitch(Terrain &terrain);

    // Player overrides all of Entity's movement
    // functions so that it transforms its camera
    // by the same amount as it transforms itself.
    void moveAlongVector(glm::vec3 dir) override;
    void rotateToAngles(float hor, float ver) override;
    void moveForwardLocal(float amount) override;
    void moveRightLocal(float amount) override;
    void moveUpLocal(float amount) override;
    void moveForwardGlobal(float amount) override;
    void moveRightGlobal(float amount) override;
    void moveUpGlobal(float amount) override;
    void rotateOnForwardLocal(float degrees) override;
    void rotateOnRightLocal(float degrees) override;
    void rotateOnUpLocal(float degrees) override;
    void rotateOnForwardGlobal(float degrees) override;
    void rotateOnRightGlobal(float degrees) override;
    void rotateOnUpGlobal(float degrees) override;

    // For sending the Player's data to the GUI
    // for display
    QString posAsQString() const;
    QString velAsQString() const;
    QString accAsQString() const;
    QString lookAsQString() const;
};
