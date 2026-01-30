# GenesisV Android

Android app that runs the [GenesisV](https://github.com/jsegura17/GenesisV) OpenGL examples using **OpenGL ES 3** and **C++** (NDK), with a Kotlin launcher and menu.

---

## English

### What is this?

**GenesisV Android** is an Android project that brings the GenesisV desktop OpenGL tutorials to mobile. It uses:

- **Kotlin** for the UI (menu and activity lifecycle)
- **C++** and **OpenGL ES 3** for all rendering (via Android NDK)
- **Game Activity** (Android Games SDK) for the fullscreen OpenGL surface and input

The original [GenesisV](https://github.com/jsegura17/GenesisV) repo is a Windows/MinGW OpenGL learning project with 15 examples (rotating triangle, colored cube, textures, etc.). This Android version reimplements those examples with ES 3 (shaders, VBO-style drawing, no fixed pipeline).

### Features

- **Menu screen**: list of 15 examples (001–015); tap one to open the OpenGL view
- **Back button**: in each example, an on-screen “Back Menu” button (top-left) returns to the menu; the system back key also finishes the activity
- **Examples 001–015**: triangle, colored quad, wireframe cube, solid colored cube, multiple objects, textured quad (wood), textured cube, cube with different textures per face, animated texture, texture filtering, tiles from a texture set, textured cube + pyramid, textured cube (no lighting), complex scene (ground + cube + tiles), advanced texture effects
- **Assets**: `wood.jpg`, `grass.jpg`, `set-001.jpg`, and `android_robot.png` in `app/src/main/assets/`

### Requirements

- **Android Studio** (or compatible IDE) with Android SDK
- **Android NDK** and **CMake** (for building the C++ library)
- **minSdk 33**, **targetSdk 36**
- Device/emulator with OpenGL ES 3 support

### How to build and run

1. Open the project in Android Studio.
2. Sync Gradle and wait for the native build to finish.
3. Run the app on a device or emulator (e.g. **Run > Run 'app'**).
4. On launch you see the **menu**; choose an example (001–015).
5. Use **“Back Menu”** (top-left) or the **system Back** key to return to the menu.

### Project structure

```
app/src/main/
├── java/com/example/genesisv/
│   ├── MenuActivity.kt      # Launcher: menu list, starts MainActivity with example index
│   └── MainActivity.kt      # GameActivity: loads native lib, passes example index, fullscreen
├── cpp/
│   ├── main.cpp             # android_main, event loop, creates Renderer
│   ├── Renderer.cpp/h       # EGL/GL init, per-example geometry and render (001–015)
│   ├── Shader.cpp/h         # Textured shader (position + UV, uProjection, uTexOffset)
│   ├── ShaderColor.cpp/h     # Color-only shader (position + color, uMVP)
│   ├── Model.h              # Vertex, Index, Model (vertices + indices + texture)
│   ├── TextureAsset.cpp/h   # Load PNG/JPG from assets via AImageDecoder
│   ├── Utility.cpp/h        # Ortho/perspective/rotation matrices, GL error check
│   ├── JniBridge.cpp/h      # getExampleIndex, setExampleIndex, requestFinishActivity, back-label bitmap
│   └── AndroidOut.cpp/h     # Logging to logcat from C++
└── assets/
    ├── wood.jpg
    ├── grass.jpg
    ├── set-001.jpg
    └── android_robot.png
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

### License

Educational project. Use and modify as you like.

---

## Español

### ¿Qué es este proyecto?

**GenesisV Android** es una aplicación Android que lleva los tutoriales de OpenGL del proyecto [GenesisV](https://github.com/jsegura17/GenesisV) (escritorio) al móvil. Utiliza:

- **Kotlin** para la interfaz (menú y ciclo de vida de la actividad)
- **C++** y **OpenGL ES 3** para todo el dibujado (vía Android NDK)
- **Game Activity** (Android Games SDK) para la superficie OpenGL a pantalla completa y la entrada

El repositorio original [GenesisV](https://github.com/jsegura17/GenesisV) es un proyecto de aprendizaje de OpenGL en Windows/MinGW con 15 ejemplos (triángulo rotando, cubo con colores, texturas, etc.). Esta versión Android reimplementa esos ejemplos con ES 3 (shaders, dibujo tipo VBO, sin pipeline fijo).

### Características

- **Pantalla de menú**: lista de 15 ejemplos (001–015); al tocar uno se abre la vista OpenGL.
- **Botón atrás**: en cada ejemplo, un botón “Back Menu” en pantalla (arriba a la izquierda) vuelve al menú; el botón atrás del sistema también cierra la actividad.
- **Ejemplos 001–015**: triángulo, cuadrado con colores, cubo en alambre, cubo sólido con colores, varios objetos, quad con textura (madera), cubo con textura, cubo con texturas distintas por cara, textura animada, filtrado de textura, tiles desde un set de texturas, cubo y pirámide con texturas, cubo con textura, escena compleja (suelo + cubo + tiles), efectos avanzados con texturas.
- **Assets**: `wood.jpg`, `grass.jpg`, `set-001.jpg` y `android_robot.png` en `app/src/main/assets/`.

### Requisitos

- **Android Studio** (o IDE compatible) con Android SDK
- **Android NDK** y **CMake** (para compilar la librería C++)
- **minSdk 33**, **targetSdk 36**
- Dispositivo o emulador con soporte OpenGL ES 3

### Cómo compilar y ejecutar

1. Abre el proyecto en Android Studio.
2. Sincroniza Gradle y espera a que termine la compilación nativa.
3. Ejecuta la app en un dispositivo o emulador (p. ej. **Run > Run 'app'**).
4. Al iniciar verás el **menú**; elige un ejemplo (001–015).
5. Usa **“Back Menu”** (arriba a la izquierda) o el botón **Atrás** del sistema para volver al menú.

### Estructura del proyecto

```
app/src/main/
├── java/com/example/genesisv/
│   ├── MenuActivity.kt      # Pantalla de menú; inicia MainActivity con el índice del ejemplo
│   └── MainActivity.kt      # GameActivity: carga la lib nativa, pasa el índice, pantalla completa
├── cpp/
│   ├── main.cpp             # android_main, bucle de eventos, crea el Renderer
│   ├── Renderer.cpp/h       # Inicialización EGL/GL, geometría y dibujado por ejemplo (001–015)
│   ├── Shader.cpp/h         # Shader con textura (posición + UV, uProjection, uTexOffset)
│   ├── ShaderColor.cpp/h    # Shader solo color (posición + color, uMVP)
│   ├── Model.h              # Vertex, Index, Model (vértices + índices + textura)
│   ├── TextureAsset.cpp/h   # Carga PNG/JPG desde assets con AImageDecoder
│   ├── Utility.cpp/h        # Matrices orto/perspectiva/rotación, comprobación de errores GL
│   ├── JniBridge.cpp/h      # getExampleIndex, setExampleIndex, requestFinishActivity, bitmap del botón
│   └── AndroidOut.cpp/h     # Salida a logcat desde C++
└── assets/
    ├── wood.jpg
    ├── grass.jpg
    ├── set-001.jpg
    └── android_robot.png
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

### Licencia

Proyecto educativo. Puedes usarlo y modificarlo libremente.
