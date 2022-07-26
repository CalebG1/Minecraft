#pragma once

#include "drawable.h"
#include <la.h>

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class Crosshair : public Drawable
{
public:
    Crosshair(OpenGLContext* context);
    void createVBOs(float xdiff, float ydiff);
    virtual void createVBOdata() override;
    GLenum drawMode() override;
};
