#ifndef SRC_CPU_H_
#define SRC_CPU_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include "../../shared/headers/sharedUtils.h"
#include "include/utils.h"

#define LOG_NAME "CPU_LOG"
#define LOG_FILE "cpu.log"
#define CONFIG_FILE "../cpu/src/config/cpu.config"


void comenzar_ciclo_instruccion();
t_instruccion* fase_fetch();
int fase_decode(t_instruccion*);
operando fase_fetch_operand(operando);
t_proceso_respuesta* fase_execute(t_instruccion* instruccion, uint32_t operador);
void operacion_NO_OP();
void operacion_IO(t_proceso_respuesta* proceso_respuesta, operando tiempo_bloqueo);
void operacion_EXIT(t_proceso_respuesta* proceso_respuesta);
void preparar_pcb_respuesta(t_paquete* paquete);

#endif /* SRC_CPU_H_ */
