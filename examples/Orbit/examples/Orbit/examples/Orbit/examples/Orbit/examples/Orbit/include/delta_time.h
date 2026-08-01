#ifndef DELTA_TIME_H
#define DELTA_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

// Inicializa el sistema de tiempo
void dt_init(void);

// Actualiza el delta time
void dt_update(void);

// Devuelve el tiempo entre frames en segundos
double dt_get(void);

// Devuelve el tiempo total desde el inicio
double dt_total_time(void);

#ifdef __cplusplus
}
#endif

#endif