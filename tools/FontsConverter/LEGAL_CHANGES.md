# Cambios realizados y cumplimiento de la licencia

Este directorio deriva del proyecto original de conversión de fuentes de `tex3ds` y conserva la licencia **GNU GPL v3 o posterior (GPL-3.0-or-later)** aplicable al código original.

Los cambios descritos a continuación constituyen modificaciones del código base y se distribuyen respetando las condiciones de la GPL aplicables a las obras derivadas.

## 1. Origen y licencia

* El código original procede del proyecto de conversión de fuentes de `tex3ds`.
* El proyecto original está distribuido bajo **GNU GPL v3 o posterior (GPL-3.0-or-later)**.
* Se conserva el archivo `COPYING` junto al código.
* Se mantienen los avisos de copyright, licencia y atribución presentes en los archivos originales, cuando corresponda.
* Los archivos que constituyen modificaciones o partes derivadas del código original permanecen sujetos a las condiciones de la GPL aplicables a dichas obras.

## 2. Cambios realizados

Se han realizado los siguientes cambios en el convertidor de fuentes con el objetivo de eliminar dependencias no deseadas y mantener la implementación basada en SDL2:

* Sustitución de la lógica basada en ImageMagick por operaciones sobre superficies SDL2.
* Eliminación de la compatibilidad antigua con Magick++ y de la correspondiente capa de adaptación histórica.
* Corrección de un problema de compilación en la cola de tareas mediante la inclusión de la cabecera necesaria para `assert`.
* Mantenimiento del flujo de generación de BCFNT y del procesamiento de alpha utilizando SDL2, en lugar de `PixelPacket`/`Magick::Quantum`.
* Eliminación de referencias obsoletas a Magick dentro del árbol activo del convertidor.

## 3. Cumplimiento de la licencia

La distribución de estos cambios se realiza teniendo en cuenta las obligaciones aplicables de la GPL:

* Se conserva el texto de la licencia GPL en `COPYING`.
* No se eliminan deliberadamente los avisos de copyright, licencia o atribución del código original.
* Las modificaciones se identifican y documentan en este archivo.
* El código fuente de la obra modificada se mantiene disponible.
* Las partes derivadas del código original continúan distribuyéndose bajo las condiciones de la GPL aplicables.
* No se pretende utilizar estas modificaciones para restringir los derechos de copia, modificación o redistribución concedidos por la GPL.

## 4. Archivos y código de terceros

Cuando un archivo o componente procede de otro proyecto, se mantienen sus correspondientes avisos de copyright y licencia.

La presencia de código de terceros dentro de este árbol no implica que dicho código sea originalmente obra de este proyecto. Las licencias y atribuciones correspondientes deben conservarse de acuerdo con sus respectivos términos.

## 5. Distribución

Cuando este proyecto o una obra derivada se distribuya a terceros, deberán cumplirse las condiciones de la GPL aplicables a la forma de distribución utilizada, incluyendo, cuando corresponda, la entrega o puesta a disposición del código fuente correspondiente y de los avisos y términos de licencia requeridos.

## 6. Nota

Este documento tiene carácter informativo y técnico. Su finalidad es documentar el origen del código, las modificaciones realizadas y las medidas adoptadas para mantener la continuidad de las licencias aplicables.

No constituye asesoramiento jurídico profesional.
