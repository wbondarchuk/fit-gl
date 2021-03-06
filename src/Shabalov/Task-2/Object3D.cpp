#include "Object3D.h"
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

Object3D::Object3D(const std::vector<VertexData> &vertData,
                   const std::vector<GLuint> &indexes, const QImage &texture)
    : m_indexBuffer(QOpenGLBuffer::IndexBuffer), m_texture(nullptr) {
  m_scale = 1.0f;
  init(vertData, indexes, texture);
}

void Object3D::init(const std::vector<VertexData> &vertData,
                    const std::vector<GLuint> &indexes, const QImage &texture) {
  if (m_vertexBuffer.isCreated())
    m_vertexBuffer.destroy();
  if (m_indexBuffer.isCreated())
    m_indexBuffer.destroy();

  m_vertexBuffer.create();
  m_vertexBuffer.bind();
  m_vertexBuffer.allocate(vertData.data(), static_cast<int>(vertData.size()) *
                                               sizeof(VertexData));
  m_vertexBuffer.release();

  m_indexBuffer.create();
  m_indexBuffer.bind();
  m_indexBuffer.allocate(indexes.data(),
                         static_cast<int>(indexes.size()) * sizeof(GLuint));
  m_indexBuffer.release();

  m_texture = std::make_unique<QOpenGLTexture>(texture.mirrored());
  m_texture->setMinificationFilter(QOpenGLTexture::Nearest);
  m_texture->setMagnificationFilter(QOpenGLTexture::Linear);
  m_texture->setWrapMode(QOpenGLTexture::Repeat);
}

void Object3D::draw(QOpenGLShaderProgram *program,
                    QOpenGLFunctions *functions) {

  Q_ASSERT(program && functions);
  if (!m_vertexBuffer.isCreated() || !m_indexBuffer.isCreated())
    return;
  m_texture->bind(0);
  program->setUniformValue("u_texture", 0);
  QMatrix4x4 modelMatrix;
  modelMatrix.setToIdentity();
  modelMatrix.translate(m_translate);
  modelMatrix.rotate(m_rotate);
  modelMatrix.scale(m_scale);
  modelMatrix = m_globalTransform * modelMatrix;

  program->setUniformValue("u_modelMatrix", modelMatrix);
  m_vertexBuffer.bind();

  auto offset = 0;

  auto vertLoc = program->attributeLocation("a_position");
  program->enableAttributeArray(vertLoc);
  program->setAttributeBuffer(vertLoc, GL_FLOAT, offset, 3, sizeof(VertexData));

  offset += sizeof(QVector3D);

  auto texLoc = program->attributeLocation("a_texcoord");
  program->enableAttributeArray(texLoc);
  program->setAttributeBuffer(texLoc, GL_FLOAT, offset, 2, sizeof(VertexData));

  offset += sizeof(QVector2D);

  auto normLoc = program->attributeLocation("a_normal");
  program->enableAttributeArray(normLoc);
  program->setAttributeBuffer(normLoc, GL_FLOAT, offset, 3, sizeof(VertexData));

  m_indexBuffer.bind();

  functions->glDrawElements(GL_LINE_STRIP, m_indexBuffer.size(),
                            GL_UNSIGNED_INT, nullptr);
  // functions->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  m_vertexBuffer.release();
  m_indexBuffer.release();
  m_texture->release();
}

void Object3D::rotate(const QQuaternion &r) { m_rotate = r * m_rotate; }

void Object3D::translate(const QVector3D &t) { m_translate += t; }
