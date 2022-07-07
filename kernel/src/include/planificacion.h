#ifndef TP_2022_1C_ECLIPSO_PLANIFICACION_H
#define TP_2022_1C_ECLIPSO_PLANIFICACION_H

#include "../../../shared/headers/sharedUtils.h"
#include <time.h>
#include <stdbool.h>
#include <stdio.h>

t_list* READY;
t_queue* NEW;
t_queue* BLOCKED;
t_list* SUSPENDED_READY;
t_list* SUSPENDED_BLOCKED;

unsigned int GRADO_MULTIPROGRAMACION;
unsigned int TIEMPO_MAXIMO_BLOQUEADO;
sem_t semGradoMultiprogramacion;
pthread_mutex_t mutexColaNew;
pthread_mutex_t mutexColaReady;
pthread_mutex_t mutexColaBloqueados;
pthread_mutex_t mutexColaSuspendedBloqued;
pthread_mutex_t mutex_cola_suspended_ready;
pthread_mutex_t  mutexGradoMultiprogramacion;
int valorSemaforoContador;
uint32_t tiempo_max_bloqueo;
int conexionCPUDispatch, conexionCPUInterrupt;

void iniciarPlanificacionCortoPlazo(t_attrs_planificacion*);
void iniciarPlanificacion(t_attrs_planificacion*);
int inicializarMutex();
void avisarProcesoTerminado(int);
void bloquearProceso(t_pcb*);
void void suspender_proceso(t_pcb* pcb);
void estimar_proxima_rafaga(time_t, t_pcb*);
time_t calcular_tiempo_en_exec(time_t); 
void calcular_rafagas_restantes_proceso_desalojado(time_t, time_t, t_pcb*);
void ordenar_procesos_lista_READY();
void checkear_proceso_y_replanificar(t_pcb*);
void replanificar_y_enviar_nuevo_proceso(t_pcb*, t_pcb*);
void planificacion_SJF(t_pcb*);
void agregar_proceso_READY(t_pcb*);
void iniciar_algoritmo_planificacion(char*, t_pcb*);
t_pcb* obtener_proceso_en_READY();
static bool sort_by_rafaga(void*, void*);
void planificacion_FIFO(t_pcb*);
void incrementar_grado_multiprogramacion();
void decrementar_grado_multiprogramacion();
void eliminar_proceso_de_READY();
unsigned int tiempo_en_suspended_blocked(t_pcb*);
void enviar_proceso_SUSPENDED_READY_a_READY();
void agregar_proceso_SUSPENDED_READY(t_pcb*);

#endif //TP_2022_1C_ECLIPSO_PLANIFICACION_H
