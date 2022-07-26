#ifndef MYGL_H
#define MYGL_H

#include <QMap>
#include <QSoundEffect>

#include "openglcontext.h"
#include "shaderprogram.h"
#include "framebuffer.h"
#include "scene/worldaxes.h"
#include "scene/camera.h"
#include "scene/terrain.h"
#include "scene/player.h"
#include "scene/quad.h"
#include "scene/inventory.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <smartpointerhelp.h>
#include <postprocessshader.h>
#include "scene/crosshair.h"



class MyGL : public OpenGLContext
{
    Q_OBJECT
private:
    WorldAxes m_worldAxes; // A wireframe representation of the world axes. It is hard-coded to sit centered at (32, 128, 32).
    Crosshair m_crosshair;
    ShaderProgram m_progLambert;// A shader program that uses lambertian reflection
    ShaderProgram m_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    ShaderProgram m_progInstanced;// A shader program that is designed to be compatible with instanced rendering
    ShaderProgram m_progUI;

    PostProcessShader m_progRedTint;
    PostProcessShader m_progBlueTint;
    PostProcessShader m_progNoTint;
    ShaderProgram m_npc;
    ShaderProgram m_bomb;
    bool startSound;

    Quad m_geomQuad;

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    GLuint textureHandle;

    Terrain m_terrain; // All of the Chunks that currently comprise the world.
    Player m_player; // The entity controlled by the user. Contains a camera to display what it sees as well.
    //
    // I should have an independent npc, but now it is a single one stored in Player field
    //

    InputBundle m_inputs; // A collection of variables to be updated in keyPressEvent, mouseMoveEvent, mousePressEvent, etc.

    QTimer m_timer; // Timer linked to tick(). Fires approximately 60 times per second.
    qint64 m_lastTime; // Stores the last time that tick() was called. Used to compute dT.

    QMap<QString,QSoundEffect*> m_sounds;

    bool walkingPlaying;


    void moveMouseToCenter(); // Forces the mouse position to the screen's center. You should call this
                              // from within a mouse move event after reading the mouse movement so that
                              // your mouse stays within the screen bounds and is always read.

    void sendPlayerDataToGUI() const;


public:
    explicit MyGL(QWidget *parent = nullptr);
    ~MyGL();

    // Added framebuffer object
    FrameBuffer framebuffer;
    Inventory inv;

    // Called once when MyGL is initialized.
    // Once this is called, all OpenGL function
    // invocations are valid (before this, they
    // will cause segfaults)
    void initializeGL() override;
    // Called whenever MyGL is resized.
    void resizeGL(int w, int h) override;
    // Called whenever MyGL::update() is called.
    // In the base code, update() is called from tick().
    void paintGL() override;

    // Called from paintGL().
    // Calls Terrain::draw().
    void renderTerrain();
    void performPostProcessRenderPass();

    // debug nose
    void debugNoiseFunctions();

    // Sounds
    void initSoundEffects();

protected:
    // Automatically invoked when the user
    // presses a key on the keyboard
    void keyPressEvent(QKeyEvent *e);
    // Automatically invoked when the user
    // releases a key on the keyboard
    void keyReleaseEvent(QKeyEvent *e);
    // Automatically invoked when the user
    // moves the mouse
    void mouseMoveEvent(QMouseEvent *e);
    // Automatically invoked when the user
    // presses a mouse button
    void mousePressEvent(QMouseEvent *e);

    void wheelEvent(QWheelEvent *ev);

private slots:
    void tick(); // Slot that gets called ~60 times per second by m_timer firing.

signals:
    void sig_sendPlayerPos(QString) const;
    void sig_sendPlayerVel(QString) const;
    void sig_sendPlayerAcc(QString) const;
    void sig_sendPlayerLook(QString) const;
    void sig_sendPlayerChunk(QString) const;
    void sig_sendPlayerTerrainZone(QString) const;
};


#endif // MYGL_H
