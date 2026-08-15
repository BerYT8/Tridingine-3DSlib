# Drawing on Screen

The engine includes two rendering APIs:

- **D2D**: 2D rendering for interfaces, text, and primitives.
- **D3D**: 3D rendering for models and basic primitives.

> **⚠️ Current Status**
>
> The **D3D** API is still under development. It is currently **very unreliable**, its interface may change without notice, and **it is not recommended for use in important projects**.

---

# 2D Rendering (D2D)

The `D2D` API allows you to draw primitives, text, and interface elements directly on screen.

## Initialization

Before using any drawing function, the system must be initialized.

```c
if (!D2D_Init())
{
    // Error
}
```

Once the module has been initialized, it must be prepared before drawing:

```c
D2D_Prepare();
```

> `D2D_Prepare()` is called **only once after `D2D_Init()`**.

When the application exits:

```c
D2D_Exit();
```

---

# Rotation

Some primitives accept a `rotation` parameter.

The API also provides utilities for working with rotations:

```c
float rotation = D2D_ValueIsRotation(395.0f); // Returns 35.0f
```

Returns a value between `0.0f` and `360.0f` from a float value.

```c
D2D_AddRotation(&rotation, 15.0f);
```

Adds the specified rotation value while keeping the result between `0.0f` and `360.0f`.

---

# Point

Draws a point.

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

# Line

Draws a line with an independent color for each endpoint.

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

# Rectangles

## Gradient Rectangle

Each corner can have a different color.

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

## Solid Rectangle

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

# Bordered Rectangles

## Gradient

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

## Solid Color

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

# Triangles

## Per-Vertex Colors

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

## Solid Color

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

# Ellipses and Circles

## Ellipse

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

## Solid Ellipse

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

## Circle

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

## Solid Circle

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

# Fonts

## Open a Font

```c
D2D_Font* font = D2D_OpenFont("engine/fonts/NotoSansMono");
```

## Close a Font

```c
D2D_CloseFont(font);
```

---

# Text

```c
D2D_DrawText(
    "Hello world",
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

    0.5f,
    0.5f,

    0,
    0,

    WORD_WRAP_MODE
);
```

Main parameters:

| Parameter | Description |
|-----------|-------------|
| `font` | Font to use. |
| `fontSize` | Font size. |
| `color` | Text color. |
| `x`, `y` | Position. |
| `depth` | Depth. |
| `w`, `h` | Drawing area. |
| `alignX` | Horizontal alignment. |
| `alignY` | Vertical alignment. |
| `textAlignX` | Horizontal text alignment. |
| `textAlignY` | Vertical text alignment. |
| `letterSpacing` | Spacing between letters. |
| `lineSpacing` | Spacing between lines. |
| `wrap` | Line wrapping mode. |

### Wrapping Modes

```c
LETTER_WRAP_MODE
```

Wraps text by characters.

```c
WORD_WRAP_MODE
```

Wraps text by words.

```c
WRAP_NONE
```

No wrapping.

---

# 3D Rendering (D3D)

> **⚠️ Experimental API**
>
> This API is still in a very early stage of development.
>
> Currently:
>
> - It may contain significant bugs.
> - Stability is not guaranteed.
> - The interface may change at any time.
> - It is not recommended for anything beyond testing.

---

## Initialization

```c
if (!D3D_Init())
{
    // Error
}
```

After initializing the module:

```c
D3D_Prepare();
```

> `D3D_Prepare()` is executed **only once after `D3D_Init()`** to prepare the rendering system.

When the application exits:

```c
D3D_Exit();
```

---

# Camera

The camera is defined as:

```c
Camera3D camera;

camera.pos = ...;
camera.rot = ...;
camera.fov = 70.0f;
```

---

# Movement

Move forward:

```c
D3D_AddFront(&camera, 5.0f);
```

Move right:

```c
D3D_AddRight(&camera, 5.0f);
```

Move upward:

```c
D3D_AddUp(&camera, 2.0f);
```

There are also functions for movement using the forward vector.

```c
D3D_AddForwardPos(&camera, movement);
```

---

# Camera Direction

Get the main direction vectors:

```c
Vec3 forward = D3D_Camera_GetForward(&camera);

Vec3 right = D3D_Camera_GetRight(&camera);

Vec3 up = D3D_Camera_GetUp(&camera);
```

---

# Direction Conversion

Look at a point:

```c
Vec3 rot = D3D_LookAt(origen, destino);
```

Direction → Rotation

```c
Vec3 rot = D3D_ForwardToRotation(forward);
```

Rotation → Direction

```c
Vec3 forward = D3D_RotationToForward(rot);
```

---

# Camera Configuration

```c
D3D_SetCameraPos(&camera, pos);

D3D_SetCameraRot(&camera, rot);

D3D_SetCameraFov(&camera, 70.0f);
```

Get values:

```c
Vec3 pos = D3D_GetCameraPos(&camera);

Vec3 rot = D3D_GetCameraRot(&camera);

float fov = D3D_GetCameraFov(&camera);
```

---

# Models

## Load

```c
Model3D* model = D3D_LoadModel3D("assets/tree.obj");
```

## Draw

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

# 3D Primitives

## Cuboid

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

## Sphere

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

# Development Status

The **D2D** module can currently be used for interfaces and stable 2D rendering.

The **D3D** module is still under active development and should not yet be considered a final API. Functions, structures, and behavior may change in future versions without maintaining backward compatibility.

---

# Next Step

Now that you know how to draw on screen, the next step is learning how to handle user input.

Continue with [**04 - Input**](04-Input.md), where you will learn how to:

- Read buttons.
- Detect button presses and releases.
- Query controller states.
- Get the touchscreen position.
- Remap physical buttons to virtual buttons.