#include "drawable.h"
#include <glm_includes.h>

Drawable::Drawable(OpenGLContext* context)
    : m_opaqueCount(-1), m_transparentCount(-1), m_bufIdxOpaque(), m_bufOpaque(), m_bufNor(), m_bufCol(), m_bufIdxTransparent(), m_bufTransparent(),
      m_idxGenerated(false), m_opaqueGenerated(false), m_norGenerated(false), m_colGenerated(false),
      m_idxTransparentGenerated(false), m_transparentGenerated(false),
      mp_context(context)
{}

Drawable::~Drawable()
{}


void Drawable::destroyVBOdata()
{
    mp_context->glDeleteBuffers(1, &m_bufIdxOpaque);
    mp_context->glDeleteBuffers(1, &m_bufOpaque);
    mp_context->glDeleteBuffers(1, &m_bufNor);
    mp_context->glDeleteBuffers(1, &m_bufCol);
    mp_context->glDeleteBuffers(1, &m_bufIdxTransparent);
    mp_context->glDeleteBuffers(1, &m_bufTransparent);

    m_transparentGenerated = m_idxTransparentGenerated = m_idxGenerated = m_opaqueGenerated = m_norGenerated = m_colGenerated = false;
    m_opaqueCount = -1;
    m_transparentCount = -1;
}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCount()
{
    return m_opaqueCount;
}

int Drawable::elemCountTransparent()
{
    return m_transparentCount;
}

void Drawable::generateIdxOpaque()
{
    m_idxGenerated = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    mp_context->glGenBuffers(1, &m_bufIdxOpaque);
}

void Drawable::generateOpaque()
{
    m_opaqueGenerated = true;
    // Create a VBO on our GPU and store its handle in bufPos
    mp_context->glGenBuffers(1, &m_bufOpaque);
}

void Drawable::generateNor()
{
    m_norGenerated = true;
    // Create a VBO on our GPU and store its handle in bufNor
    mp_context->glGenBuffers(1, &m_bufNor);
}

void Drawable::generateCol()
{
    m_colGenerated = true;
    // Create a VBO on our GPU and store its handle in bufCol
    mp_context->glGenBuffers(1, &m_bufCol);
}

void Drawable::generateIdxTransparent()
{
    m_idxTransparentGenerated = true;
    // Create a VBO on our GPU and store its handle in m_bufIdxTransparent
    mp_context->glGenBuffers(1, &m_bufIdxTransparent);
}

void Drawable::generateTransparent()
{
    m_transparentGenerated = true;
    // Create a VBO on our GPU and store its handle in m_bufTransparent
    mp_context->glGenBuffers(1, &m_bufTransparent);
}

bool Drawable::bindIdxOpaque()
{
    if(m_idxGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxOpaque);
    }
    return m_idxGenerated;
}

bool Drawable::bindOpaque()
{
    if(m_opaqueGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufOpaque);
    }
    return m_opaqueGenerated;
}

bool Drawable::bindNor()
{
    if(m_norGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
    }
    return m_norGenerated;
}

bool Drawable::bindCol()
{
    if(m_colGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    }
    return m_colGenerated;
}

bool Drawable::bindIdxTransparent()
{
    if(m_idxTransparentGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxTransparent);
    }
    return m_idxTransparentGenerated;
}

bool Drawable::bindTransparent()
{
    if(m_transparentGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufTransparent);
    }
    return m_transparentGenerated;
}

InstancedDrawable::InstancedDrawable(OpenGLContext *context)
    : Drawable(context), m_numInstances(0), m_bufPosOffset(-1), m_offsetGenerated(false)
{}

InstancedDrawable::~InstancedDrawable(){}

int InstancedDrawable::instanceCount() const {
    return m_numInstances;
}

void InstancedDrawable::generateOffsetBuf() {
    m_offsetGenerated = true;
    mp_context->glGenBuffers(1, &m_bufPosOffset);
}

bool InstancedDrawable::bindOffsetBuf() {
    if(m_offsetGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPosOffset);
    }
    return m_offsetGenerated;
}


void InstancedDrawable::clearOffsetBuf() {
    if(m_offsetGenerated) {
        mp_context->glDeleteBuffers(1, &m_bufPosOffset);
        m_offsetGenerated = false;
    }
}
void InstancedDrawable::clearColorBuf() {
    if(m_colGenerated) {
        mp_context->glDeleteBuffers(1, &m_bufCol);
        m_colGenerated = false;
    }
}
