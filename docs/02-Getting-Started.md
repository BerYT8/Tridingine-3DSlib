# Primer proyecto

En este documento aprenderás a crear y ejecutar tu primer proyecto con **Tridingine-3DSlib**.

---

# Estructura del proyecto

Después de generar el kit de desarrollo, encontrarás una estructura similar a la siguiente:

```text
Lib/
├── include/
├── lib/
├── templates/
├── examples/
├── tools/
├── romfs/
├── build.bat
├── CMakeLists.txt
├── game.json
└── main.cpp
```

El archivo principal del proyecto es `main.cpp`, donde comenzará la ejecución del juego.

---

# Incluyendo la biblioteca

Para utilizar la API simplemente incluye:

```cpp
#include <Tridingine.h>
```

Toda la API está diseñada para utilizarse desde **C**, aunque también puede utilizarse sin problemas desde **C++**.

---

# Estructura mínima de un juego

Todo proyecto debe tener, como mínimo, la siguiente estructura:

```cpp
#include <Tridingine.h>

int main(int argc, char *argv[])
{
    S2S_ScreensInit();
#if defined(GAME_TITLE)
    const char* title = GAME_TITLE;
    SetWindowTitle(title);
#endif

    // Inicializaciones

    while (S2S_ScreensRunning())
    {
        S2S_BeginFrame();

        // Lógica

        S2S_EndFrame();
    }

    // Salidas

    S2S_ScreensExit();

    return 0;
}
```

Sin esta estructura no existirá una ventana válida ni un contexto de ejecución correctamente inicializado.

---

# Explicación

## S2S_ScreensInit()

Inicializa el sistema de pantallas.

Esta función prepara la aplicación para comenzar la ejecución del juego.

Debe llamarse una única vez al iniciar el programa.

---

## S2S_ScreensRunning()

Devuelve si la aplicación debe continuar ejecutándose.

Mientras esta función devuelva `true`, el juego seguirá funcionando.

Cuando el usuario cierre la ventana o la aplicación finalice, devolverá `false`.

---

## S2S_BeginFrame()

Marca el comienzo de un nuevo frame.

Toda la lógica del juego y las operaciones de dibujo deben realizarse entre `S2S_BeginFrame()` y `S2S_EndFrame()`.

---

## S2S_EndFrame()

Finaliza el frame actual.

Esta función presenta la imagen en pantalla y prepara el siguiente frame.

---

## S2S_ScreensExit()

Libera todos los recursos utilizados por el sistema de pantallas.

Debe ejecutarse antes de terminar el programa.

---

## SetWindowTitle()

Únicamente como función de pc para asignar el nombre de la ventana con el que hay en game.json.

---


# Compilar el proyecto

Para generar la versión de Windows:

```bat
build.bat
```

Para generar la versión de Nintendo 3DS:

```bat
build.bat 3ds
```

No es necesario modificar el código fuente para cambiar de plataforma.

---

# Próximo paso

Una vez que el proyecto básico funciona correctamente, puedes continuar con [**03 - Graphics**](03-Graphics.md), donde aprenderás a dibujar:

* Rectángulos
* Triángulos
* Elipses
* Rectángulos con borde
* Líneas
* Puntos
* Texto
* Sprites
* Y el resto de primitivas gráficas disponibles en Tridingine-3DSlib.
