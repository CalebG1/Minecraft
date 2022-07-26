#include "crosshair.h"

Crosshair::Crosshair(OpenGLContext *context) : Drawable(context)
{}

void Crosshair::createVBOs(float xdiff, float ydiff)
{
    GLuint idx[4] = {0, 1, 2, 3};
    glm::vec4 pos[4] = {glm::vec4(0,-ydiff,0.99f,1), glm::vec4(0,ydiff,0.99f,1),
                        glm::vec4(-xdiff,0,0.99f,1), glm::vec4(xdiff,0,0.99f,1)};
    glm::vec4 col[4] = {glm::vec4(1,1,1,1), glm::vec4(1,1,1,1),
                        glm::vec4(1,1,1,1), glm::vec4(1,1,1,1)};

    m_opaqueCount = 4;

    generateIdxOpaque();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxOpaque);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), idx, GL_STATIC_DRAW);
    generateOpaque();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufOpaque);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), pos, GL_STATIC_DRAW);
    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), col, GL_STATIC_DRAW);
}

void Crosshair::createVBOdata() {
    // never used
}

GLenum Crosshair::drawMode()
{
    return GL_LINES;
}
