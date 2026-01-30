#ifndef ANDROIDGLINVESTIGATIONS_RENDERER_H
#define ANDROIDGLINVESTIGATIONS_RENDERER_H

#include <EGL/egl.h>
#include <android/asset_manager.h>
#include <memory>
#include <vector>

#include "Model.h"
#include "Shader.h"
#include "ShaderColor.h"

struct android_app;

class Renderer {
public:
    /*!
     * @param pApp the android_app this Renderer belongs to, needed to configure GL
     * @param exampleIndex índice del ejemplo del menú (0 = Base, 1 = 001, … 16 = 015)
     */
    inline Renderer(android_app *pApp, int exampleIndex = 0) :
            app_(pApp),
            exampleIndex_(exampleIndex),
            display_(EGL_NO_DISPLAY),
            surface_(EGL_NO_SURFACE),
            context_(EGL_NO_CONTEXT),
            width_(0),
            height_(0),
            shaderNeedsNewProjectionMatrix_(true),
            angle_(0.f),
            angleX_(0.f),
            angleY_(0.f),
            angleZ_(0.f),
            cubeAngleX_(0.f),
            cubeAngleY_(0.f),
            pyramidAngleX_(0.f),
            pyramidAngleY_(0.f),
            textureOffset_(0.f),
            coloredMode_(GL_TRIANGLES) {
        initRenderer();
    }

    virtual ~Renderer();

    /*!
     * Handles input from the android_app.
     *
     * Note: this will clear the input queue
     */
    void handleInput();

    /*!
     * Renders all the models in the renderer
     */
    void render();

private:
    /*!
     * Performs necessary OpenGL initialization. Customize this if you want to change your EGL
     * context or application-wide settings.
     */
    void initRenderer();

    /*!
     * @brief we have to check every frame to see if the framebuffer has changed in size. If it has,
     * update the viewport accordingly
     */
    void updateRenderArea();

    /*!
     * Creates the models for this sample. You'd likely load a scene configuration from a file or
     * use some other setup logic in your full game.
     */
    void createModels();

    /*! Overlay fijo "Back Menu" en la esquina superior izquierda (solo cuando exampleIndex_ >= 1). */
    void drawBackButtonOverlay();

    void buildTexturedCube(AAssetManager *assetManager, float halfSize, const char *texturePath, bool singleModel);
    void buildCubeMultiTexture(AAssetManager *assetManager);
    void buildTileQuads(AAssetManager *assetManager);
    void buildTexturedPyramid(AAssetManager *assetManager, const char *texturePath);
    void buildScene014(AAssetManager *assetManager);
    void buildTileQuad015(AAssetManager *assetManager);

    android_app *app_;
    int exampleIndex_;
    EGLDisplay display_;
    EGLSurface surface_;
    EGLContext context_;
    EGLint width_;
    EGLint height_;

    bool shaderNeedsNewProjectionMatrix_;

    float angle_;
    float angleX_;
    float angleY_;
    float angleZ_;
    float cubeAngleX_, cubeAngleY_, pyramidAngleX_, pyramidAngleY_;
    float textureOffset_;

    std::unique_ptr<Shader> shader_;
    std::unique_ptr<ShaderColor> shaderColor_;
    std::vector<Model> models_;

    std::vector<ColoredVertex> coloredVertices_;
    std::vector<uint16_t> coloredIndices_;
    GLenum coloredMode_;
    std::vector<ColoredVertex> coloredVertices2_;
    std::vector<uint16_t> coloredIndices2_;

    GLuint backButtonTextureId_ = 0;
    static constexpr int kBackButtonLeft = 20;
    static constexpr int kBackButtonTop = 20;
    static constexpr int kBackButtonWidth = 200;
    static constexpr int kBackButtonHeight = 56;
};

#endif //ANDROIDGLINVESTIGATIONS_RENDERER_H