#ifndef GENESISV_SHADERCOLOR_H
#define GENESISV_SHADERCOLOR_H

#include <string>
#include <GLES3/gl3.h>

/*! Vértice con posición y color (para ejemplos 001-005, líneas o triángulos). */
struct ColoredVertex {
    float x, y, z;
    float r, g, b, a;
};

/*! Shader para geometría con color por vértice (sin textura). uMVP = proyección * vista * modelo. */
class ShaderColor {
public:
    static ShaderColor *load(const std::string &vertexSource,
                             const std::string &fragmentSource);

    ~ShaderColor();

    void activate() const;
    void setMVP(const float *mvp) const;
    /** Dibuja geometría indexada. vertexData: array de ColoredVertex; mode: GL_TRIANGLES o GL_LINES. */
    void draw(const ColoredVertex *vertexData,
              const uint16_t *indexData,
              int indexCount,
              GLenum mode) const;

private:
    ShaderColor(GLuint program, GLint position, GLint color, GLint mvp);
    static GLuint compileShader(GLenum type, const std::string &source);

    GLuint program_;
    GLint position_;
    GLint color_;
    GLint mvp_;
};

#endif
