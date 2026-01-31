#include "Renderer.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <GLES3/gl3.h>
#include <memory>
#include <vector>
#include <android/imagedecoder.h>
#include <cmath>

#include "AndroidOut.h"
#include "JniBridge.h"
#include "LevelManager.h"
#include "Shader.h"
#include "ShaderColor.h"
#include "TileTextureManager.h"
#include "Utility.h"
#include "TextureAsset.h"

//! executes glGetString and outputs the result to logcat
#define PRINT_GL_STRING(s) {aout << #s": "<< glGetString(s) << std::endl;}

/*!
 * @brief if glGetString returns a space separated list of elements, prints each one on a new line
 *
 * This works by creating an istringstream of the input c-style string. Then that is used to create
 * a vector -- each element of the vector is a new element in the input string. Finally a foreach
 * loop consumes this and outputs it to logcat using @a aout
 */
#define PRINT_GL_STRING_AS_LIST(s) { \
std::istringstream extensionStream((const char *) glGetString(s));\
std::vector<std::string> extensionList(\
        std::istream_iterator<std::string>{extensionStream},\
        std::istream_iterator<std::string>());\
aout << #s":\n";\
for (auto& extension: extensionList) {\
    aout << extension << "\n";\
}\
aout << std::endl;\
}

//! Color for cornflower blue. Can be sent directly to glClearColor
#define CORNFLOWER_BLUE 100 / 255.f, 149 / 255.f, 237 / 255.f, 1

// Vertex shader, you'd typically load this from assets
static const char *vertex = R"vertex(#version 300 es
in vec3 inPosition;
in vec2 inUV;

out vec2 fragUV;

uniform mat4 uProjection;
uniform vec2 uTexOffset;

void main() {
    fragUV = inUV + uTexOffset;
    gl_Position = uProjection * vec4(inPosition, 1.0);
}
)vertex";

// Fragment shader, you'd typically load this from assets
static const char *fragment = R"fragment(#version 300 es
precision mediump float;

in vec2 fragUV;

uniform sampler2D uTexture;

out vec4 outColor;

void main() {
    outColor = texture(uTexture, fragUV);
}
)fragment";

/*!
 * Half the height of the projection matrix. This gives you a renderable area of height 4 ranging
 * from -2 to 2
 */
static constexpr float kProjectionHalfHeight = 2.f;

/*!
 * The near plane distance for the projection matrix. Since this is an orthographic projection
 * matrix, it's convenient to have negative values for sorting (and avoiding z-fighting at 0).
 */
static constexpr float kProjectionNearPlane = -1.f;

/*!
 * The far plane distance for the projection matrix. Since this is an orthographic porjection
 * matrix, it's convenient to have the far plane equidistant from 0 as the near plane.
 */
static constexpr float kProjectionFarPlane = 1.f;

Renderer::~Renderer() {
    if (backButtonTextureId_) {
        glDeleteTextures(1, &backButtonTextureId_);
        backButtonTextureId_ = 0;
    }
    if (display_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context_ != EGL_NO_CONTEXT) {
            eglDestroyContext(display_, context_);
            context_ = EGL_NO_CONTEXT;
        }
        if (surface_ != EGL_NO_SURFACE) {
            eglDestroySurface(display_, surface_);
            surface_ = EGL_NO_SURFACE;
        }
        eglTerminate(display_);
        display_ = EGL_NO_DISPLAY;
    }
}

void Renderer::render() {
    updateRenderArea();

    const float aspect = (height_ > 0) ? float(width_) / height_ : 1.f;
    constexpr float kNear = 0.1f;
    constexpr float kFar = 100.f;

    if (sceneIndex_ == 0 && levelManager_) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader_->activate();
        float projectionMatrix[16] = {0};
        Utility::buildOrthographicMatrix(
                projectionMatrix,
                kProjectionHalfHeight,
                aspect,
                kProjectionNearPlane,
                kProjectionFarPlane);
        shader_->setProjectionMatrix(projectionMatrix);
        shader_->setTexOffset(0.f, 0.f);
        levelManager_->Draw(*shader_);
        drawBackButtonOverlay();
        auto swapResult = eglSwapBuffers(display_, surface_);
        assert(swapResult == EGL_TRUE);
        return;
    }

    if (exampleIndex_ >= 1 && exampleIndex_ <= 15) {
        angle_ += 0.5f;
        angleX_ += 0.5f;
        angleY_ += 0.4f;
        angleZ_ += 0.3f;
        cubeAngleX_ += 0.5f;
        cubeAngleY_ += 0.4f;
        pyramidAngleX_ += 0.4f;
        pyramidAngleY_ += 0.3f;
        textureOffset_ += 0.002f;
        if (textureOffset_ > 1.f) textureOffset_ -= 1.f;

        float P[16], T[16], Rx[16], Ry[16], Rz[16], M[16], MVP[16];
        Utility::buildPerspectiveMatrix(P, 45.f * 3.14159265f / 180.f, aspect, kNear, kFar);
        Utility::buildTranslationMatrix(T, 0.f, 0.f, -6.f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (exampleIndex_ == 5) {
            // 005: cubo a la izquierda, pirámide a la derecha
            float Tcube[16], Tpyramid[16], Rcx[16], Rcy[16], Rpx[16], Rpy[16];
            Utility::buildTranslationMatrix(Tcube, -1.5f, 0.f, 0.f);
            Utility::buildTranslationMatrix(Tpyramid, 1.5f, 0.f, 0.f);
            Utility::buildRotationX(Rcx, cubeAngleX_);
            Utility::buildRotationY(Rcy, cubeAngleY_);
            Utility::buildRotationX(Rpx, pyramidAngleX_);
            Utility::buildRotationY(Rpy, pyramidAngleY_);
            Utility::matrixMultiply(M, T, Tcube);
            Utility::matrixMultiply(M, M, Rcy);
            Utility::matrixMultiply(M, M, Rcx);
            Utility::matrixMultiply(MVP, P, M);
            shaderColor_->activate();
            shaderColor_->setMVP(MVP);
            if (!coloredVertices_.empty() && !coloredIndices_.empty())
                shaderColor_->draw(coloredVertices_.data(), coloredIndices_.data(),
                                   static_cast<int>(coloredIndices_.size()), coloredMode_);
            Utility::matrixMultiply(M, T, Tpyramid);
            Utility::matrixMultiply(M, M, Rpy);
            Utility::matrixMultiply(M, M, Rpx);
            Utility::matrixMultiply(MVP, P, M);
            shaderColor_->setMVP(MVP);
            if (!coloredVertices2_.empty() && !coloredIndices2_.empty())
                shaderColor_->draw(coloredVertices2_.data(), coloredIndices2_.data(),
                                   static_cast<int>(coloredIndices2_.size()), coloredMode_);
        } else if (exampleIndex_ == 6) {
            // 006: quad con textura, perspectiva y rotación
            Utility::buildRotationX(Rx, angleX_ * 0.5f);
            Utility::buildRotationY(Ry, angle_);
            Utility::matrixMultiply(M, T, Ry);
            Utility::matrixMultiply(M, M, Rx);
            Utility::matrixMultiply(MVP, P, M);
            shader_->activate();
            shader_->setProjectionMatrix(MVP);
            if (!models_.empty())
                shader_->drawModel(models_[0]);
        } else if (exampleIndex_ >= 1 && exampleIndex_ <= 4 && !coloredVertices_.empty()) {
            // 001-004: geometría coloreada
            if (exampleIndex_ == 1 || exampleIndex_ == 2) {
                Utility::buildRotationX(Rx, angle_ * 0.5f);
                Utility::buildRotationY(Ry, angle_);
                Utility::matrixMultiply(M, T, Ry);
                Utility::matrixMultiply(M, M, Rx);
            } else {
                Utility::buildRotationX(Rx, angleX_);
                Utility::buildRotationY(Ry, angleY_);
                Utility::buildRotationZ(Rz, angleZ_);
                Utility::matrixMultiply(M, T, Ry);
                Utility::matrixMultiply(M, M, Rx);
                Utility::matrixMultiply(M, M, Rz);
            }
            Utility::matrixMultiply(MVP, P, M);
            shaderColor_->activate();
            shaderColor_->setMVP(MVP);
            shaderColor_->draw(coloredVertices_.data(), coloredIndices_.data(),
                               static_cast<int>(coloredIndices_.size()), coloredMode_);
        } else if (exampleIndex_ == 7 || exampleIndex_ == 13) {
            Utility::buildRotationX(Rx, angleX_);
            Utility::buildRotationY(Ry, angleY_);
            Utility::matrixMultiply(M, T, Ry);
            Utility::matrixMultiply(M, M, Rx);
            Utility::matrixMultiply(MVP, P, M);
            shader_->activate();
            shader_->setTexOffset(0.f, 0.f);
            shader_->setProjectionMatrix(MVP);
            if (!models_.empty()) shader_->drawModel(models_[0]);
        } else if (exampleIndex_ == 8) {
            Utility::buildRotationX(Rx, angleX_);
            Utility::buildRotationY(Ry, angleY_);
            Utility::matrixMultiply(M, T, Ry);
            Utility::matrixMultiply(M, M, Rx);
            Utility::matrixMultiply(MVP, P, M);
            shader_->activate();
            shader_->setTexOffset(0.f, 0.f);
            shader_->setProjectionMatrix(MVP);
            for (const auto &model : models_) shader_->drawModel(model);
        } else if (exampleIndex_ == 9) {
            Utility::buildRotationY(Ry, angle_);
            Utility::matrixMultiply(M, T, Ry);
            Utility::matrixMultiply(MVP, P, M);
            shader_->activate();
            shader_->setTexOffset(textureOffset_, textureOffset_);
            shader_->setProjectionMatrix(MVP);
            if (!models_.empty()) shader_->drawModel(models_[0]);
        } else if (exampleIndex_ == 10) {
            Utility::buildRotationY(Ry, angle_);
            Utility::matrixMultiply(M, T, Ry);
            Utility::matrixMultiply(MVP, P, M);
            shader_->activate();
            shader_->setTexOffset(0.f, 0.f);
            shader_->setProjectionMatrix(MVP);
            if (!models_.empty()) shader_->drawModel(models_[0]);
        } else if (exampleIndex_ == 11) {
            Utility::buildRotationX(Rx, angleX_);
            Utility::buildRotationY(Ry, angleY_);
            Utility::matrixMultiply(M, T, Ry);
            Utility::matrixMultiply(M, M, Rx);
            Utility::matrixMultiply(MVP, P, M);
            shader_->activate();
            shader_->setTexOffset(0.f, 0.f);
            shader_->setProjectionMatrix(MVP);
            for (const auto &model : models_) shader_->drawModel(model);
        } else if (exampleIndex_ == 12) {
            float Tc[16], Tp[16], Rcx[16], Rcy[16], Rpx[16], Rpy[16];
            Utility::buildTranslationMatrix(Tc, -1.5f, 0.f, 0.f);
            Utility::buildTranslationMatrix(Tp, 1.5f, 0.f, 0.f);
            Utility::buildRotationX(Rcx, cubeAngleX_);
            Utility::buildRotationY(Rcy, cubeAngleY_);
            Utility::buildRotationX(Rpx, pyramidAngleX_);
            Utility::buildRotationY(Rpy, pyramidAngleY_);
            Utility::matrixMultiply(M, T, Tc);
            Utility::matrixMultiply(M, M, Rcy);
            Utility::matrixMultiply(M, M, Rcx);
            Utility::matrixMultiply(MVP, P, M);
            shader_->activate();
            shader_->setTexOffset(0.f, 0.f);
            shader_->setProjectionMatrix(MVP);
            if (models_.size() >= 1) shader_->drawModel(models_[0]);
            Utility::matrixMultiply(M, T, Tp);
            Utility::matrixMultiply(M, M, Rpy);
            Utility::matrixMultiply(M, M, Rpx);
            Utility::matrixMultiply(MVP, P, M);
            shader_->setProjectionMatrix(MVP);
            if (models_.size() >= 2) shader_->drawModel(models_[1]);
        } else if (exampleIndex_ == 14) {
            float Tgr[16], Tcb[16], Tt1[16], Tt2[16];
            Utility::buildTranslationMatrix(Tgr, 0.f, -2.f, 0.f);
            Utility::matrixMultiply(M, T, Tgr);
            Utility::matrixMultiply(MVP, P, M);
            shader_->activate();
            shader_->setTexOffset(0.f, 0.f);
            shader_->setProjectionMatrix(MVP);
            if (models_.size() >= 1) shader_->drawModel(models_[0]);
            Utility::buildTranslationMatrix(Tcb, -1.5f, 0.f, 0.f);
            Utility::buildRotationY(Ry, angle_ * 2.f);
            Utility::matrixMultiply(M, T, Tcb);
            Utility::matrixMultiply(M, M, Ry);
            Utility::matrixMultiply(MVP, P, M);
            shader_->setProjectionMatrix(MVP);
            if (models_.size() >= 2) shader_->drawModel(models_[1]);
            Utility::buildTranslationMatrix(Tt1, 1.5f, 0.f, 0.f);
            Utility::buildRotationY(Ry, -angle_);
            Utility::matrixMultiply(M, T, Tt1);
            Utility::matrixMultiply(M, M, Ry);
            Utility::matrixMultiply(MVP, P, M);
            shader_->setProjectionMatrix(MVP);
            if (models_.size() >= 3) shader_->drawModel(models_[2]);
            Utility::buildTranslationMatrix(Tt2, 0.f, 1.5f, 0.f);
            Utility::buildRotationY(Ry, angle_ * 1.5f);
            Utility::matrixMultiply(M, T, Tt2);
            Utility::matrixMultiply(M, M, Ry);
            Utility::matrixMultiply(MVP, P, M);
            shader_->setProjectionMatrix(MVP);
            if (models_.size() >= 4) shader_->drawModel(models_[3]);
        } else if (exampleIndex_ == 15) {
            Utility::buildRotationX(Rx, angleX_);
            Utility::buildRotationY(Ry, angleY_);
            Utility::matrixMultiply(M, T, Ry);
            Utility::matrixMultiply(M, M, Rx);
            Utility::matrixMultiply(MVP, P, M);
            shader_->activate();
            shader_->setTexOffset(0.f, 0.f);
            shader_->setProjectionMatrix(MVP);
            for (const auto &model : models_) shader_->drawModel(model);
        }
    } else {
        // Base y resto: orto 2D, quad con textura
        if (shaderNeedsNewProjectionMatrix_) {
            float projectionMatrix[16] = {0};
            Utility::buildOrthographicMatrix(
                    projectionMatrix,
                    kProjectionHalfHeight,
                    aspect,
                    kProjectionNearPlane,
                    kProjectionFarPlane);
            shader_->setProjectionMatrix(projectionMatrix);
            shaderNeedsNewProjectionMatrix_ = false;
        }
        glClear(GL_COLOR_BUFFER_BIT);
        if (!models_.empty()) {
            shader_->activate();
            for (const auto &model : models_)
                shader_->drawModel(model);
        }
    }

    if (exampleIndex_ >= 1)
        drawBackButtonOverlay();

    auto swapResult = eglSwapBuffers(display_, surface_);
    assert(swapResult == EGL_TRUE);
}

void Renderer::initRenderer() {
    // Choose your render attributes
    constexpr EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    // The default display is probably what you want on Android
    auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);

    // figure out how many configs there are
    EGLint numConfigs;
    eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

    // get the list of configurations
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);

    // Find a config we like.
    // Could likely just grab the first if we don't care about anything else in the config.
    // Otherwise hook in your own heuristic
    auto config = *std::find_if(
            supportedConfigs.get(),
            supportedConfigs.get() + numConfigs,
            [&display](const EGLConfig &config) {
                EGLint red, green, blue, depth;
                if (eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red)
                    && eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green)
                    && eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue)
                    && eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depth)) {

                    aout << "Found config with " << red << ", " << green << ", " << blue << ", "
                         << depth << std::endl;
                    return red == 8 && green == 8 && blue == 8 && depth == 24;
                }
                return false;
            });

    aout << "Found " << numConfigs << " configs" << std::endl;
    aout << "Chose " << config << std::endl;

    // create the proper window surface
    EGLint format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    EGLSurface surface = eglCreateWindowSurface(display, config, app_->window, nullptr);

    // Create a GLES 3 context
    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    EGLContext context = eglCreateContext(display, config, nullptr, contextAttribs);

    // get some window metrics
    auto madeCurrent = eglMakeCurrent(display, surface, surface, context);
    assert(madeCurrent);

    display_ = display;
    surface_ = surface;
    context_ = context;

    // make width and height invalid so it gets updated the first frame in @a updateRenderArea()
    width_ = -1;
    height_ = -1;

    PRINT_GL_STRING(GL_VENDOR);
    PRINT_GL_STRING(GL_RENDERER);
    PRINT_GL_STRING(GL_VERSION);
    PRINT_GL_STRING_AS_LIST(GL_EXTENSIONS);

    shader_ = std::unique_ptr<Shader>(
            Shader::loadShader(vertex, fragment, "inPosition", "inUV", "uProjection"));
    assert(shader_);

    static const char *colorVert = R"(#version 300 es
in vec3 inPosition;
in vec4 inColor;
out vec4 fragColor;
uniform mat4 uMVP;
void main() { gl_Position = uMVP * vec4(inPosition, 1.0); fragColor = inColor; }
)";
    static const char *colorFrag = R"(#version 300 es
precision mediump float;
in vec4 fragColor;
out vec4 outColor;
void main() { outColor = fragColor; }
)";
    shaderColor_ = std::unique_ptr<ShaderColor>(
            ShaderColor::load(std::string(colorVert), std::string(colorFrag)));
    assert(shaderColor_);

    shader_->activate();

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    createModels();
}

void Renderer::updateRenderArea() {
    EGLint width;
    eglQuerySurface(display_, surface_, EGL_WIDTH, &width);

    EGLint height;
    eglQuerySurface(display_, surface_, EGL_HEIGHT, &height);

    if (width != width_ || height != height_) {
        width_ = width;
        height_ = height;
        glViewport(0, 0, width, height);
        shaderNeedsNewProjectionMatrix_ = true;
    }
}

/**
 * @brief Create geometry for the selected example (001-006) or default textured quad.
 */
void Renderer::createModels() {
    aout << "Renderer: example index " << exampleIndex_ << ", scene index " << sceneIndex_
         << std::endl;

    auto assetManager = app_->activity->assetManager;

    if (sceneIndex_ == 0) {
        tileTextureManager_ = std::make_unique<TileTextureManager>(assetManager);
        levelManager_ = std::make_unique<LevelManager>(
                [this](int tileId) { return tileTextureManager_->getTextureId(tileId); });
        levelManager_->LoadLevel({{1, 2, 7, 2, 3}, {0, 5, 5, 5, 0}});
        return;
    }

    switch (exampleIndex_) {
        case 1: { // 001: Triángulo rotando (R, G, B)
            coloredVertices_ = {
                    {0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 1.f},
                    {-1.f, -1.f, 0.f, 0.f, 1.f, 0.f, 1.f},
                    {1.f, -1.f, 0.f, 0.f, 0.f, 1.f, 1.f}
            };
            coloredIndices_ = {0, 1, 2};
            coloredMode_ = GL_TRIANGLES;
            break;
        }
        case 2: { // 002: Cuadrado con colores (R, G, B, Y)
            coloredVertices_ = {
                    {-1.f, 1.f, 0.f, 1.f, 0.f, 0.f, 1.f},
                    {1.f, 1.f, 0.f, 0.f, 1.f, 0.f, 1.f},
                    {1.f, -1.f, 0.f, 0.f, 0.f, 1.f, 1.f},
                    {-1.f, -1.f, 0.f, 1.f, 1.f, 0.f, 1.f}
            };
            coloredIndices_ = {0, 1, 2, 0, 2, 3};
            coloredMode_ = GL_TRIANGLES;
            break;
        }
        case 3: { // 003: Cubo en alambre (wireframe)
            // 8 vértices del cubo ±1
            float s = 1.f;
            coloredVertices_ = {
                    {-s, -s, s, 1.f, 1.f, 1.f, 1.f},
                    {s, -s, s, 1.f, 1.f, 1.f, 1.f},
                    {s, s, s, 1.f, 1.f, 1.f, 1.f},
                    {-s, s, s, 1.f, 1.f, 1.f, 1.f},
                    {-s, -s, -s, 1.f, 1.f, 1.f, 1.f},
                    {s, -s, -s, 1.f, 1.f, 1.f, 1.f},
                    {s, s, -s, 1.f, 1.f, 1.f, 1.f},
                    {-s, s, -s, 1.f, 1.f, 1.f, 1.f}
            };
            coloredIndices_ = {
                    0, 1, 1, 2, 2, 3, 3, 0,
                    4, 5, 5, 6, 6, 7, 7, 4,
                    0, 4, 1, 5, 2, 6, 3, 7
            };
            coloredMode_ = GL_LINES;
            break;
        }
        case 4: { // 004: Cubo sólido con colores por cara (R, G, B, Y, Magenta, Cyan)
            float s = 1.f;
            coloredVertices_ = {
                    {-s, -s, s, 1.f, 0.f, 0.f, 1.f},
                    {s, -s, s, 1.f, 0.f, 0.f, 1.f},
                    {s, s, s, 1.f, 0.f, 0.f, 1.f},
                    {-s, s, s, 1.f, 0.f, 0.f, 1.f},
                    {-s, -s, -s, 0.f, 1.f, 0.f, 1.f},
                    {-s, s, -s, 0.f, 1.f, 0.f, 1.f},
                    {s, s, -s, 0.f, 1.f, 0.f, 1.f},
                    {s, -s, -s, 0.f, 1.f, 0.f, 1.f},
                    {-s, s, -s, 0.f, 0.f, 1.f, 1.f},
                    {-s, s, s, 0.f, 0.f, 1.f, 1.f},
                    {s, s, s, 0.f, 0.f, 1.f, 1.f},
                    {s, s, -s, 0.f, 0.f, 1.f, 1.f},
                    {-s, -s, -s, 1.f, 1.f, 0.f, 1.f},
                    {s, -s, -s, 1.f, 1.f, 0.f, 1.f},
                    {s, -s, s, 1.f, 1.f, 0.f, 1.f},
                    {-s, -s, s, 1.f, 1.f, 0.f, 1.f},
                    {s, -s, -s, 1.f, 0.f, 1.f, 1.f},
                    {s, s, -s, 1.f, 0.f, 1.f, 1.f},
                    {s, s, s, 1.f, 0.f, 1.f, 1.f},
                    {s, -s, s, 1.f, 0.f, 1.f, 1.f},
                    {-s, -s, -s, 0.f, 1.f, 1.f, 1.f},
                    {-s, -s, s, 0.f, 1.f, 1.f, 1.f},
                    {-s, s, s, 0.f, 1.f, 1.f, 1.f},
                    {-s, s, -s, 0.f, 1.f, 1.f, 1.f}
            };
            coloredIndices_ = {
                    0, 1, 2, 0, 2, 3,
                    4, 5, 6, 4, 6, 7,
                    8, 9, 10, 8, 10, 11,
                    12, 13, 14, 12, 14, 15,
                    16, 17, 18, 16, 18, 19,
                    20, 21, 22, 20, 22, 23
            };
            coloredMode_ = GL_TRIANGLES;
            break;
        }
        case 5: { // 005: Cubo y pirámide rotando a distintos lados
            float h = 0.5f;
            coloredVertices_ = {
                    {-h, -h, h, 1.f, 0.f, 0.f, 1.f},
                    {h, -h, h, 1.f, 0.f, 0.f, 1.f},
                    {h, h, h, 1.f, 0.f, 0.f, 1.f},
                    {-h, h, h, 1.f, 0.f, 0.f, 1.f},
                    {-h, -h, -h, 0.f, 1.f, 0.f, 1.f},
                    {-h, h, -h, 0.f, 1.f, 0.f, 1.f},
                    {h, h, -h, 0.f, 1.f, 0.f, 1.f},
                    {h, -h, -h, 0.f, 1.f, 0.f, 1.f},
                    {-h, h, -h, 0.f, 0.f, 1.f, 1.f},
                    {-h, h, h, 0.f, 0.f, 1.f, 1.f},
                    {h, h, h, 0.f, 0.f, 1.f, 1.f},
                    {h, h, -h, 0.f, 0.f, 1.f, 1.f},
                    {-h, -h, -h, 1.f, 1.f, 0.f, 1.f},
                    {h, -h, -h, 1.f, 1.f, 0.f, 1.f},
                    {h, -h, h, 1.f, 1.f, 0.f, 1.f},
                    {-h, -h, h, 1.f, 1.f, 0.f, 1.f},
                    {h, -h, -h, 1.f, 0.f, 1.f, 1.f},
                    {h, h, -h, 1.f, 0.f, 1.f, 1.f},
                    {h, h, h, 1.f, 0.f, 1.f, 1.f},
                    {h, -h, h, 1.f, 0.f, 1.f, 1.f},
                    {-h, -h, -h, 0.f, 1.f, 1.f, 1.f},
                    {-h, -h, h, 0.f, 1.f, 1.f, 1.f},
                    {-h, h, h, 0.f, 1.f, 1.f, 1.f},
                    {-h, h, -h, 0.f, 1.f, 1.f, 1.f}
            };
            coloredIndices_ = {
                    0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7,
                    8, 9, 10, 8, 10, 11, 12, 13, 14, 12, 14, 15,
                    16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23
            };
            // Pirámide: 4 caras triangulares + base cuadrada (5 vértices únicos, repetidos con color)
            coloredVertices2_ = {
                    {0.f, h, 0.f, 1.f, 0.f, 0.f, 1.f},
                    {-h, -h, h, 0.f, 1.f, 0.f, 1.f},
                    {h, -h, h, 0.f, 0.f, 1.f, 1.f},
                    {0.f, h, 0.f, 1.f, 1.f, 0.f, 1.f},
                    {h, -h, -h, 0.f, 1.f, 1.f, 1.f},
                    {-h, -h, -h, 1.f, 0.f, 1.f, 1.f},
                    {0.f, h, 0.f, 1.f, 0.f, 0.f, 1.f},
                    {h, -h, h, 0.f, 0.f, 1.f, 1.f},
                    {h, -h, -h, 1.f, 0.f, 1.f, 1.f},
                    {0.f, h, 0.f, 0.f, 1.f, 0.f, 1.f},
                    {-h, -h, -h, 0.f, 1.f, 1.f, 1.f},
                    {-h, -h, h, 1.f, 0.f, 0.f, 1.f},
                    {-h, -h, h, 1.f, 1.f, 1.f, 1.f},
                    {h, -h, h, 1.f, 1.f, 1.f, 1.f},
                    {h, -h, -h, 1.f, 1.f, 1.f, 1.f},
                    {-h, -h, -h, 1.f, 1.f, 1.f, 1.f}
            };
            coloredIndices2_ = {
                    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
                    12, 13, 14, 12, 14, 15
            };
            coloredMode_ = GL_TRIANGLES;
            break;
        }
        case 6: { // 006: Cuadrado con textura (madera)
            std::vector<Vertex> vertices = {
                    Vertex(Vector3{-1.f, -1.f, 0.f}, Vector2{0.f, 0.f}),
                    Vertex(Vector3{1.f, -1.f, 0.f}, Vector2{1.f, 0.f}),
                    Vertex(Vector3{1.f, 1.f, 0.f}, Vector2{1.f, 1.f}),
                    Vertex(Vector3{-1.f, 1.f, 0.f}, Vector2{0.f, 1.f})
            };
            std::vector<Index> indices = {0, 1, 2, 0, 2, 3};
            auto spTex = TextureAsset::loadAsset(assetManager, "wood.jpg");
            models_.emplace_back(vertices, indices, spTex);
            break;
        }
        case 7: { // 007: Cubo con textura wood
            buildTexturedCube(assetManager, 1.f, "wood.jpg", true);
            break;
        }
        case 8: { // 008: Cubo wood en 5 caras, grass en top
            buildCubeMultiTexture(assetManager);
            break;
        }
        case 9: { // 009: Quad con textura animada (UV offset)
            std::vector<Vertex> v = {
                    Vertex(Vector3{-1.f, -1.f, 0.f}, Vector2{0.f, 0.f}),
                    Vertex(Vector3{1.f, -1.f, 0.f}, Vector2{2.f, 0.f}),
                    Vertex(Vector3{1.f, 1.f, 0.f}, Vector2{2.f, 2.f}),
                    Vertex(Vector3{-1.f, 1.f, 0.f}, Vector2{0.f, 2.f})
            };
            std::vector<Index> idx = {0, 1, 2, 0, 2, 3};
            models_.emplace_back(v, idx, TextureAsset::loadAsset(assetManager, "wood.jpg"));
            break;
        }
        case 10: { // 010: Quad con textura (filtro LINEAR por defecto)
            std::vector<Vertex> v = {
                    Vertex(Vector3{-1.f, -1.f, 0.f}, Vector2{0.f, 0.f}),
                    Vertex(Vector3{1.f, -1.f, 0.f}, Vector2{3.f, 0.f}),
                    Vertex(Vector3{1.f, 1.f, 0.f}, Vector2{3.f, 3.f}),
                    Vertex(Vector3{-1.f, 1.f, 0.f}, Vector2{0.f, 3.f})
            };
            std::vector<Index> idx = {0, 1, 2, 0, 2, 3};
            models_.emplace_back(v, idx, TextureAsset::loadAsset(assetManager, "wood.jpg"));
            break;
        }
        case 11: { // 011: 4 tiles desde set-001.jpg (grid 4x4)
            buildTileQuads(assetManager);
            break;
        }
        case 12: { // 012: Cubo wood + pirámide grass
            buildTexturedCube(assetManager, 0.5f, "wood.jpg", false);
            buildTexturedPyramid(assetManager, "grass.jpg");
            break;
        }
        case 13: { // 013: Cubo con textura (iluminación simulada = mismo que 007)
            buildTexturedCube(assetManager, 1.f, "wood.jpg", true);
            break;
        }
        case 14: { // 014: Escena: suelo grass, cubo wood, tiles set-001
            buildScene014(assetManager);
            break;
        }
        case 15: { // 015: Cubo wood + tile set-001
            buildTexturedCube(assetManager, 1.f, "wood.jpg", true);
            buildTileQuad015(assetManager);
            break;
        }
        default: { // Base y resto: quad con textura android_robot
            std::vector<Vertex> vertices = {
                    Vertex(Vector3{1, 1, 0}, Vector2{0, 0}),
                    Vertex(Vector3{-1, 1, 0}, Vector2{1, 0}),
                    Vertex(Vector3{-1, -1, 0}, Vector2{1, 1}),
                    Vertex(Vector3{1, -1, 0}, Vector2{0, 1})
            };
            std::vector<Index> indices = {0, 1, 2, 0, 2, 3};
            auto spAndroidRobotTexture = TextureAsset::loadAsset(assetManager, "android_robot.png");
            models_.emplace_back(vertices, indices, spAndroidRobotTexture);
            break;
        }
    }
}

void Renderer::buildTexturedCube(AAssetManager *assetManager, float s, const char *texturePath, bool) {
    std::vector<Vertex> v = {
            Vertex(Vector3{-s, -s, s}, Vector2{0.f, 0.f}),
            Vertex(Vector3{s, -s, s}, Vector2{1.f, 0.f}),
            Vertex(Vector3{s, s, s}, Vector2{1.f, 1.f}),
            Vertex(Vector3{-s, s, s}, Vector2{0.f, 1.f}),
            Vertex(Vector3{-s, -s, -s}, Vector2{1.f, 0.f}),
            Vertex(Vector3{-s, s, -s}, Vector2{1.f, 1.f}),
            Vertex(Vector3{s, s, -s}, Vector2{0.f, 1.f}),
            Vertex(Vector3{s, -s, -s}, Vector2{0.f, 0.f}),
            Vertex(Vector3{-s, s, -s}, Vector2{0.f, 1.f}),
            Vertex(Vector3{-s, s, s}, Vector2{0.f, 0.f}),
            Vertex(Vector3{s, s, s}, Vector2{1.f, 0.f}),
            Vertex(Vector3{s, s, -s}, Vector2{1.f, 1.f}),
            Vertex(Vector3{-s, -s, -s}, Vector2{1.f, 1.f}),
            Vertex(Vector3{s, -s, -s}, Vector2{0.f, 1.f}),
            Vertex(Vector3{s, -s, s}, Vector2{0.f, 0.f}),
            Vertex(Vector3{-s, -s, s}, Vector2{1.f, 0.f}),
            Vertex(Vector3{s, -s, -s}, Vector2{1.f, 0.f}),
            Vertex(Vector3{s, s, -s}, Vector2{1.f, 1.f}),
            Vertex(Vector3{s, s, s}, Vector2{0.f, 1.f}),
            Vertex(Vector3{s, -s, s}, Vector2{0.f, 0.f}),
            Vertex(Vector3{-s, -s, -s}, Vector2{0.f, 0.f}),
            Vertex(Vector3{-s, -s, s}, Vector2{1.f, 0.f}),
            Vertex(Vector3{-s, s, s}, Vector2{1.f, 1.f}),
            Vertex(Vector3{-s, s, -s}, Vector2{0.f, 1.f})
    };
    std::vector<Index> idx;
    for (int i = 0; i < 6; i++) {
        int b = i * 4;
        idx.insert(idx.end(), {uint16_t(b), uint16_t(b + 1), uint16_t(b + 2), uint16_t(b), uint16_t(b + 2), uint16_t(b + 3)});
    }
    auto sp = TextureAsset::loadAsset(assetManager, texturePath);
    models_.emplace_back(std::move(v), std::move(idx), sp);
}

void Renderer::buildCubeMultiTexture(AAssetManager *assetManager) {
    float s = 1.f;
    auto spWood = TextureAsset::loadAsset(assetManager, "wood.jpg");
    auto spGrass = TextureAsset::loadAsset(assetManager, "grass.jpg");
    auto addFace = [&](const std::vector<Vertex> &face, const std::shared_ptr<TextureAsset> &tex) {
        std::vector<Index> i = {0, 1, 2, 0, 2, 3};
        models_.emplace_back(face, i, tex);
    };
    addFace({
                 Vertex(Vector3{-s, -s, s}, Vector2{0.f, 0.f}),
                 Vertex(Vector3{s, -s, s}, Vector2{1.f, 0.f}),
                 Vertex(Vector3{s, s, s}, Vector2{1.f, 1.f}),
                 Vertex(Vector3{-s, s, s}, Vector2{0.f, 1.f})
         }, spWood); // front
    addFace({
                 Vertex(Vector3{-s, -s, -s}, Vector2{1, 0}),
                 Vertex(Vector3{-s, s, -s}, Vector2{1, 1}),
                 Vertex(Vector3{s, s, -s}, Vector2{0, 1}),
                 Vertex(Vector3{s, -s, -s}, Vector2{0, 0})
         }, spWood); // back
    addFace({
                 Vertex(Vector3{-s, s, -s}, Vector2{0, 1}),
                 Vertex(Vector3{-s, s, s}, Vector2{0, 0}),
                 Vertex(Vector3{s, s, s}, Vector2{1, 0}),
                 Vertex(Vector3{s, s, -s}, Vector2{1, 1})
         }, spGrass); // top
    addFace({
                 Vertex(Vector3{-s, -s, -s}, Vector2{1, 1}),
                 Vertex(Vector3{s, -s, -s}, Vector2{0, 1}),
                 Vertex(Vector3{s, -s, s}, Vector2{0, 0}),
                 Vertex(Vector3{-s, -s, s}, Vector2{1, 0})
         }, spWood); // bottom
    addFace({
                 Vertex(Vector3{s, -s, -s}, Vector2{1, 0}),
                 Vertex(Vector3{s, s, -s}, Vector2{1, 1}),
                 Vertex(Vector3{s, s, s}, Vector2{0, 1}),
                 Vertex(Vector3{s, -s, s}, Vector2{0, 0})
         }, spWood); // right
    addFace({
                 Vertex(Vector3{-s, -s, -s}, Vector2{0, 0}),
                 Vertex(Vector3{-s, -s, s}, Vector2{1, 0}),
                 Vertex(Vector3{-s, s, s}, Vector2{1, 1}),
                 Vertex(Vector3{-s, s, -s}, Vector2{0, 1})
         }, spWood); // left
}

void Renderer::buildTileQuads(AAssetManager *assetManager) {
    auto sp = TextureAsset::loadAsset(assetManager, "set-001.jpg");
    for (int row = 0; row <= 1; row++)
        for (int col = 0; col <= 1; col++) {
            float u1 = col / 4.f, u2 = (col + 1) / 4.f, v1 = row / 4.f, v2 = (row + 1) / 4.f;
            float x = col * 2.f - 1.f, y = 1.f - row * 2.f;
            std::vector<Vertex> v = {
                    Vertex(Vector3{x - 0.5f, y - 0.5f, 0.f}, Vector2{u1, v1}),
                    Vertex(Vector3{x + 0.5f, y - 0.5f, 0.f}, Vector2{u2, v1}),
                    Vertex(Vector3{x + 0.5f, y + 0.5f, 0.f}, Vector2{u2, v2}),
                    Vertex(Vector3{x - 0.5f, y + 0.5f, 0.f}, Vector2{u1, v2})
            };
            std::vector<Index> i = {0, 1, 2, 0, 2, 3};
            models_.emplace_back(v, i, sp);
        }
}

void Renderer::buildTexturedPyramid(AAssetManager *assetManager, const char *texturePath) {
    auto sp = TextureAsset::loadAsset(assetManager, texturePath);
    float h = 0.5f;
    std::vector<Vertex> v = {
            Vertex(Vector3{0.f, h, 0.f}, Vector2{0.5f, 1.f}),
            Vertex(Vector3{-h, -h, h}, Vector2{0.f, 0.f}),
            Vertex(Vector3{h, -h, h}, Vector2{1.f, 0.f}),
            Vertex(Vector3{0.f, h, 0.f}, Vector2{0.5f, 1.f}),
            Vertex(Vector3{h, -h, -h}, Vector2{1.f, 0.f}),
            Vertex(Vector3{-h, -h, -h}, Vector2{0.f, 0.f}),
            Vertex(Vector3{0.f, h, 0.f}, Vector2{0.5f, 1.f}),
            Vertex(Vector3{h, -h, h}, Vector2{0.f, 0.f}),
            Vertex(Vector3{h, -h, -h}, Vector2{1.f, 0.f}),
            Vertex(Vector3{0.f, h, 0.f}, Vector2{0.5f, 1.f}),
            Vertex(Vector3{-h, -h, -h}, Vector2{1.f, 0.f}),
            Vertex(Vector3{-h, -h, h}, Vector2{0.f, 0.f}),
            Vertex(Vector3{-h, -h, h}, Vector2{0.f, 0.f}),
            Vertex(Vector3{h, -h, h}, Vector2{1.f, 0.f}),
            Vertex(Vector3{h, -h, -h}, Vector2{1.f, 1.f}),
            Vertex(Vector3{-h, -h, -h}, Vector2{0.f, 1.f})
    };
    std::vector<Index> i = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 12, 14, 15};
    models_.emplace_back(v, i, sp);
}

void Renderer::buildScene014(AAssetManager *assetManager) {
    auto spGrass = TextureAsset::loadAsset(assetManager, "grass.jpg");
    auto spWood = TextureAsset::loadAsset(assetManager, "wood.jpg");
    auto spSet = TextureAsset::loadAsset(assetManager, "set-001.jpg");
    std::vector<Vertex> ground = {
            Vertex(Vector3{-3.f, 0.f, -3.f}, Vector2{0.f, 0.f}),
            Vertex(Vector3{3.f, 0.f, -3.f}, Vector2{4.f, 0.f}),
            Vertex(Vector3{3.f, 0.f, 3.f}, Vector2{4.f, 4.f}),
            Vertex(Vector3{-3.f, 0.f, 3.f}, Vector2{0.f, 4.f})
    };
    models_.emplace_back(ground, std::vector<Index>{0, 1, 2, 0, 2, 3}, spGrass);
    buildTexturedCube(assetManager, 0.5f, "wood.jpg", false);
    float u1 = 0.f, u2 = 0.25f, v1 = 0.f, v2 = 0.25f;
    std::vector<Vertex> tile1 = {
            Vertex(Vector3{0.9f, 0.9f, 1.5f}, Vector2{u1, v1}),
            Vertex(Vector3{1.9f, 0.9f, 1.5f}, Vector2{u2, v1}),
            Vertex(Vector3{1.9f, 1.9f, 1.5f}, Vector2{u2, v2}),
            Vertex(Vector3{0.9f, 1.9f, 1.5f}, Vector2{u1, v2})
    };
    models_.emplace_back(tile1, std::vector<Index>{0, 1, 2, 0, 2, 3}, spSet);
    u1 = 0.25f; u2 = 0.5f; v1 = 0.25f; v2 = 0.5f;
    std::vector<Vertex> tile2 = {
            Vertex(Vector3{-0.9f, 0.6f, 1.5f}, Vector2{u1, v1}),
            Vertex(Vector3{-0.1f, 0.6f, 1.5f}, Vector2{u2, v1}),
            Vertex(Vector3{-0.1f, 1.4f, 1.5f}, Vector2{u2, v2}),
            Vertex(Vector3{-0.9f, 1.4f, 1.5f}, Vector2{u1, v2})
    };
    models_.emplace_back(tile2, std::vector<Index>{0, 1, 2, 0, 2, 3}, spSet);
}

void Renderer::buildTileQuad015(AAssetManager *assetManager) {
    auto sp = TextureAsset::loadAsset(assetManager, "set-001.jpg");
    float u1 = 0.f, u2 = 0.25f, v1 = 0.f, v2 = 0.25f;
    std::vector<Vertex> v = {
            Vertex(Vector3{1.9f, -0.3f, 0.f}, Vector2{u1, v1}),
            Vertex(Vector3{2.5f, -0.3f, 0.f}, Vector2{u2, v1}),
            Vertex(Vector3{2.5f, 0.3f, 0.f}, Vector2{u2, v2}),
            Vertex(Vector3{1.9f, 0.3f, 0.f}, Vector2{u1, v2})
    };
    models_.emplace_back(v, std::vector<Index>{0, 1, 2, 0, 2, 3}, sp);
}

void Renderer::drawBackButtonOverlay() {
    if ((exampleIndex_ < 1 && sceneIndex_ < 0) || width_ <= 0 || height_ <= 0) return;

    PendingBackLabel pending;
    if (getPendingBackButtonLabel(&pending) && pending.pixels && pending.width > 0 && pending.height > 0) {
        if (backButtonTextureId_) {
            glDeleteTextures(1, &backButtonTextureId_);
            backButtonTextureId_ = 0;
        }
        glGenTextures(1, &backButtonTextureId_);
        glBindTexture(GL_TEXTURE_2D, backButtonTextureId_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pending.width, pending.height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, pending.pixels);
        clearPendingBackButtonLabel();
    }

    glDisable(GL_DEPTH_TEST);

    float proj[16];
    float halfW = width_ * 0.5f;
    float halfH = height_ * 0.5f;
    Utility::buildOrthographicMatrix(proj, halfH, float(width_) / height_, -1.f, 1.f);
    float left = kBackButtonLeft - halfW;
    float right = (kBackButtonLeft + kBackButtonWidth) - halfW;
    float bottom = (height_ - kBackButtonTop - kBackButtonHeight) - halfH;
    float top = (height_ - kBackButtonTop) - halfH;

    ColoredVertex bgVerts[] = {
            {left, bottom, 0.f, 0.2f, 0.2f, 0.4f, 0.9f},
            {right, bottom, 0.f, 0.2f, 0.2f, 0.4f, 0.9f},
            {right, top, 0.f, 0.2f, 0.2f, 0.4f, 0.9f},
            {left, top, 0.f, 0.2f, 0.2f, 0.4f, 0.9f}
    };
    uint16_t bgIndices[] = {0, 1, 2, 0, 2, 3};
    shaderColor_->activate();
    shaderColor_->setMVP(proj);
    shaderColor_->draw(bgVerts, bgIndices, 6, GL_TRIANGLES);

    if (backButtonTextureId_) {
        Vertex texVerts[] = {
                Vertex(Vector3{left, bottom, 0.f}, Vector2{0.f, 1.f}),
                Vertex(Vector3{right, bottom, 0.f}, Vector2{1.f, 1.f}),
                Vertex(Vector3{right, top, 0.f}, Vector2{1.f, 0.f}),
                Vertex(Vector3{left, top, 0.f}, Vector2{0.f, 0.f})
        };
        uint16_t texIndices[] = {0, 1, 2, 0, 2, 3};
        shader_->activate();
        shader_->setTexOffset(0.f, 0.f);
        shader_->setProjectionMatrix(proj);
        shader_->drawTexturedQuad(texVerts, 4, texIndices, 6, backButtonTextureId_);
    }

    glEnable(GL_DEPTH_TEST);
}

void Renderer::handleInput() {
    // handle all queued inputs
    auto *inputBuffer = android_app_swap_input_buffers(app_);
    if (!inputBuffer) {
        // no inputs yet.
        return;
    }

    // handle motion events (motionEventsCounts can be 0).
    for (auto i = 0; i < inputBuffer->motionEventsCount; i++) {
        auto &motionEvent = inputBuffer->motionEvents[i];
        auto action = motionEvent.action;

        // Find the pointer index, mask and bitshift to turn it into a readable value.
        auto pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        aout << "Pointer(s): ";

        // get the x and y position of this event if it is not ACTION_MOVE.
        auto &pointer = motionEvent.pointers[pointerIndex];
        auto x = GameActivityPointerAxes_getX(&pointer);
        auto y = GameActivityPointerAxes_getY(&pointer);

        // determine the action type and process the event accordingly.
        switch (action & AMOTION_EVENT_ACTION_MASK) {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN: {
                aout << "(" << pointer.id << ", " << x << ", " << y << ") "
                     << "Pointer Down";
                if (exampleIndex_ >= 1 || sceneIndex_ >= 0) {
                    int px = static_cast<int>(x);
                    int py = static_cast<int>(y);
                    if (px >= kBackButtonLeft && px <= kBackButtonLeft + kBackButtonWidth &&
                        py >= kBackButtonTop && py <= kBackButtonTop + kBackButtonHeight) {
                        requestFinishActivity(app_);
                    }
                }
                break;
            }

            case AMOTION_EVENT_ACTION_CANCEL:
                // treat the CANCEL as an UP event: doing nothing in the app, except
                // removing the pointer from the cache if pointers are locally saved.
                // code pass through on purpose.
            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_POINTER_UP:
                aout << "(" << pointer.id << ", " << x << ", " << y << ") "
                     << "Pointer Up";
                break;

            case AMOTION_EVENT_ACTION_MOVE:
                // There is no pointer index for ACTION_MOVE, only a snapshot of
                // all active pointers; app needs to cache previous active pointers
                // to figure out which ones are actually moved.
                for (auto index = 0; index < motionEvent.pointerCount; index++) {
                    pointer = motionEvent.pointers[index];
                    x = GameActivityPointerAxes_getX(&pointer);
                    y = GameActivityPointerAxes_getY(&pointer);
                    aout << "(" << pointer.id << ", " << x << ", " << y << ")";

                    if (index != (motionEvent.pointerCount - 1)) aout << ",";
                    aout << " ";
                }
                aout << "Pointer Move";
                break;
            default:
                aout << "Unknown MotionEvent Action: " << action;
        }
        aout << std::endl;
    }
    // clear the motion input count in this buffer for main thread to re-use.
    android_app_clear_motion_events(inputBuffer);

    // handle input key events.
    for (auto i = 0; i < inputBuffer->keyEventsCount; i++) {
        auto &keyEvent = inputBuffer->keyEvents[i];
        aout << "Key: " << keyEvent.keyCode <<" ";
        switch (keyEvent.action) {
            case AKEY_EVENT_ACTION_DOWN:
                aout << "Key Down";
                break;
            case AKEY_EVENT_ACTION_UP:
                aout << "Key Up";
                break;
            case AKEY_EVENT_ACTION_MULTIPLE:
                // Deprecated since Android API level 29.
                aout << "Multiple Key Actions";
                break;
            default:
                aout << "Unknown KeyEvent Action: " << keyEvent.action;
        }
        aout << std::endl;
    }
    // clear the key input count too.
    android_app_clear_key_events(inputBuffer);
}