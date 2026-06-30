# Instalación

En este documento se explica cómo compilar **Tridingine-3DSlib**, generar el kit de desarrollo y crear proyectos listos para usar.

---

# Requisitos

Actualmente, **Tridingine-3DSlib** está desarrollada principalmente para **Windows**, aunque la biblioteca ha sido diseñada para ser multiplataforma.

En futuras versiones se ofrecerá soporte completo para:

- Windows
- Linux
- macOS

utilizando la misma API y la misma estructura de proyecto.

---

# Software necesario

Antes de compilar la biblioteca es necesario instalar las siguientes herramientas.

## DevkitPro

Instala **DevkitPro** utilizando el instalador oficial.

La ruta de instalación debe ser la predeterminada:

```text
C:\devkitPro
```

Esta dependencia es necesaria para poder compilar proyectos destinados a **Nintendo 3DS**.

---

## CMake

Instala la versión más reciente de **CMake** y asegúrate de que puede ejecutarse desde la línea de comandos.

---

# Compilar Tridingine-3DSlib

Desde la carpeta raíz del repositorio ejecuta:

```bat
.\build.bat
```

Este comando compilará la biblioteca para Windows y generará el kit de desarrollo.

---

# Carpetas generadas

Al finalizar la compilación se generará la carpeta:

```text
build/
```

Dentro de ella encontrarás:

## Code/

Contiene los archivos generados para la compilación en Windows.

## Lib/

Contiene todo el kit de desarrollo necesario para crear juegos con Tridingine-3DSlib.

---

# Compilar para Nintendo 3DS

Para generar la versión destinada a Nintendo 3DS ejecuta:

```bat
.\build.bat 3ds
```

En este caso la salida se almacenará en:

```text
build_3ds/
```

en lugar de `build/`.

---

# Estructura del kit de desarrollo

Después de compilar, la carpeta `Lib` tendrá una estructura similar a la siguiente:

```text
Lib/
├── include/
├── lib/
├── templates/
├── examples/
├── tools/
├── content/
├── romfs/
├── ProjectMaker.exe
├── build.bat
├── CMakeLists.txt
├── game.json
└── main.cpp
```

---

## include/

Archivos de cabecera públicos de la biblioteca.

---

## lib/

Bibliotecas compiladas necesarias para enlazar el proyecto.

---

## templates/

Plantillas internas para la creación de proyectos.

---

## examples/

Ejemplos de uso de la API.

---

## tools/

Herramientas internas utilizadas por la biblioteca.

---

## content/

En esta carpeta deben colocarse todos los recursos del juego.

Por ejemplo:

- Texturas
- Audio
- Fuentes
- Archivos de configuración
- Shaders
- Cualquier otro recurso necesario durante la ejecución

Todo el contenido de esta carpeta se empaquetará automáticamente con el juego .

---

## romfs/

En esta carpeta encontrarás dos subcarpetas con los nombres **pc** y **3ds**.

Estas serán las que contengan los archivos compilados de content para cada plataforma.

---

## ProjectMaker.exe

Herramienta para crear proyectos vacíos basados en el kit de desarrollo.

Permite generar una estructura limpia y lista para empezar a desarrollar.

### Uso

```bat
ProjectMaker.exe -o [carpeta]
```

### Parámetro

- **-o [carpeta]**: carpeta de salida donde se generará el proyecto.

### Resultado

El programa generará un proyecto completamente limpio con la estructura necesaria para comenzar a desarrollar directamente con **Tridingine-3DSlib**, incluyendo:

- Archivos base del proyecto
- Estructura lista para compilar
- Configuración inicial preparada

---

# Compilar un proyecto

El kit generado ya incluye todo lo necesario para desarrollar un juego.

Para compilar la versión de Windows ejecuta:

```bat
.\build.bat
```

Para compilar la versión de Nintendo 3DS ejecuta:

```bat
.\build.bat 3ds
```

No es necesario modificar el código fuente del juego para cambiar de plataforma.

---

# CMakeLists.txt

El archivo `CMakeLists.txt` puede modificarse libremente.

Su principal objetivo es permitir añadir nuevos archivos fuente o carpetas al proyecto editando la lista de *sources*.

---

# game.json

El archivo `game.json` contiene la información utilizada para generar el archivo `.3dsx`.

Ejemplo:

```json
{
    "file": "name_of_file",
    "title": "Game Title",
    "author": "You",
    "description": "Description."
}
```

Campos disponibles:

- **file**: Nombre del archivo ejecutable.
- **title**: Título del juego.
- **author**: Autor del proyecto.
- **description**: Descripción mostrada por el Homebrew Launcher.

En futuras versiones se añadirá soporte para personalizar la carátula y otros metadatos del ejecutable.

---

# Siguiente paso

Una vez generado un proyecto con `ProjectMaker.exe`, continúa con:

**[02 - Primer proyecto](02-Getting-Started.md)** para crear tu primer juego utilizando **Tridingine-3DSlib**.