#include "mygl.h"
#include <glm_includes.h>
#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QDateTime>
#include <QImage>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this),
      m_crosshair(this),
      m_progLambert(this), m_progFlat(this), m_progInstanced(this), m_progUI(this),
      m_progRedTint(this),
      m_progBlueTint(this),
      m_progNoTint(this),
      m_npc(this),
      m_bomb(this),
      startSound(true),
      m_geomQuad(this),
      m_terrain(this), m_player(glm::vec3(250.f, 175.f, 10.f), m_terrain), m_lastTime(0),
      m_sounds(),
      walkingPlaying(false),
      framebuffer(this, this->width()*this->devicePixelRatio(), this->height()*this->devicePixelRatio(), this->devicePixelRatio()),
      inv(this, 1258, 799)
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);
    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible

    inv.populateInventory();
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
}


void MyGL::moveMouseToCenter() {
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}


void MyGL::resizeGL(int w, int h) {


    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();

    inv.windowHeight = h * 1.f;
    inv.windowWidth = w * 1.f;

//    inv.windowHeight = static_cast<unsigned int>(height());

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);
    m_npc.setViewProjMatrix(viewproj);
    m_bomb.setViewProjMatrix(viewproj);

    // Create the frame buffer
    framebuffer.resize(w, h, this->devicePixelRatio());
    framebuffer.destroy();
    framebuffer.create();
    m_crosshair.destroyVBOdata();
    float xdiff = 20.f / (this->width() * this->devicePixelRatio());
    float ydiff = 20.f / (this->height() * this->devicePixelRatio());
    m_crosshair.createVBOs(xdiff, ydiff);
    printGLErrorLog();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {
    initSoundEffects();
    if(startSound) {
        m_sounds["BackgroundDetected"]->play();
        startSound = false;
    }
//    if (m_player.m_position.y < 128) {
//        m_sounds["FaceDetected"]->play();
//    }
    if (m_player.ship_npc.bomb.soundOn == true) {
        m_sounds["ExplodeDetected"]->play();
    }


    qint64 currTime = QDateTime::currentMSecsSinceEpoch();
//     std::cout << std::to_string(currTime)<< std::endl;
    float dT = (currTime - m_lastTime) / 1000.f;
    m_lastTime = currTime;
    m_progLambert.setTime(currTime);
    m_progBlueTint.setTime(currTime);

    // This is where I want the time of the ship to be moved I think maybe
    m_npc.setTime(currTime);
    m_bomb.setTime(currTime);


//    m_terrain.terrainExpansion(m_player.mcr_position);
    float x = m_player.mcr_position.x;
    float z = m_player.mcr_position.z;

//    m_player.tick(dT, m_inputs);
    m_terrain.multithreadExpansion(m_player.mcr_position, dT);
    m_terrain.delayRedstoneLighting(dT);
    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline

//    float beforeX = m_player.m_position.x;
//    float beforeZ = m_player.m_position.z;
    m_player.tick(dT, m_inputs, currTime%(6283*10));
//    float afterX = m_player.m_position.x;
//    float afterZ = m_player.m_position.z;
//    if (!walkingPlaying && (beforeX != afterX || beforeZ != afterZ)) {
//        m_sounds["WalkingDetected"]->play();
//        m_sounds["WalkingDetected"]->stop();

//        walkingPlaying = true;
//    }
//    if (abs(beforeX - afterX) < 0.5 && abs(beforeZ - afterZ) < 0.5) {
//         m_sounds["WalkingDetected"]->stop();
//         walkingPlaying = false;
//    }


    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data

    // Sound stuff:
    //QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )");
}

//void MyShip::ply_sf(QString &add) {
//    if (!SettingData::sfMut) {
//        QSoundEffect *sf = new QSoundEffect();
//        sf->setSource(QUrl::fromLocalFile(add));
//        sf->setVolume(SettingData::sfVol / 100.0);
//        sf->play();
//    }
//}
void MyGL::initSoundEffects()
{
    QSoundEffect* walking = new QSoundEffect(this);
    QSoundEffect* rajiv = new QSoundEffect(this);
    QSoundEffect* blockbreak = new QSoundEffect(this);
    QSoundEffect* blockplace = new QSoundEffect(this);
    QSoundEffect* bombfall = new QSoundEffect(this);
    QSoundEffect* craft = new QSoundEffect(this);
    QSoundEffect* explode = new QSoundEffect(this);
    QSoundEffect* background = new QSoundEffect(this);
    QSoundEffect* water = new QSoundEffect(this);

//    walking->setSource(QUrl::fromLocalFile(":/sounds/walking.wav"));
//    walking->setLoopCount(1);
    blockbreak->setSource(QUrl::fromLocalFile(":/sounds/blockbreak.wav"));
    blockbreak->setLoopCount(1);
    blockplace->setSource(QUrl::fromLocalFile(":/sounds/blockplace.wav"));
    blockplace->setLoopCount(1);
    bombfall->setSource(QUrl::fromLocalFile(":/sounds/bombfall.wav"));
    bombfall->setLoopCount(1);
    craft->setSource(QUrl::fromLocalFile(":/sounds/craft.wav"));
    craft->setLoopCount(1);
    explode->setSource(QUrl::fromLocalFile(":/sounds/explode.wav"));
    explode->setLoopCount(1);
    background->setSource(QUrl::fromLocalFile(":/sounds/gupta.wav"));
    background->setLoopCount(1);
    water->setSource(QUrl::fromLocalFile(":/sounds/water.wav"));
    water->setLoopCount(1);
    rajiv->setSource(QUrl::fromLocalFile(":/sounds/rajiv.wav"));
    rajiv->setLoopCount(10);

//    m_sounds.insert("WalkingDetected",walking);
    m_sounds.insert("RajivDetected",rajiv);
    m_sounds.insert("BlockbreakDetected",blockbreak);
    m_sounds.insert("BlockplaceDetected",blockplace);
    m_sounds.insert("BombfallDetected",bombfall);
    m_sounds.insert("CraftDetected",craft);
    m_sounds.insert("ExplodeDetected",explode);
    m_sounds.insert("BackgroundDetected",background);
    m_sounds.insert("WaterDetected",water);
}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(m_player.posAsQString());
    emit sig_sendPlayerVel(m_player.velAsQString());
    emit sig_sendPlayerAcc(m_player.accAsQString());
    emit sig_sendPlayerLook(m_player.lookAsQString());
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL() {
    // Clear the screen so that we only see newly drawn images

    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progLambert.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_npc.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_bomb.setViewProjMatrix(m_player.mcr_camera.getViewProj());

    float x = m_player.mcr_position.x;
    float y = m_player.mcr_position.y;
    float z = m_player.mcr_position.z;
    x = static_cast<int>(x) - (static_cast<int>(x) % 16);
    y = static_cast<int>(y);
    z = static_cast<int>(z) - (static_cast<int>(z) % 16);


    framebuffer.bindFrameBuffer();
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    m_progLambert.setSampler(0);

    int range = m_terrain.renderRadius * 64;
    m_terrain.draw(x-range, x+range, z-range, z+range, &m_progLambert);
    m_player.ship_npc.draw(&m_npc, &m_bomb);

    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    framebuffer.bindToTextureSlot(1); // check these
    float x1 = floor(m_player.m_camera.m_position.x);
    float y1 = floor(m_player.m_camera.m_position.y);
    float z1 = floor(m_player.m_camera.m_position.z);
    BlockType b = m_terrain.getBlockAt(x1,y1,z1); // Added 1 here
    BlockType b2 = m_terrain.getBlockAt(x1,y1 - 1,z1); // Added 1 here
    BlockType b3 = m_terrain.getBlockAt(x1,y1 - 2,z1); // Added 1 here

    if(b == WATER && b2 == WATER){
        m_terrain.inWater = true;
        m_progBlueTint.draw(m_geomQuad, 1);
    } else if (b2 == WATER && b3 == WATER){
        m_terrain.inWater = true;
    } else {
        m_terrain.inWater = false;

    }

    if(b == LAVA) {
        m_terrain.inLava = true;
        m_progRedTint.draw(m_geomQuad, 1);
    } else {
        m_terrain.inLava = false;
    }
    if (m_player.ship_npc.bomb.stopMoving == true) {
        m_progRedTint.draw(m_geomQuad, 1);
    } else if ((b != WATER) && (b != LAVA)) {
        m_progNoTint.draw(m_geomQuad, 1);
    }

    glDisable(GL_DEPTH_TEST);
    m_progFlat.setModelMatrix(glm::mat4());
    m_progFlat.setViewProjMatrix(glm::mat4());
    m_progFlat.draw(m_crosshair);

//    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
//    m_progFlat.draw(m_worldAxes);


    m_progUI.setSampler(0);
    m_progUI.drawUI(inv);

    glEnable(GL_DEPTH_TEST);
}

void MyGL::keyPressEvent(QKeyEvent *e) {
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = true;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = true;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = true;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = true;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = true;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = true;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = true;
    }
}


void MyGL::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = false;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = false;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = false;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = false;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = false;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = false;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = false;
    } else if (e->key() == Qt::Key_F) {
        m_player.toggleFly();
    } else if(e->key() == Qt::Key_I){
        inv.invOpen = !inv.invOpen;
        if(inv.invOpen){
            setCursor(Qt::ArrowCursor); // Make the cursor invisible
        } else {
            setCursor(Qt::BlankCursor); // Make the cursor invisible
            if(inv.selectedItem != nullptr && inv.selectedItem->boxType == 3){
                inv.addToInventory(inv.selectedItem->item, inv.selectedItem->count);
                inv.items[inv.outputIdx]->item = EMPTY;
                inv.items[inv.outputIdx]->count = 0;
            }
            inv.selectedItem = nullptr;
        }
        inv.createVBOdata();

    } else if (e->key() == Qt::Key_T) {
        m_player.switchSwitch(m_terrain);
    }
}

void MyGL::wheelEvent(QWheelEvent *ev)
{
    if(ev->angleDelta().y() < 0) {

        if(inv.currentToolbar > 7){
           inv.currentToolbar = 0;
        } else {
           inv.currentToolbar++;
        }
    } else if(ev->angleDelta().y() > 0) {
        if(inv.currentToolbar < 1){
           inv.currentToolbar = 8;
        } else {
           inv.currentToolbar--;
        }
    }
    inv.createVBOdata();

}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    if(!inv.invOpen){
        m_inputs.mouseX = e->position().x();
        m_inputs.mouseY = e->position().y();
    } else {
        inv.mouseMove(e->pos().x(), e->pos().y());
        inv.createVBOdata();
    }

}

void MyGL::mousePressEvent(QMouseEvent *e) {
    if(!inv.invOpen){
        if (e->button() == Qt::LeftButton) {
            BlockType removed = m_player.removeBlock(m_terrain);
            if(removed != EMPTY){
                m_sounds["BlockbreakDetected"]->play();
                inv.addToInventory(removed, 1);
            }
        } else if (e->button() == Qt::RightButton) {
            if(inv.items[inv.currentToolbar]->item != EMPTY){
                if(m_player.addBlock(m_terrain, inv.items[inv.currentToolbar]->item)){

                    m_sounds["BlockplaceDetected"]->play();
                    inv.items[inv.currentToolbar]->decreaseBlock();
                }
            }
        }
        inv.createVBOdata();
    } else {

        if(e->button() == Qt::LeftButton || e->button() == Qt::RightButton){
            inv.click(e->pos().x(), e->pos().y(), e->button() == Qt::LeftButton);
            inv.createVBOdata();
        }


    }
}



void MyGL::initializeGL()
{


    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();


    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of the world axes
    // m_worldAxes.createVBOdata();
    // gets 20 pixels as a fraction of x/y screen length
    float xdiff = 20.f / (this->width() * this->devicePixelRatio());
    float ydiff = 20.f / (this->height() * this->devicePixelRatio());
    m_crosshair.createVBOs(xdiff, ydiff);
    m_geomQuad.createVBOdata();
    inv.createVBOdata();
    // Create the frame buffer
    framebuffer.create();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
//    m_progInstanced.create(":/glsl/instanced.vert.glsl", ":/glsl/lambert.frag.glsl");
    m_progRedTint.create(":/glsl/passthrough.vert.glsl", ":/glsl/redtint.frag.glsl");
    m_progBlueTint.create(":/glsl/passthrough.vert.glsl", ":/glsl/bluetint.frag.glsl");
    m_progNoTint.create(":/glsl/passthrough.vert.glsl", ":/glsl/noOp.frag.glsl");
    m_progUI.create(":/glsl/ui.vert.glsl", ":/glsl/ui.frag.glsl");
//    m_progInstanced.create(":/glsl/instanced.vert.glsl", ":/glsl/lambert.frag.glsl");

//    m_npc.create(":/glsl/lambert.vert.glsl", ":/glsl/npc.frag.glsl");
    m_npc.create(":/glsl/npc.vert.glsl", ":/glsl/npc.frag.glsl");
    m_bomb.create(":/glsl/npc.vert.glsl", ":/glsl/npc.frag.glsl");

    // Create a Vertex Attribute Object

    glGenVertexArrays(1, &vao);

    glGenTextures(1, &textureHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    QImage img(":/textures/minecraft_textures_all.png"); // Load the specified file into a QImage
    img = img.convertToFormat(QImage::Format_ARGB32);
    img = img.mirrored();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(),
                0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, img.bits());
    m_progLambert.setSampler(0);
    m_progUI.setSampler(0);
    printGLErrorLog();

    //Create the instance of the world axes

    // Set a color with which to draw geometry.
    // This will ultimately not be used when you change
    // your program to render Chunks with vertex colors
    // and UV coordinates
    m_progLambert.setGeometryColor(glm::vec4(0,1,0,1));

    m_npc.setGeometryColor(glm::vec4(0.5,0.5,0.5,1)); // Grey color (don't want texturing)
    m_bomb.setGeometryColor(glm::vec4(1.f,0.2,0.2,1));

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);

    m_terrain.CreateInitScene(m_player.mcr_position.x,m_player.mcr_position.z);
    // debug noise functions
//    debugNoiseFunctions();
}


// TODO: Change this so it renders the nine zones of generated
// terrain that surround the player (refer to Terrain::m_generatedTerrain
// for more info)
void MyGL::renderTerrain() {
    // Added this for milestone 2
    framebuffer.bindFrameBuffer();
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // This is done twice ?
     // NEED TO BIND MINECRAFT TEXTURE AND TELL TERRAIN TO DRAW ALL THE CHUNKS
//    mp_modelCurrent->bindBGTexture


    // get player position
    float x = m_player.mcr_position.x;
    float z = m_player.mcr_position.z;

    x = static_cast<int>(x) - (static_cast<int>(x) % 16);
    z = static_cast<int>(z) - (static_cast<int>(z) % 16);

//    // populate neighboring TGZ if not previously created
//    m_terrain.populateNeighboringTGZ(x,z);

//    // find the left lower corner of the TGZ
//    glm::ivec2 tgzPos = m_terrain.getTGZCoord(x,z);

    // Now draw with span as 5x5 TGZ bounding box to player's position
//    int tgzSide = 64;
//    int chunkSize = 16;
    int range = m_terrain.renderRadius * 64;
    m_terrain.draw(x-range, x+range, z-range, z+range, &m_progLambert);
    // Now draw with span as 3x3 TGZ bounding box to player's position
//    int tgzSide = 96;
//    int chunkSize = 32;

    m_progUI.drawUI(inv);

}

void MyGL::performPostProcessRenderPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Bind the texture associated with Frame Buffer (bindToTextureSlot)
    framebuffer.bindToTextureSlot(1); // Not sure what to pass in here
    // Create new shader program (subclasses from whats give
    // add a uv field
    // Use that to call the quad (copy and modify from hw 4)
    m_progRedTint.draw(m_geomQuad, 1); // Have to modify the draw (the second argument should be the texture slot that Im using
}

// Converts Height with assumed range [0,255] to RGB
// Assumes height.min is blue and height.max is red
glm::ivec3 convertHtToRGB(float ht)
{
    glm::ivec3 col;
    glm::vec3 colmax = glm::vec3(0.f,0.f,255.f);
    glm::vec3 colmin = glm::vec3(255.f,0.f,0.f);
    float htmin = 0.f;
    float htmax = 255.f;
    float k = (ht-htmin)/(htmax-htmin);
    col.r = floor(k*colmin.r + (1.f-k)*colmax.r);
    col.g = floor(k*colmin.g + (1.f-k)*colmax.g);
    col.b = floor(k*colmin.b + (1.f-k)*colmax.b);
    return col;
}

void MyGL::debugNoiseFunctions()
{

}




