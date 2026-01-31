#ifndef GENESISV_LEVELMANAGER_H
#define GENESISV_LEVELMANAGER_H

#include <functional>
#include <string>
#include <vector>
#include <GLES3/gl3.h>
#include <android/asset_manager.h>

#include "Model.h"
#include "Shader.h"

/*! Un tile del nivel: posición en mundo y ID de textura OpenGL. */
struct TileEntity {
    Vector3 position;
    GLuint textureId;
};

/*!
 * Gestiona un nivel tilemap: carga una matriz de IDs (o desde .txt),
 * genera TileEntities y los dibuja con el Shader.
 */
class LevelManager {
public:
    /*! Tamaño en unidades mundo de un tile (ancho y alto). */
    static constexpr float TILE_SIZE = 1.0f;

    /*!
     * @param getTextureId Callback que dado un tile ID (1, 2, 3, 5, 7) devuelve el GLuint de la textura.
     *                     Para ID 0 (aire) no se llama.
     */
    explicit LevelManager(std::function<GLuint(int)> getTextureId);

    /*!
     * Carga el nivel desde una matriz de enteros. Recorre matrix[fila][col];
     * para cada valor != 0 calcula posición (x = col * TILE_SIZE, y = -row * TILE_SIZE)
     * y añade un TileEntity con el textureId devuelto por getTextureId.
     */
    void LoadLevel(const std::vector<std::vector<int>> &matrix);

    /*!
     * Carga el nivel desde un archivo .txt en assets.
     * Una línea por fila; números separados por espacios. Llama a LoadLevel con la matriz parseada.
     * @return true si se pudo abrir y parsear el archivo.
     */
    bool LoadLevelFromFile(struct AAssetManager *assetManager, const std::string &path);

    /*!
     * Dibuja todos los tiles: por cada TileEntity construye un quad centrado en position
     * y llama a shader.drawTexturedQuad(...). La proyección debe estar configurada fuera.
     */
    void Draw(Shader &shader) const;

private:
    std::function<GLuint(int)> getTextureId_;
    std::vector<TileEntity> tiles_;
};

#endif //GENESISV_LEVELMANAGER_H
