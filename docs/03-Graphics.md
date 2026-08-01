# Dibujar en pantalla

El motor incluye dos APIs de renderizado:

- **D2D**: renderizado 2D para interfaces, texto y primitivas.
- **D3D**: renderizado 3D para modelos y primitivas básicas.

> **⚠️ Estado actual**
>
> La API **D3D** aún se encuentra en desarrollo. Actualmente es **muy poco confiable**, la interfaz puede cambiar sin previo aviso y **no se recomienda utilizarla en proyectos importantes**.

---

# Renderizado 2D (D2D)

La API `D2D` permite dibujar primitivas, texto y elementos de interfaz directamente en pantalla.

## Inicialización

Antes de utilizar cualquier función de dibujo es necesario inicializar el sistema.

```c
if (!D2D_Init())
{
    // Error
}
```

Una vez inicializado el módulo, debe prepararse antes de comenzar a dibujar:

```c
D2D_Prepare();
```

> `D2D_Prepare()` se llama **una única vez después de `D2D_Init()`**, no una vez por cada frame.

Al finalizar la aplicación:

```c
D2D_Exit();
```

---

# Rotación

Algunas primitivas aceptan un parámetro `rotation`.

La API también proporciona utilidades para trabajar con rotaciones:

```c
float rotation = D2D_ValueIsRotation(45.0f);

D2D_AddRotation(&rotation, 15.0f);
```

---

# Punto

Dibuja un punto.

```c
D2D_DrawPoint(
    x,
    y,
    rotation,
    depth,
    thickness,
    color
);
```

---

# Línea

Permite dibujar una línea con color independiente para cada extremo.

```c
D2D_DrawLine(
    x0, y0, colorA,
    x1, y1, colorB,
    thickness,
    rotation,
    depth,
    align
);
```

---

# Rectángulos

## Rectángulo con gradiente

Cada esquina puede tener un color diferente.

```c
D2D_DrawRectangle(
    x,
    y,
    width,
    height,
    rotation,
    depth,
    alignX,
    alignY,
    topLeft,
    topRight,
    bottomRight,
    bottomLeft
);
```

## Rectángulo sólido

```c
D2D_DrawRectSolid(
    x,
    y,
    width,
    height,
    rotation,
    depth,
    alignX,
    alignY,
    color
);
```

---

# Rectángulos con borde

## Con gradiente

```c
D2D_DrawBorderedRect(
    x,
    y,
    width,
    height,
    radius,
    depth,
    alignX,
    alignY,
    c1,
    c2,
    c3,
    c4
);
```

## Color sólido

```c
D2D_DrawBorderedRectSolid(
    x,
    y,
    width,
    height,
    radius,
    depth,
    alignX,
    alignY,
    color
);
```

---

# Triángulos

## Con colores por vértice

```c
D2D_DrawTriangle(
    x0, y0, c0,
    x1, y1, c1,
    x2, y2, c2,
    rotation,
    depth,
    alignX,
    alignY
);
```

## Color sólido

```c
D2D_DrawTriangleSolid(
    x0, y0,
    x1, y1,
    x2, y2,
    rotation,
    depth,
    alignX,
    alignY,
    color
);
```

---

# Elipses y círculos

## Elipse

```c
D2D_DrawEllipse(
    x,
    y,
    radiusX,
    radiusY,
    rotation,
    depth,
    alignX,
    alignY,
    c0,
    c1,
    c2,
    c3
);
```

## Elipse sólida

```c
D2D_DrawEllipseSolid(
    x,
    y,
    radiusX,
    radiusY,
    rotation,
    depth,
    alignX,
    alignY,
    color
);
```

## Círculo

```c
D2D_DrawCircle(
    x,
    y,
    radius,
    rotation,
    depth,
    alignX,
    alignY,
    c0,
    c1,
    c2,
    c3
);
```

## Círculo sólido

```c
D2D_DrawCircleSolid(
    x,
    y,
    radius,
    rotation,
    depth,
    alignX,
    alignY,
    color
);
```

---

# Fuentes

## Abrir una fuente

```c
D2D_Font* font = D2D_OpenFont("fonts/Roboto.ttf");
```

## Liberarla

```c
D2D_CloseFont(font);
```

---

# Texto

```c
D2D_DrawText(
    "Hola mundo",
    font,
    32.0f,
    WHITE,

    100,
    50,
    0,

    400,
    200,

    0.5f,
    0.5f,

    0,
    0,

    WORD_WRAP_MODE
);
```

Parámetros principales:

| Parámetro | Descripción |
|-----------|-------------|
| `font` | Fuente utilizada. |
| `fontSize` | Tamaño de letra. |
| `color` | Color del texto. |
| `x`, `y` | Posición. |
| `depth` | Profundidad. |
| `w`, `h` | Área de dibujo. |
| `alignX` | Alineación horizontal. |
| `alignY` | Alineación vertical. |
| `letterSpacing` | Espaciado entre letras. |
| `lineSpacing` | Espaciado entre líneas. |
| `wrap` | Tipo de ajuste de línea. |

### Modos de ajuste

```c
LETTER_WRAP_MODE
```

Ajuste por caracteres.

```c
WORD_WRAP_MODE
```

Ajuste por palabras.

```c
WRAP_NONE
```

Sin ajuste.

---

# Renderizado 3D (D3D)

> **⚠️ API experimental**
>
> Esta API está en una etapa muy temprana de desarrollo.
>
> Actualmente:
>
> - Puede contener errores importantes.
> - La estabilidad no está garantizada.
> - La interfaz puede cambiar en cualquier momento.
> - No se recomienda su uso fuera de pruebas.

---

## Inicialización

```c
if (!D3D_Init())
{
    // Error
}
```

Después de inicializar el módulo:

```c
D3D_Prepare();
```

> `D3D_Prepare()` se ejecuta **una única vez tras `D3D_Init()`** para preparar el sistema de renderizado.

Al cerrar la aplicación:

```c
D3D_Exit();
```

---

# Cámara

La cámara está definida como:

```c
Camera3D camera;

camera.pos = ...;
camera.rot = ...;
camera.fov = 70.0f;
```

---

# Movimiento

Mover hacia delante:

```c
D3D_AddFront(&camera, 5.0f);
```

Mover hacia la derecha:

```c
D3D_AddRight(&camera, 5.0f);
```

Mover hacia arriba:

```c
D3D_AddUp(&camera, 2.0f);
```

También existen funciones para mover utilizando el vector frontal.

```c
D3D_AddForwardPos(&camera, movement);
```

---

# Dirección de la cámara

Obtener los vectores principales:

```c
Vec3 forward = D3D_Camera_GetForward(&camera);

Vec3 right = D3D_Camera_GetRight(&camera);

Vec3 up = D3D_Camera_GetUp(&camera);
```

---

# Conversión de direcciones

Mirar hacia un punto:

```c
Vec3 rot = D3D_LookAt(origen, destino);
```

Dirección → Rotación

```c
Vec3 rot = D3D_ForwardToRotation(forward);
```

Rotación → Dirección

```c
Vec3 forward = D3D_RotationToForward(rot);
```

---

# Configuración de cámara

```c
D3D_SetCameraPos(&camera, pos);

D3D_SetCameraRot(&camera, rot);

D3D_SetCameraFov(&camera, 70.0f);
```

Consultar valores:

```c
Vec3 pos = D3D_GetCameraPos(&camera);

Vec3 rot = D3D_GetCameraRot(&camera);

float fov = D3D_GetCameraFov(&camera);
```

---

# Modelos

## Cargar

```c
Model3D* model = D3D_LoadModel3D("assets/tree.obj");
```

## Dibujar

```c
D3D_DrawModel(
    model,
    position,
    rotation,
    scale,
    material,
    &camera
);
```

---

# Primitivas 3D

## Cuboide

```c
D3D_DrawCuboid(
    position,
    rotation,
    scale,
    center,
    material,
    &camera
);
```

## Esfera

```c
D3D_DrawSphere(
    position,
    rotation,
    scale,
    center,
    material,
    &camera
);
```

---

# Estado de desarrollo

Actualmente el módulo **D2D** puede utilizarse para interfaces y renderizado 2D de forma estable.

El módulo **D3D** continúa en desarrollo activo y todavía no debe considerarse una API final. Es posible que funciones, estructuras y comportamientos cambien en futuras versiones sin mantener compatibilidad con versiones anteriores.

---

# Próximo paso

Ahora que conoces cómo dibujar en pantalla, el siguiente paso es aprender a gestionar la entrada del usuario.

Continúa con [**04 - Input**](04-Input.md), donde aprenderás a:

- Leer botones.
- Detectar pulsaciones y liberaciones.
- Consultar el estado de los controles.
- Obtener la posición de la pantalla táctil.
- Reasignar botones físicos a botones virtuales.