# Tridingine-3DSlib

**Tridingine-3DSlib** es una biblioteca que permite desarrollar juegos utilizando una API limpia y sencilla, escribiendo prácticamente el mismo código para **PC** y **Nintendo 3DS**.

Su objetivo es ofrecer una experiencia de desarrollo multiplataforma donde el juego mantenga el mismo aspecto y comportamiento en ambas plataformas.

> **Estado:** En desarrollo.

---

# Características

- API limpia y fácil de utilizar.
- Diseñada principalmente para proyectos en **C** con mejoras para **C++**.
- Mismo código fuente para PC y Nintendo 3DS.
- Resultado visual y funcional prácticamente idéntico en ambas plataformas.
- Pensada para crear juegos de forma rápida y sencilla.

---

# Compilar la librería

Desde la raíz del proyecto ejecuta:

```bat
.\build.bat
```

Una vez finalice la compilación, encontrarás todo el kit necesario para desarrollar juegos en:

```text
build/
└── Lib/
```

Esta carpeta contiene la biblioteca, archivos de cabecera y el resto de componentes necesarios para crear proyectos con Tridingine-3DSlib.

---

# Documentación

La documentación está organizada en varios archivos independientes.

- [01 - Instalación](docs/01-Installation.md)
- [02 - Primer proyecto](docs/02-Getting-Started.md)
- [03 - Gráficos](docs/03-Graphics.md)
- [04 - Input](docs/04-Input.md)
- [05 - Audio](docs/05-Audio.md)
- [06 - Recursos](docs/06-Resources.md)
- [07 - Compilación para PC](docs/07-PC.md)
- [08 - Compilación para Nintendo 3DS](docs/08-3DS.md)
- [09 - Ejemplos](docs/09-Examples.md)

> Parte de la documentación puede estar pendiente de publicarse.

---

# Filosofía

Tridingine-3DSlib está diseñada para que puedas:

- Escribir el juego una sola vez.
- Utilizar una única API para todas las plataformas soportadas.
- Compilar el mismo proyecto para PC y Nintendo 3DS.
- Obtener un comportamiento y una apariencia consistentes.

---

# Licencia

Este proyecto está distribuido bajo la **Apache License 2.0**. Consulta el archivo `LICENSE` para más información.