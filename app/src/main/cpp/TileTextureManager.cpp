#include "TileTextureManager.h"

namespace {
    const char *tileIdToPath(int tileId) {
        switch (tileId) {
            case 1: return "deserttileset/Tile/1.png";
            case 2: return "deserttileset/Tile/2.png";
            case 3: return "deserttileset/Tile/3.png";
            case 5: return "deserttileset/Tile/5.png";
            case 7: return "deserttileset/Tile/7.png";
            default: return nullptr;
        }
    }
}

TileTextureManager::TileTextureManager(AAssetManager *assetManager)
    : assetManager_(assetManager) {}

GLuint TileTextureManager::getTextureId(int tileId) {
    if (!assetManager_)
        return 0;
    auto it = cache_.find(tileId);
    if (it != cache_.end())
        return it->second->getTextureID();
    const char *path = tileIdToPath(tileId);
    if (path) {
        auto sp = TextureAsset::loadAsset(assetManager_, path);
        if (sp) {
            GLuint id = sp->getTextureID();
            cache_[tileId] = std::move(sp);
            return id;
        }
    }
    if (!fallbackTexture_) {
        fallbackTexture_ = TextureAsset::loadAsset(assetManager_, "wood.jpg");
        if (!fallbackTexture_)
            fallbackTexture_ = TextureAsset::loadAsset(assetManager_, "android_robot.png");
    }
    return fallbackTexture_ ? fallbackTexture_->getTextureID() : 0;
}
