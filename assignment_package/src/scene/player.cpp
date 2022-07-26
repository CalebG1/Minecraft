#include "player.h"
#include <QString>
#include <iostream>

Player::Player(glm::vec3 pos, Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
      flying(true), spaceWaterPressed(false), alignTimer(0.f),
//      m_soundEf(),
      mcr_camera(m_camera),
      ship_npc(pos, terrain.mp_context, 0.5,100.f, terrain)
//      ship_npc(pos, terrain.mp_context, 0.5,5.f, terrain)

{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input, float time)
{
     alignTimer += dT;
    processInputs(input);
    computePhysics(dT, mcr_terrain);
      if (alignTimer > 1.0f) {
        alignVecs();
        alignTimer = 0;
    }
    ship_npc.tick(dT, 0.15, m_position, time);
//    std::cout << std::to_string(m_position.x) + "  " + std::to_string(m_position.z)<< std::endl;
//    std::cout << std::to_string(dT)<< std::endl;
    if ((abs(ship_npc.bomb.m_position.x - this->m_position.x) < 1) &&
        (abs(ship_npc.bomb.m_position.y - this->m_position.y) < 1) &&
        (abs(ship_npc.bomb.m_position.z - this->m_position.z) < 1)) {
        ship_npc.bomb.stopMoving = true;
    } else {
        ship_npc.bomb.stopMoving = false;
    }
}
//    float test1 = ship_npc.bomb.m_position.x;
//    float test2 = ship_npc.bomb.m_position.y;
//    float test3 = ship_npc.bomb.m_position.z;
//    float test4 = this->m_position.x;
//    float test5 = this->m_position.y;
//    float test6 = this->m_position.z;



void Player::alignVecs() { // ensures player vectors don't get screwed up
    m_forward = m_camera.m_forward;
    m_camera.m_up = glm::vec3(0,1,0);
}

// Switches flight mode off/on
// Resets jumping value to ensure it won't be stuck on true
void Player::toggleFly() {
    flying = !flying;
    jumping = false;
}

// Removes block in the center of the screen using player's forward vector
BlockType Player::removeBlock(Terrain &terrain) {
    int checks = 20; // checks per block
    int max_dist = 10;

    // Checks for blocks at a distance in the range [0, max_dist]
    for (int i = 1; i <= max_dist * checks; i++) {
        glm::vec3 cam = m_position + glm::vec3(0, 1.5f, 0) + 0.5f * m_forward;
        glm::vec3 test = cam + (float) i * m_forward / (float) checks;
        if ((terrain.getBlockAt(test) != EMPTY) && (terrain.getBlockAt(test) != BEDROCK)
                && (terrain.getBlockAt(test) != WATER) && (terrain.getBlockAt(test) != LAVA)
                && (terrain.getBlockAt(test) != REDSTONE)) {
            BlockType removedBlock = terrain.getBlockAt(test);

            terrain.placeBlockAt(test, EMPTY);
            return removedBlock;
        }
    }

    return EMPTY;
}

// Adds block in the center of the screen using player's forward vector
bool Player::addBlock(Terrain &terrain, BlockType t) {
//     m_sounds["FaceDetected"]->play();
    int checks = 20; // checks per block
    int max_dist = 10;
    // Smaller vertices so you can fall in 1x1 holes
    glm::vec3 vertices[12] = { glm::vec3(0.3, 0, 0.3),
                               glm::vec3(-0.3, 0, 0.3),
                               glm::vec3(0.3, 0, -0.3),
                               glm::vec3(-0.3, 0, -0.3),
                               glm::vec3(0.3, 0.9, 0.3),
                               glm::vec3(-0.3, 0.9, 0.3),
                               glm::vec3(0.3, 0.9, -0.3),
                               glm::vec3(-0.3, 0.9, -0.3),
                               glm::vec3(0.3, 1.8, 0.3),
                               glm::vec3(-0.3, 1.8, 0.3),
                               glm::vec3(0.3, 1.8, -0.3),
                               glm::vec3(-0.3, 1.8, -0.3) };

    // Checks for blocks at a distance in the range [0, max_dist]
    for (int i = 1; i <= max_dist * checks; i++) {
        glm::vec3 cam = m_position + glm::vec3(0, 1.5f, 0);
        glm::vec3 test = cam + (float) i * m_forward / (float) checks;
        if (terrain.getBlockAt(test) != EMPTY && transparentBlocks.count(terrain.getBlockAt(test)) == 0 && terrain.getBlockAt(test) != WATER && terrain.getBlockAt(test) != LAVA) {

            // Calculate the exact point of intersection (not discrete)
            glm::vec3 block_center = glm::floor(test) + glm::vec3(0.5f);
            float Xint = std::max((abs(cam[0] - block_center[0]) - 0.5f) / abs(m_forward[0]), 0.f);
            float Yint = std::max((abs(cam[1] - block_center[1]) - 0.5f) / abs(m_forward[1]), 0.f);
            float Zint = std::max((abs(cam[2] - block_center[2]) - 0.5f) / abs(m_forward[2]), 0.f);
            float c = std::max(std::max(Xint, Yint), Zint); // multiple of forward vector needed to intersect block
            glm::vec3 intersection = cam + c * m_forward;

            // Determine face which is being looked at
            glm::vec3 to_block = intersection - block_center;
            glm::vec3 face;

            if (std::abs(to_block[0]) >= std::abs(to_block[1]) && std::abs(to_block[0]) >= std::abs(to_block[2])) { // x face
                if (to_block[0] >= 0) {
                    face = glm::vec3(1, 0, 0);
                } else {
                    face = glm::vec3(-1, 0, 0);
                }
            } else if (std::abs(to_block[1]) >= std::abs(to_block[0]) && std::abs(to_block[1]) >= std::abs(to_block[2])) { // y face
                if (to_block[1] >= 0) {
                    face = glm::vec3(0, 1, 0);
                } else {
                    face = glm::vec3(0, -1, 0);
                }
            } else { // z face
                if (to_block[2] >= 0) {
                    face = glm::vec3(0, 0, 1);
                } else {
                    face = glm::vec3(0, 0, -1);
                }
            }

            glm::vec3 new_block = block_center + face;

            // Collision test with player vertices before placing block
            for (const auto& v : vertices) {
                glm::vec3 vertex = m_position + v;
                if (glm::floor(vertex) + glm::vec3(0.5f) == new_block) {
                     return false;
                }
            }
            // ensures you don't place blocks over top of other blocks accidentally
            if (terrain.getBlockAt(new_block) == WATER || transparentBlocks.count(terrain.getBlockAt(new_block)) == 1 || terrain.getBlockAt(new_block) == REDSTONE || terrain.getBlockAt(new_block) == EMPTY) {
                if (terrain.getBlockAt(new_block) != t) {
                      terrain.placeBlockAt(new_block, t);
                      return true;
                }
            }
            return false;
        }
    }
}

void Player::activateRedstone(Terrain &terrain) {
//    int checks = 30; // checks per block
//    int max_dist = 10;

//    // Checks for blocks at a distance in the range [0, max_dist]
//    for (int i = 1; i <= max_dist * checks; i++) {
//        glm::vec3 cam = m_position + glm::vec3(0, 1.5f, 0) + 0.5f * m_forward;
//        glm::vec3 test = cam + (float) i * m_forward / (float) checks;
//        if (terrain.getBlockAt(test) == REDSTONE || terrain.getBlockAt(test) == UNLIT_REDSTONE_TORCH) {
//            glm::vec3 coord = glm::floor(test);
//            terrain.m_redstoneBeingLit.push_back(glm::vec3(coord[0], coord[1], coord[2]));
//            break;
//        } else if (terrain.getBlockAt(test) != EMPTY) {
//            break;
//        }
//    }
}

void Player::switchSwitch(Terrain &terrain) {
    int checks = 30; // checks per block
    int max_dist = 10;

    // Checks for blocks at a distance in the range [0, max_dist]
    for (int i = 1; i <= max_dist * checks; i++) {
        glm::vec3 cam = m_position + glm::vec3(0, 1.5f, 0) + 0.5f * m_forward;
        glm::vec3 test = cam + (float) i * m_forward / (float) checks;
        BlockType type = terrain.getBlockAt(test);
        if (type == SWITCH_OFF) {
            terrain.updateBlockAt(test, SWITCH_ON);
            glm::vec3 coord = glm::floor(test);
            terrain.switchSwitch(coord[0], coord[1], coord[2]);
            break;
        } else if (type == SWITCH_ON) {
            terrain.updateBlockAt(test, SWITCH_OFF);
            glm::vec3 coord = glm::floor(test);
            terrain.switchSwitch(coord[0], coord[1], coord[2]);
            break;
        } else if (type != EMPTY && transparentBlocks.count(type) == 0) {
            break;
        }
    }
}

void Player::processInputs(InputBundle &inputs) {

    // Sets camera rotation to an angle based on mouse coordinates
    rotateToAngles(glm::clamp(-540.f * (inputs.mouseX / m_camera.getWidth() - 0.5f), -270.f, 270.f),
                   glm::clamp(-180.f * (inputs.mouseY / m_camera.getHeight() - 0.5f), -90.f, 90.f));

    // Determines direction of acceleration (multiple inputs will result in acceleration
    // in the direction of the average of the inputs)
    glm::vec3 avec = glm::vec3(0.f);

    if (inputs.wPressed) {
        avec += m_forward;
    }
    if (inputs.aPressed) {
        avec -= m_right;
    }
    if (inputs.sPressed) {
        avec -= m_forward;
    }
    if (inputs.dPressed) {
        avec += m_right;
    }
    if (inputs.qPressed) {
        avec -= m_up;
    }
    if (inputs.ePressed) {
        avec += m_up;
    }
    if (!flying) {
        avec = glm::vec3(avec[0], 0.f, avec[2]);
    }
    if (avec != glm::vec3(0.f)) {
        avec = glm::normalize(avec);
    }

//    if (inputs.spacePressed && !jumping && !flying && !spaceWaterPressed) {
//        avec += glm::vec3(0.f, 20.f, 0.f);

//        jumping = true;
//    }
    // For moving in lava / water
    if ((inputs.spacePressed && mcr_terrain.inLava) || (inputs.spacePressed && mcr_terrain.inWater)) {
        spaceWaterPressed = true;
    } else {
        spaceWaterPressed = false;
    }
    if (inputs.spacePressed && !jumping && !flying && !spaceWaterPressed) {
        avec += glm::vec3(0.f, 20.f, 0.f);

        jumping = true;
    }
    m_acceleration = avec * 2.f;
}

void Player::computePhysics(float dT, Terrain &terrain) {
    float accel_rate = 1.2; // amplifies acceleration (def 1.2)
    float friction = 0.8; // x/z direction slow down factor every tick (lower = faster slow down)
    float air_res = 0.95; // y direction slow down factor every tick (lower = faster slow down)
    float gravity = -1.f; // downwards acceleration added every tick while walking

//    float x = static_cast<int>(m_position.x) - (static_cast<int>(m_position.x) % 16);
//    float y = static_cast<int>(m_position.y);
//    float z = static_cast<int>(m_position.z) - (static_cast<int>(m_position.z) % 16);
//    if (terrain.getBlockAt(x,y,z)) {

//    }
    if (terrain.inLava || terrain.inWater) {
        accel_rate *= 0.67;
        gravity *= 0.67;
    }



    // Smaller vertices so you can fall in 1x1 holes
    glm::vec3 vertices[12] = { glm::vec3(0.3, 0, 0.3),
                               glm::vec3(-0.3, 0, 0.3),
                               glm::vec3(0.3, 0, -0.3),
                               glm::vec3(-0.3, 0, -0.3),
                               glm::vec3(0.3, 0.9, 0.3),
                               glm::vec3(-0.3, 0.9, 0.3),
                               glm::vec3(0.3, 0.9, -0.3),
                               glm::vec3(-0.3, 0.9, -0.3),
                               glm::vec3(0.3, 1.8, 0.3),
                               glm::vec3(-0.3, 1.8, 0.3),
                               glm::vec3(0.3, 1.8, -0.3),
                               glm::vec3(-0.3, 1.8, -0.3) };


    if (!flying) { // adds gravity
        m_acceleration += glm::vec3(0, gravity, 0);
    }

    m_velocity += m_acceleration * accel_rate;

    // Reduces velocity by friction/air resistance constants
    m_velocity[0] *= friction;
    m_velocity[2] *= friction;
    if (flying) {
        m_velocity[1] *= friction;
    } else {
        m_velocity[1] *= air_res;
    }

    // Scales movement based on actual time rather than tick speed
    glm::vec3 movement = m_velocity * dT;

    if (!flying) {
        int checks = 20;

        // Casts ray in velocity vector direction, with fixed # of checks along the vector to ensure
        // collision even if you only nick the corner of a block.
        for (const auto& v : vertices) {
            for (int i = 1; i <= checks+1; i++) {
                for (int dir = 0; dir < 3; dir++) {
                    // Reduces velocity in a particular axis to 0 if there is a block in that direction
                    glm::vec3 axis = glm::vec3(0.f);
                    axis[dir] = movement[dir];
                    glm::vec3 checkPoint = m_position + v + float(i) * axis / float(checks);
                    if ((terrain.getBlockAt(checkPoint) != EMPTY) && (terrain.getBlockAt(checkPoint) != REDSTONE)) {
                        if ((terrain.getBlockAt(checkPoint) != WATER) && (terrain.getBlockAt(checkPoint) != LAVA)) {
                            if (dir == 1 && movement[1] < 0) { // Resets player's ability to jump if you hit a block while moving down
                                jumping = false;
                            }
                            movement[dir] = 0.f;
                            m_velocity[dir] = 0.f;
                        } else {
//                            jumping = true;
//                            movement[dir] = 0.f;
//                            m_velocity[dir] = 0.f;

                        }
                    } else if (dir == 1 && movement[1] < 0) { // ensures you won't be able to jump in mid-air, even if you didn't jump to get there (e.g. walking off cliff)
                        jumping = true;
                    }
                }
            }
        }

        // Extra collision test with final computed velocity
        for (const auto& v : vertices) {
            glm::vec3 moved_vertex = m_position + v + movement;
            if ((terrain.getBlockAt(moved_vertex) != EMPTY)  && (terrain.getBlockAt(moved_vertex) != WATER) && (terrain.getBlockAt(moved_vertex) != LAVA) && (terrain.getBlockAt(moved_vertex) != REDSTONE)) {
                 return;
            }
        }
    }
    if ((terrain.inLava || terrain.inWater) && spaceWaterPressed) {
//        moveAlongVector(glm::vec3(0.f,0.07,0.f));
        moveAlongVector(glm::vec3(movement.x,0.07,movement.z));
    } else {
        moveAlongVector(movement);
    }
}

void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::rotateToAngles(float hor, float ver) {
    Entity::rotateToAngles(hor, ver);
    m_camera.rotateToAngles(hor, ver);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}
