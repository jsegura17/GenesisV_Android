#ifndef GENESISV_TILETEXTUREMANAGER_H
#define GENESISV_TILETEXTUREMANAGER_H

#include <android/asset_manager.h>
#include <map>
#include <GLES3/gl3.h>
#include <memory>
#include "TextureAsset.h"

/*!
 * Carga y cachea texturas de tiles por ID. Mapeo: 1→1.png, 2→2.png, 3→3.png, 5→5.png, 7→7.png.
 * Usar como resolver para LevelManager: getTextureId(tileId).
 */
class TileTextureManager {
public:
    /*! Carga bajo demanda; getTextureId() carga la textura la primera vez que se pide ese ID. */
    explicit TileTextureManager(AAssetManager *assetManager);

    /*!
     * Devuelve el GLuint de la textura para el tile ID dado.
     * Para IDs soportados (1, 2, 3, 5, 7) carga el PNG si hace falta. Otros IDs devuelven 0.
     */
    GLuint getTextureId(int tileId);

private:
    AAssetManager *assetManager_;
    std::map<int, std::shared_ptr<TextureAsset>> cache_;
    std::shared_ptr<TextureAsset> fallbackTexture_;
};

#endif //GENESISV_TILETEXTUREMANAGER_H
