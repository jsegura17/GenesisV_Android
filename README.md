# GenesisV Android

Android app that runs the [GenesisV](https://github.com/jsegura17/GenesisV) OpenGL examples using **OpenGL ES 3** and **C++** (NDK), with a Kotlin launcher and menu. Includes a main menu, OpenGL examples (001–015), Scenes OpenGL (tilemap/LevelManager), and a parameters screen.

---

## English

### What is this?

**GenesisV Android** is an Android project that brings the GenesisV desktop OpenGL tutorials to mobile. It uses:

- **Kotlin** for the UI (menu, parameters, activity lifecycle)
- **C++** and **OpenGL ES 3** for all rendering (via Android NDK)
- **Game Activity** (Android Games SDK) for the fullscreen OpenGL surface and input

The original [GenesisV](https://github.com/jsegura17/GenesisV) repo is a Windows/MinGW OpenGL learning project with 15 examples. This Android version reimplements those examples with ES 3 and adds a **Scenes OpenGL** section with a tilemap (LevelManager) for a 2D platform-style floor.

### Main menu

- **Ejemplos OpenGL** — Opens a list of 15 examples (001–015). Tap one to run it in fullscreen OpenGL.
- **Scenes OpenGL** — Opens a list of 5 scene options:
  - **Scene 2D - Platform - Floor**: Tilemap rendered with LevelManager (matrix of tile IDs → textured quads). Uses assets from `deserttileset/Tile/` (1.png, 2.png, 3.png, 5.png, 7.png).
  - **Scene 2D - Platform - Background**, **Static Obj**, **Anim**, **Player**: Placeholder “Under Construction” screens for future use.
- **Parametros** — Settings screen with a toggle **Activar Rotación de Pantalla**. When enabled, the OpenGL screen (MainActivity) can rotate with the device; when disabled, it stays in portrait. Value is saved in SharedPreferences.
- **Exit App** — Closes the app (finishes all activities in the task).

### Features

- **Menu screen**: Main menu with four options (Ejemplos OpenGL, Scenes OpenGL, Parametros, Exit App).
- **Ejemplos OpenGL**: Submenu with 15 examples (001–015); tap one to open the OpenGL view.
- **Scenes OpenGL**: Submenu with 5 scene options; first one (Floor) draws a tilemap via LevelManager; others show “Under Construction”.
- **Back button**: In each OpenGL example or scene, an on-screen “Back Menu” button (top-left) returns to the previous screen; the system back key also finishes the activity.
- **Parameters**: Toggle for screen rotation in OpenGL view; state persisted in SharedPreferences.
- **Examples 001–015**: Rotating triangle, colored quad, wireframe cube, solid colored cube, multiple objects, textured quad (wood), textured cube, cube with different textures per face, animated texture, texture filtering, tiles from a texture set, textured cube + pyramid, textured cube, complex scene (ground + cube + tiles), advanced texture effects.
- **LevelManager**: Loads a level from a matrix of integers (or from a .txt file); each non-zero cell becomes a `TileEntity` (position + texture ID). Draws all tiles with the textured shader. Used in “Scene 2D - Platform - Floor”.
- **Assets**: `wood.jpg`, `grass.jpg`, `set-001.jpg`, `android_robot.png`, and `deserttileset/` (Tile 1–16, Objects) in `app/src/main/assets/`.

### Requirements

- **Android Studio** (or compatible IDE) with Android SDK
- **Android NDK** and **CMake** (for building the C++ library)
- **minSdk 33**, **targetSdk 36**
- Device/emulator with OpenGL ES 3 support

### How to build and run

1. Open the project in Android Studio.
2. Sync Gradle and wait for the native build to finish.
3. Run the app on a device or emulator (e.g. **Run > Run 'app'**).
4. On launch you see the **main menu**. Choose **Ejemplos OpenGL** for the 15 examples, **Scenes OpenGL** for the tilemap/floor and other scenes, **Parametros** to toggle screen rotation, or **Exit App** to close.
5. In any OpenGL screen, use **“Back Menu”** (top-left) or the **system Back** key to return.

### Project structure

```
app/src/main/
├── java/com/example/genesisv/
│   ├── MenuActivity.kt           # Launcher: main menu (Ejemplos, Scenes, Parametros, Exit App)
│   ├── ExamplesListActivity.kt  # Submenu: list of 15 examples → MainActivity with example index
│   ├── ScenesMenuActivity.kt    # Submenu: 5 scene options → MainActivity (Floor) or UnderConstruction
│   ├── ParametersActivity.kt    # Toggle "Activar Rotación de Pantalla", SharedPreferences
│   ├── UnderConstructionActivity.kt  # Placeholder for scenes 2–5
│   └── MainActivity.kt           # GameActivity: OpenGL surface, setExampleIndex/setSceneIndex, Back Menu
├── cpp/
│   ├── main.cpp                  # android_main, event loop, creates Renderer(exampleIndex, sceneIndex)
│   ├── Renderer.cpp/h            # EGL/GL init, examples 001–015, scene 0 (LevelManager), Back Menu overlay
│   ├── LevelManager.cpp/h        # LoadLevel(matrix), LoadLevelFromFile(.txt), Draw(Shader) — tilemap
│   ├── TileTextureManager.cpp/h  # getTextureId(tileId) — loads deserttileset/Tile/1.png etc., fallback
│   ├── Shader.cpp/h              # Textured shader (position + UV, uProjection, uTexOffset)
│   ├── ShaderColor.cpp/h         # Color-only shader (position + color, uMVP)
│   ├── Model.h                   # Vertex, Index, Model (vertices + indices + texture)
│   ├── TextureAsset.cpp/h        # Load PNG/JPG from assets via AImageDecoder
│   ├── Utility.cpp/h             # Ortho/perspective/rotation matrices, GL error check
│   ├── JniBridge.cpp/h           # getExampleIndex, getSceneIndex, setExampleIndex, setSceneIndex, requestFinishActivity, back-label bitmap
│   └── AndroidOut.cpp/h          # Logging to logcat from C++
└── assets/
    ├── wood.jpg, grass.jpg, set-001.jpg, android_robot.png
    └── deserttileset/
        ├── Tile/   # 1.png … 16.png (for LevelManager tilemap)
        └── Objects/
```

### Examples (001–015)

| #   | Description |
|-----|-------------|
| 001 | Rotating triangle (per-vertex colors) |
| 002 | Rotating colored quad |
| 003 | Wireframe cube |
| 004 | Solid colored cube (different color per face) |
| 005 | Multiple objects: cube and pyramid, rotating at different speeds |
| 006 | Textured quad (wood), rotating |
| 007 | Textured cube (wood), rotating |
| 008 | Cube with wood on 5 faces, grass on top |
| 009 | Animated texture (UV offset) on a quad |
| 010 | Textured quad with repeated UVs (filtering demo) |
| 011 | Four quads showing different tiles from a 4×4 texture set |
| 012 | Textured cube (wood) and textured pyramid (grass), side by side |
| 013 | Textured cube (same as 007) |
| 014 | Complex scene: grass ground, wooden cube, and tile quads |
| 015 | Textured cube plus a tile quad |

### Scenes OpenGL

| Option | Description |
|--------|-------------|
| Scene 2D - Platform - Floor | Tilemap from LevelManager; matrix `{{1,2,7,2,3},{0,5,5,5,0}}`; tiles 1,2,3,5,7 from `deserttileset/Tile/`. |
| Scene 2D - Platform - Background / Static Obj / Anim / Player | “Under Construction” placeholder. |

### License

Educational project. Use and modify as you like.
2026 - GenesisCGt - GameDivision
---

## Español

### ¿Qué es este proyecto?

**GenesisV Android** es una aplicación Android que lleva los tutoriales de OpenGL del proyecto [GenesisV](https://github.com/jsegura17/GenesisV) (escritorio) al móvil. Utiliza:

- **Kotlin** para la interfaz (menú, parámetros y ciclo de vida de las actividades)
- **C++** y **OpenGL ES 3** para todo el dibujado (vía Android NDK)
- **Game Activity** (Android Games SDK) para la superficie OpenGL a pantalla completa y la entrada

El repositorio original [GenesisV](https://github.com/jsegura17/GenesisV) es un proyecto de aprendizaje de OpenGL en Windows/MinGW con 15 ejemplos. Esta versión Android reimplementa esos ejemplos con ES 3 y añade la sección **Scenes OpenGL** con un tilemap (LevelManager) para un suelo tipo plataforma 2D.

### Menú principal

- **Ejemplos OpenGL** — Abre la lista de 15 ejemplos (001–015). Al tocar uno se ejecuta en pantalla completa OpenGL.
- **Scenes OpenGL** — Abre una lista de 5 opciones de escena:
  - **Scene 2D - Platform - Floor**: Tilemap dibujado con LevelManager (matriz de IDs de tiles → quads con textura). Usa assets de `deserttileset/Tile/` (1.png, 2.png, 3.png, 5.png, 7.png).
  - **Scene 2D - Platform - Background**, **Static Obj**, **Anim**, **Player**: Pantallas placeholder “Under Construction” para uso futuro.
- **Parametros** — Pantalla de ajustes con un toggle **Activar Rotación de Pantalla**. Si está activado, la pantalla OpenGL (MainActivity) puede girar con el dispositivo; si está desactivado, se mantiene en vertical. El valor se guarda en SharedPreferences.
- **Exit App** — Cierra la aplicación (cierra todas las actividades de la tarea).

### Características

- **Pantalla de menú**: Menú principal con cuatro opciones (Ejemplos OpenGL, Scenes OpenGL, Parametros, Exit App).
- **Ejemplos OpenGL**: Submenú con 15 ejemplos (001–015); al tocar uno se abre la vista OpenGL.
- **Scenes OpenGL**: Submenú con 5 opciones de escena; la primera (Floor) dibuja un tilemap con LevelManager; el resto muestra “Under Construction”.
- **Botón atrás**: En cada ejemplo o escena OpenGL, un botón “Back Menu” en pantalla (arriba a la izquierda) vuelve a la pantalla anterior; el botón atrás del sistema también cierra la actividad.
- **Parámetros**: Toggle para rotación de pantalla en la vista OpenGL; estado guardado en SharedPreferences.
- **Ejemplos 001–015**: Triángulo rotando, cuadrado con colores, cubo en alambre, cubo sólido con colores, varios objetos, quad con textura (madera), cubo con textura, cubo con texturas distintas por cara, textura animada, filtrado de textura, tiles desde un set de texturas, cubo y pirámide con texturas, cubo con textura, escena compleja (suelo + cubo + tiles), efectos avanzados con texturas.
- **LevelManager**: Carga un nivel desde una matriz de enteros (o desde un .txt); cada celda distinta de cero se convierte en un `TileEntity` (posición + ID de textura). Dibuja todos los tiles con el shader de textura. Se usa en “Scene 2D - Platform - Floor”.
- **Assets**: `wood.jpg`, `grass.jpg`, `set-001.jpg`, `android_robot.png` y `deserttileset/` (Tile 1–16, Objects) en `app/src/main/assets/`.

### Requisitos

- **Android Studio** (o IDE compatible) con Android SDK
- **Android NDK** y **CMake** (para compilar la librería C++)
- **minSdk 33**, **targetSdk 36**
- Dispositivo o emulador con soporte OpenGL ES 3

### Cómo compilar y ejecutar

1. Abre el proyecto en Android Studio.
2. Sincroniza Gradle y espera a que termine la compilación nativa.
3. Ejecuta la app en un dispositivo o emulador (p. ej. **Run > Run 'app'**).
4. Al iniciar verás el **menú principal**. Elige **Ejemplos OpenGL** para los 15 ejemplos, **Scenes OpenGL** para el tilemap/suelo y otras escenas, **Parametros** para activar/desactivar la rotación de pantalla, o **Exit App** para cerrar.
5. En cualquier pantalla OpenGL, usa **“Back Menu”** (arriba a la izquierda) o el botón **Atrás** del sistema para volver.

### Estructura del proyecto

```
app/src/main/
├── java/com/example/genesisv/
│   ├── MenuActivity.kt           # Lanzador: menú principal (Ejemplos, Scenes, Parametros, Exit App)
│   ├── ExamplesListActivity.kt  # Submenú: lista de 15 ejemplos → MainActivity con índice
│   ├── ScenesMenuActivity.kt    # Submenú: 5 opciones de escena → MainActivity (Floor) o UnderConstruction
│   ├── ParametersActivity.kt    # Toggle "Activar Rotación de Pantalla", SharedPreferences
│   ├── UnderConstructionActivity.kt  # Placeholder para escenas 2–5
│   └── MainActivity.kt          # GameActivity: superficie OpenGL, setExampleIndex/setSceneIndex, Back Menu
├── cpp/
│   ├── main.cpp                  # android_main, bucle de eventos, crea Renderer(exampleIndex, sceneIndex)
│   ├── Renderer.cpp/h            # Inicialización EGL/GL, ejemplos 001–015, escena 0 (LevelManager), overlay Back Menu
│   ├── LevelManager.cpp/h        # LoadLevel(matrix), LoadLevelFromFile(.txt), Draw(Shader) — tilemap
│   ├── TileTextureManager.cpp/h  # getTextureId(tileId) — carga deserttileset/Tile/1.png etc., fallback
│   ├── Shader.cpp/h              # Shader con textura (posición + UV, uProjection, uTexOffset)
│   ├── ShaderColor.cpp/h         # Shader solo color (posición + color, uMVP)
│   ├── Model.h                   # Vertex, Index, Model (vértices + índices + textura)
│   ├── TextureAsset.cpp/h       # Carga PNG/JPG desde assets con AImageDecoder
│   ├── Utility.cpp/h             # Matrices orto/perspectiva/rotación, comprobación de errores GL
│   ├── JniBridge.cpp/h          # getExampleIndex, getSceneIndex, setExampleIndex, setSceneIndex, requestFinishActivity, bitmap del botón
│   └── AndroidOut.cpp/h         # Salida a logcat desde C++
└── assets/
    ├── wood.jpg, grass.jpg, set-001.jpg, android_robot.png
    └── deserttileset/
        ├── Tile/   # 1.png … 16.png (para tilemap LevelManager)
        └── Objects/
```

### Ejemplos (001–015)

| #   | Descripción |
|-----|-------------|
| 001 | Triángulo rotando (colores por vértice) |
| 002 | Cuadrado con colores rotando |
| 003 | Cubo en alambre (wireframe) |
| 004 | Cubo sólido con colores (cada cara de un color) |
| 005 | Varios objetos: cubo y pirámide rotando a distinta velocidad |
| 006 | Quad con textura (madera) rotando |
| 007 | Cubo con textura (madera) rotando |
| 008 | Cubo con madera en 5 caras y césped en la superior |
| 009 | Textura animada (offset de UV) en un quad |
| 010 | Quad con textura y UV repetidos (filtrado) |
| 011 | Cuatro quads con distintos tiles de un set de textura 4×4 |
| 012 | Cubo con textura (madera) y pirámide con textura (césped), lado a lado |
| 013 | Cubo con textura (igual que 007) |
| 014 | Escena compleja: suelo de césped, cubo de madera y quads con tiles |
| 015 | Cubo con textura más un quad con tile |

### Scenes OpenGL

| Opción | Descripción |
|--------|-------------|
| Scene 2D - Platform - Floor | Tilemap con LevelManager; matriz `{{1,2,7,2,3},{0,5,5,5,0}}`; tiles 1,2,3,5,7 de `deserttileset/Tile/`. |
| Scene 2D - Platform - Background / Static Obj / Anim / Player | Placeholder “Under Construction”. |

### Licencia

Proyecto educativo. Puedes usarlo y modificarlo libremente.
2026 - GenesisCGt - GameDivision