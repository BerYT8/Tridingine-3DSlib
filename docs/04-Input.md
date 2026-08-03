# Entrada (Input)

El módulo **Input** proporciona una capa de abstracción para la entrada del usuario, permitiendo utilizar los mismos botones independientemente de la plataforma.

Actualmente soporta:

- Nintendo 3DS
- PC (SDL)

La traducción entre los botones físicos y los botones del motor se realiza automáticamente.

---

# Inicialización

Antes de utilizar el sistema de entrada:

```c
input_init();
```

Cada frame debe actualizarse el estado de la entrada:

```c
input_read();
```

Al finalizar la aplicación:

```c
input_exit();
```

---

# Botones virtuales

El motor utiliza un conjunto de botones abstractos comunes entre todas las plataformas.

## Botones principales

```c
INPUT_KEY_A
INPUT_KEY_B
INPUT_KEY_X
INPUT_KEY_Y
```

---

## Gatillos

```c
INPUT_KEY_L
INPUT_KEY_R

INPUT_KEY_ZL
INPUT_KEY_ZR
```

---

## Sistema

```c
INPUT_KEY_START
INPUT_KEY_SELECT
```

---

## Cruceta (D-Pad)

```c
INPUT_KEY_DUP
INPUT_KEY_DDOWN
INPUT_KEY_DLEFT
INPUT_KEY_DRIGHT
```

---

## Circle Pad

```c
INPUT_KEY_CPAD_UP
INPUT_KEY_CPAD_DOWN
INPUT_KEY_CPAD_LEFT
INPUT_KEY_CPAD_RIGHT
```

---

## C-Stick

```c
INPUT_KEY_CSTICK_UP
INPUT_KEY_CSTICK_DOWN
INPUT_KEY_CSTICK_LEFT
INPUT_KEY_CSTICK_RIGHT
```

---

## Pantalla táctil

```c
INPUT_KEY_TOUCH
```

---

## Especiales

Detectar cualquier botón:

```c
INPUT_KEY_ANY
```

Sin botón válido:

```c
INPUT_KEY_NONE
```

---

# Consultar botones

## Pulsado este frame

Devuelve `true` únicamente durante el frame en el que el botón fue presionado.

```c
if (input_isKeyPressed(INPUT_KEY_A))
{
    // ...
}
```

---

## Liberado este frame

```c
if (input_isKeyReleased(INPUT_KEY_A))
{
    // ...
}
```

---

## Mantenido

Devuelve `true` mientras el botón permanezca presionado.

```c
if (input_isKeyDown(INPUT_KEY_A))
{
    // ...
}
```

---

## No pulsado

```c
if (input_isKeyUp(INPUT_KEY_A))
{
    // ...
}
```

---

# Reasignar botones

Es posible asociar un botón físico de la plataforma a un botón virtual del motor.

```c
input_bindKey(
    INPUT_KEY_A,
    position,
    platformKey
);
```

Esto permite personalizar los controles sin modificar el resto del código del juego.

---

# Entrada táctil

Obtener la posición actual del toque:

```c
Vec2 touch = input_getTouch();
```

El valor devuelto corresponde a las coordenadas de la pantalla táctil de la plataforma.

---

# Botones del ratón (PC)

En la versión para PC también existen constantes para los botones del ratón.

Botón izquierdo:

```c
PC_MOUSE_LEFT_BUTTON
```

Botón derecho:

```c
PC_MOUSE_RIGHT_BUTTON
```

Ambos botones:

```c
PC_MOUSE_BOTH_BUTTONS
```

Estas constantes solo están disponibles cuando se compila para la plataforma PC.

---

# Flujo típico de uso

```c
input_init();

while (S2S_ScreensRunning())
{
    input_read();

    if (input_isKeyPressed(INPUT_KEY_START))
    {
        // Abrir menú
    }

    if (input_isKeyDown(INPUT_KEY_A))
    {
        // Acción continua
    }
}

input_exit();
```

---

# Próximo paso

Ya puedes recibir entrada del usuario e interactuar con el juego.

Continúa con [**05 - Audio**](05-Audio.md), donde aprenderás a:

- Inicializar el sistema de audio.
- Reproducir efectos de sonido.
- Reproducir música.
- Controlar el volumen y la reproducción.