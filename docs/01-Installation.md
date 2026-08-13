# Instalación

En este documento se explica cómo compilar **Tridingine-3DSlib**, generar el kit de desarrollo y crear proyectos listos para usar.

---

# Requisitos

Actualmente, **Tridingine-3DSlib** está diseñada principalmente para ejecutarse de forma nativa según tu sistema operativo. El flujo de compilación soporta:

- **Windows**: Ejecución nativa mediante archivos `.bat`.
- **Linux / macOS**: Ejecución nativa mediante archivos `.sh`.

utilizando la misma API y la misma estructura de proyecto.

---

# Software necesario

Antes de compilar la biblioteca es necesario instalar las siguientes herramientas.

## DevkitPro

Instala **DevkitPro** utilizando el instalador oficial.

La ruta de instalación predeterminada varía según el sistema:
- **Windows**: `C:\devkitPro`
- **Linux / macOS**: `/opt/devkitpro`

Esta dependencia es necesaria para poder compilar proyectos destinados a **Nintendo 3DS**.

---

## CMake

Instala la versión más reciente de **CMake** y asegúrate de que esté añadida al PATH de tu sistema para poder ejecutarla desde la línea de comandos.

---

# Compilar Tridingine-3DSlib (Paso 1)

El proceso de construcción consta de dos fases. En esta primera fase se compila **únicamente la librería principal** del motor para la plataforma elegida.

Desde la carpeta raíz del repositorio, ejecuta el comando correspondiente a tu sistema operativo:

### En Windows:
```bat
.\build.bat
```

### En Linux / macOS:
```bash
chmod +x ./build.sh
./build.sh
```

---

# Generar el Kit de Desarrollo (Paso 2)

Una vez compilada la librería principal en el Paso 1, **es obligatorio** ejecutar el script de post-construcción. Este script se encarga de compilar la herramienta `ProjectMaker`, todas las utilidades secundarias (`PakMaker`, `SoundMaker3DS`, etc.) y empaquetar la estructura final de la carpeta `Lib`.

Desde la carpeta raíz del repositorio, ejecuta:

### En Windows:
```bat
.\MakeProjectMaker.bat
```

### En Linux / macOS:
```bash
chmod +x ./MakeProjectMaker.sh
./MakeProjectMaker.sh
```

---

# Carpetas generadas

Al finalizar ambos scripts, encontrarás la siguiente estructura en la raíz:

## build/
Contiene los archivos temporales de configuración y objetos generados durante la compilación de PC.

## ProjectMaker.exe / ProjectMaker
El ejecutable de la herramienta de creación de proyectos se copiará directamente en la raíz de tu repositorio tras ejecutar el script del Paso 2.

## build/Lib/
Este es tu **Kit de Desarrollo** empaquetado y listo para usar. Contiene todo lo necesario para crear juegos con Tridingine-3DSlib.

---

# Estructura del kit de desarrollo (build/Lib/)

Después de completar con éxito el Paso 2 (`MakeProjectMaker`), la carpeta `build/Lib/` tendrá la siguiente estructura:

```text
build/Lib/
├── include/
├── lib/
│   ├── pc/
│   └── 3ds/
├── templates/
├── examples/
├── tools/
├── content/
└── romfs/
    ├── pc/
    └── 3ds/
```

---

## include/
Archivos de cabecera públicos (`.h` / `.hpp`) de la biblioteca.

---

## lib/
Bibliotecas estáticas compiladas necesarias para enlazar el proyecto (`.lib` para PC, `.a` para 3DS).

---

## templates/
Plantillas internas utilizadas por el entorno para la creación de proyectos.

---

## examples/
Ejemplos de uso de la API de Tridingine.

---

## tools/
Herramientas internas compiladas (como `PakMaker` o `SoundMaker3DS`) utilizadas por el ecosistema del motor.

---

## content/
En esta carpeta deben colocarse todos los recursos base del juego (Texturas, Audio, Fuentes, Shaders, etc.).

---

## romfs/
Contiene los recursos procesados y optimizados listos para ser consumidos por el ejecutable de cada plataforma (**pc** y **3ds**).

---

# Herramienta ProjectMaker

`ProjectMaker` es la utilidad incluida en la raíz para crear proyectos vacíos basados en el kit de desarrollo generado en `build/Lib/`.

### Uso

### En Windows:
```bat
ProjectMaker.exe -o [carpeta_de_salida]
```

### En Linux / macOS:
```bash
./ProjectMaker -o [carpeta_de_salida]
```

### Resultado
El programa generará un proyecto completamente limpio en el destino especificado con la estructura necesaria para comenzar a desarrollar directamente, incluyendo configuraciones de CMake base y scripts locales de construcción.

---

# Compilar un proyecto generado

Los proyectos creados con `ProjectMaker` son independientes y contienen sus propios scripts automáticos. Para compilar tu nuevo juego:

### Para la versión de PC:
- En Windows: `.\build.bat`
- En Linux: `./build.sh`

### Para la versión de Nintendo 3DS:
- En Windows: `.\build.bat 3ds`
- En Linux: `./build.sh build_3ds`

No es necesario modificar el código fuente del juego para cambiar de plataforma.

---

# Ejecutar directamente en Nintendo 3DS mediante `3dslink`

Los proyectos para Nintendo 3DS pueden enviarse directamente a una consola mediante **`3dslink`**, lo que permite compilar y probar el juego sin tener que copiar manualmente el archivo `.3dsx` a la tarjeta SD.

Esta función está disponible al añadir `link` a la llamada de compilación para 3DS.

## Requisitos

Para utilizar esta función necesitas:

- Tener **DevkitPro** instalado y `3dslink` disponible en el `PATH`.
- Tener la **Nintendo 3DS conectada a la misma red Wi-Fi que el PC**.
- Tener **Homebrew Launcher/Menu abierto en la 3DS**.
- Dentro de Homebrew Launcher/Menu, pulsar **`Y`** para activar el **Netloader**.
- Conocer la dirección IP de la 3DS si se quiere utilizar una conexión directa.

Cuando el Netloader está activo, la 3DS queda esperando una conexión desde el PC.

> **Importante:** la 3DS debe estar conectada a la misma red local que el PC. No es necesario realizar *port forwarding* ni abrir la consola a Internet.

---

# game.json

El archivo `game.json` de tu proyecto contiene la información utilizada por las herramientas para empaquetar el juego (especialmente el archivo `.3dsx` de la consola).

Ejemplo:
```json
{
    "file": "name_of_file",
    "title": "Game Title",
    "author": "You",
    "description": "Description."
}
```

---

# Siguiente paso

Una vez generado tu proyecto con `ProjectMaker`, continúa con:

**[02 - Primer proyecto](02-Getting-Started.md)** para crear tu primer juego utilizando **Tridingine-3DSlib**.
