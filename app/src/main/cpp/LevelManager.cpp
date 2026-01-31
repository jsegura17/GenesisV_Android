#include "LevelManager.h"
#include <android/asset_manager.h>
#include <cstdlib>
#include <sstream>
#include <string>

LevelManager::LevelManager(std::function<GLuint(int)> getTextureId)
    : getTextureId_(std::move(getTextureId)) {}

void LevelManager::LoadLevel(const std::vector<std::vector<int>> &matrix) {
    tiles_.clear();
    for (size_t row = 0; row < matrix.size(); ++row) {
        const auto &line = matrix[row];
        for (size_t col = 0; col < line.size(); ++col) {
            int cell = line[col];
            if (cell == 0)
                continue;
            GLuint textureId = getTextureId_(cell);
            if (textureId == 0)
                continue;
            float x = static_cast<float>(col) * TILE_SIZE;
            float y = -static_cast<float>(row) * TILE_SIZE;
            float z = 0.f;
            tiles_.push_back(TileEntity{Vector3{x, y, z}, textureId});
        }
    }
}

bool LevelManager::LoadLevelFromFile(AAssetManager *assetManager, const std::string &path) {
    if (!assetManager)
        return false;
    AAsset *asset = AAssetManager_open(assetManager, path.c_str(), AASSET_MODE_BUFFER);
    if (!asset)
        return false;
    size_t length = static_cast<size_t>(AAsset_getLength(asset));
    std::string content(length, '\0');
    if (AAsset_read(asset, &content[0], length) != static_cast<int>(length)) {
        AAsset_close(asset);
        return false;
    }
    AAsset_close(asset);

    std::vector<std::vector<int>> matrix;
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        std::vector<int> row;
        std::istringstream lineStream(line);
        int value;
        while (lineStream >> value)
            row.push_back(value);
        if (!row.empty())
            matrix.push_back(std::move(row));
    }
    if (matrix.empty())
        return false;
    LoadLevel(matrix);
    return true;
}

void LevelManager::Draw(Shader &shader) const {
    const float h = TILE_SIZE * 0.5f;
    static const uint16_t kIndices[] = {0, 1, 2, 0, 2, 3};

    for (const TileEntity &entity : tiles_) {
        float x = entity.position.x;
        float y = entity.position.y;
        float z = entity.position.z;

        Vertex vertices[4] = {
            Vertex(Vector3{x - h, y - h, z}, Vector2{0.f, 0.f}),
            Vertex(Vector3{x + h, y - h, z}, Vector2{1.f, 0.f}),
            Vertex(Vector3{x + h, y + h, z}, Vector2{1.f, 1.f}),
            Vertex(Vector3{x - h, y + h, z}, Vector2{0.f, 1.f})
        };

        shader.drawTexturedQuad(vertices, 4, kIndices, 6, entity.textureId);
    }
}
