#ifndef ANDROIDGLINVESTIGATIONS_UTILITY_H
#define ANDROIDGLINVESTIGATIONS_UTILITY_H

#include <cassert>

class Utility {
public:
    static bool checkAndLogGlError(bool alwaysLog = false);

    static inline void assertGlError() { assert(checkAndLogGlError()); }

    /**
     * Generates an orthographic projection matrix given the half height, aspect ratio, near, and far
     * planes
     *
     * @param outMatrix the matrix to write into
     * @param halfHeight half of the height of the screen
     * @param aspect the width of the screen divided by the height
     * @param near the distance of the near plane
     * @param far the distance of the far plane
     * @return the generated matrix, this will be the same as @a outMatrix so you can chain calls
     *     together if needed
     */
    static float *buildOrthographicMatrix(
            float *outMatrix,
            float halfHeight,
            float aspect,
            float near,
            float far);

    /** Perspective matrix (column-major). fovY_rad in radians. */
    static float *buildPerspectiveMatrix(
            float *outMatrix,
            float fovY_rad,
            float aspect,
            float near,
            float far);

    /** Translation matrix (column-major). */
    static float *buildTranslationMatrix(float *outMatrix, float x, float y, float z);

    /** Rotation around X axis (column-major), angle in degrees. */
    static float *buildRotationX(float *outMatrix, float angleDeg);
    /** Rotation around Y axis (column-major), angle in degrees. */
    static float *buildRotationY(float *outMatrix, float angleDeg);
    /** Rotation around Z axis (column-major), angle in degrees. */
    static float *buildRotationZ(float *outMatrix, float angleDeg);

    /** out = A * B (column-major 4x4). */
    static float *matrixMultiply(float *out, const float *A, const float *B);

    static float *buildIdentityMatrix(float *outMatrix);
};

#endif //ANDROIDGLINVESTIGATIONS_UTILITY_H