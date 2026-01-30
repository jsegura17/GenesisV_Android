#include "ShaderColor.h"
#include "AndroidOut.h"
#include "Utility.h"
#include <cstddef>
#include <cstring>

static const char *kVertexSource = R"(#version 300 es
in vec3 inPosition;
in vec4 inColor;
out vec4 fragColor;
uniform mat4 uMVP;
void main() {
    gl_Position = uMVP * vec4(inPosition, 1.0);
    fragColor = inColor;
}
)";

static const char *kFragmentSource = R"(#version 300 es
precision mediump float;
in vec4 fragColor;
out vec4 outColor;
void main() {
    outColor = fragColor;
}
)";

ShaderColor *ShaderColor::load(const std::string &vertexSource,
                               const std::string &fragmentSource) {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (!vs) return nullptr;
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (!fs) {
        glDeleteShader(vs);
        return nullptr;
    }
    GLuint program = glCreateProgram();
    if (!program) {
        glDeleteShader(vs);
        glDeleteShader(fs);
        return nullptr;
    }
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        GLint len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        if (len) {
            GLchar *log = new GLchar[len];
            glGetProgramInfoLog(program, len, nullptr, log);
            aout << "ShaderColor link error: " << log << std::endl;
            delete[] log;
        }
        glDeleteProgram(program);
        return nullptr;
    }
    GLint pos = glGetAttribLocation(program, "inPosition");
    GLint col = glGetAttribLocation(program, "inColor");
    GLint mvp = glGetUniformLocation(program, "uMVP");
    if (pos == -1 || col == -1 || mvp == -1) {
        glDeleteProgram(program);
        return nullptr;
    }
    return new ShaderColor(program, pos, col, mvp);
}

ShaderColor::ShaderColor(GLuint program, GLint position, GLint color, GLint mvp)
    : program_(program), position_(position), color_(color), mvp_(mvp) {}

ShaderColor::~ShaderColor() {
    if (program_) {
        glDeleteProgram(program_);
        program_ = 0;
    }
}

void ShaderColor::activate() const {
    glUseProgram(program_);
}

void ShaderColor::setMVP(const float *mvp) const {
    glUniformMatrix4fv(mvp_, 1, GL_FALSE, mvp);
}

void ShaderColor::draw(const ColoredVertex *vertexData,
                       const uint16_t *indexData,
                       int indexCount,
                       GLenum mode) const {
    constexpr size_t stride = sizeof(ColoredVertex);
    const void *colorOffset = reinterpret_cast<const char *>(vertexData) + offsetof(ColoredVertex, r);
    glVertexAttribPointer(position_, 3, GL_FLOAT, GL_FALSE, stride, vertexData);
    glEnableVertexAttribArray(position_);
    glVertexAttribPointer(color_, 4, GL_FLOAT, GL_FALSE, stride, colorOffset);
    glEnableVertexAttribArray(color_);
    glDrawElements(mode, indexCount, GL_UNSIGNED_SHORT, indexData);
    glDisableVertexAttribArray(color_);
    glDisableVertexAttribArray(position_);
}

GLuint ShaderColor::compileShader(GLenum type, const std::string &source) {
    GLuint shader = glCreateShader(type);
    if (!shader) return 0;
    const GLchar *src = source.c_str();
    GLint len = static_cast<GLint>(source.length());
    glShaderSource(shader, 1, &src, &len);
    glCompileShader(shader);
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        GLint len2 = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len2);
        if (len2) {
            GLchar *log = new GLchar[len2];
            glGetShaderInfoLog(shader, len2, nullptr, log);
            aout << "ShaderColor compile error: " << log << std::endl;
            delete[] log;
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}
