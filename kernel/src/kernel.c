#include <semaphore.h>
#include "kernel.h"

//Variables globales
t_log* logger;
int kernel_fd;
t_queue * colaProcesosNew;
t_config * config;

t_queue * colaProcesosReady;
uint32_t gradoMultiprogramacion;
sem_t semGradoMultiprogramacion;

void sighandler(int s) {
    cerrar_programa(logger);
    exit(0);
}

int main() {
    signal(SIGINT, sighandler);

 //   sem_init(&semGradoMultiprogramacion,0,gradoMultiprogramacion);
    logger = log_create("kernel.log", "KERNEL", 1, LOG_LEVEL_DEBUG);
    config = iniciarConfig(CONFIG_FILE);

    char* ip= config_get_string_value(config,"IP_MEMORIA");
    char* puerto= config_get_string_value(config,"PUERTO_MEMORIA");

    kernel_fd = iniciar_kernel(ip,puerto);

    ip= config_get_string_value(config,"IP_CPU");
    puerto= config_get_string_value(config,"PUERTO_ESCUCHA");
    kernel_fd = iniciar_kernel(ip,puerto);

	log_info(logger, "Kernel listo para recibir una consola");
    colaProcesosNew = queue_create();
    colaProcesosReady = queue_create();
    while (escuchar_consolas(logger, "KERNEL", kernel_fd));

    //cerrar_programa(logger);

    return 0;
}

void cerrar_programa(t_log* logger) {
	log_destroy(logger);
}

static void procesar_conexion(void* void_args) {
	t_procesar_conexion_attrs* attrs = (t_procesar_conexion_attrs*) void_args;
	t_log* logger = attrs->log;
    int consola_fd = attrs->fd;
    free(attrs);

    op_code cop;
    t_list* listaInstrucciones = list_create();

    while (consola_fd != -1) {

		op_code cod_op = recibirOperacion(consola_fd);
		switch (cod_op) {
			case MENSAJE:
                recibirMensaje(consola_fd, logger);
				break;
			case LISTA_INSTRUCCIONES:
			    listaInstrucciones = recibirListaInstrucciones(consola_fd);
                int tamanioProceso = recibirTamanioProceso(consola_fd);
                t_pcb* pcb = crearEstructuraPcb(listaInstrucciones, tamanioProceso);
           //     printf("\nPCB->idProceso:%d\n", pcb->idProceso);
            //    printf("\nPCB->tamanioProceso:%d\n", pcb->tamanioProceso);
            /*    for(uint32_t i=0; i<list_size(listaInstrucciones); i++){
                    t_instruccion *instruccion = list_get(pcb->listaInstrucciones,i);
                    printf("\nEN EL PCB\ninstruccion-->codigoInstruccion->%d\toperando1->%d\toperando2->%d\n",
                           instruccion->codigo_operacion,
                           instruccion->parametros[0],
                           instruccion->parametros[1]);
                }*/
               // printf("PCB->programCounter:%d", pcb->programCounter);
                queue_push(colaProcesosNew, pcb);
              //  sem_wait(&semGradoMultiprogramacion);
                queue_push(colaProcesosReady, queue_pop(colaProcesosNew));

           //     sem_post(&semGradoMultiprogramacion);
                printf("\n\ntamanio de la cola de procesos en ready: %d\n\n", queue_size(colaProcesosReady));
                enviarMensaje("Recibí la lista de instrucciones. Termino de ejecutar y te aviso", consola_fd);
                //TODO enviar a la consla que se recibió correctamente la lista de instrucciones por ahora
                break;
			case -1:
				log_info(logger, "La consola se desconecto.");
				consola_fd = -1;
				break;
			default:
				log_warning(logger,"Operacion desconocida.");
				break;
			}
	}
}

int recibir_opcion() {
	char* opcion = malloc(4);
	log_info(logger, "Ingrese una opcion: ");
	scanf("%s",opcion);
	int op = atoi(opcion);
	free(opcion);
	return op;
}

int validar_y_ejecutar_opcion_consola(int opcion, int consola_fd, int kernel_fd) {

	int continuar = 1;

	switch(opcion) {
		case 1:
			log_info(logger,"kernel continua corriendo, esperando nueva consola.");
			consola_fd = esperarConsola(kernel_fd);
			break;
		case 0:
			log_info(logger,"Terminando kernel...");
			close(kernel_fd);
			continuar = 0;
			break;
		default:
			log_error(logger,"Opcion invalida. Volve a intentarlo");
			recibir_opcion();
	}

	return continuar;

}


int accion_kernel(int consola_fd, int kernel_fd) {

	log_info(logger, "¿Desea mantener el kernel corriendo? 1- Si 0- No");
	int opcion = recibir_opcion();

	if (recibirOperacion(consola_fd) == SIN_CONSOLAS) {
	  log_info(logger, "No hay mas clientes conectados.");
	  log_info(logger, "¿Desea mantener el kernel corriendo? 1- Si 0- No");
	  opcion = recibir_opcion();
	  return validar_y_ejecutar_opcion_consola(opcion, consola_fd, kernel_fd);
	}

	return validar_y_ejecutar_opcion_consola(opcion, consola_fd, kernel_fd);

}

int escuchar_consolas(t_log* logger, char* nombre_kernel, int kernel_fd) {
    int consola_fd = esperarConsola(kernel_fd);
    if (consola_fd != -1) {
        pthread_t hilo;
        t_procesar_conexion_attrs* attrs = malloc(sizeof(t_procesar_conexion_attrs));
        attrs->log = logger;
        attrs->fd = consola_fd;
        attrs->nombre_kernel = nombre_kernel;
        pthread_create(&hilo, NULL, (void*) procesar_conexion, (void*) attrs);
        pthread_detach(hilo);
        return 1;
    }
    return 0;
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}

t_pcb* crearEstructuraPcb(t_list* listaInstrucciones, int tamanioProceso) {

    t_pcb *pcb =  malloc(sizeof(t_pcb));
    t_instruccion *instruccion = list_get(listaInstrucciones,0);

    pcb->idProceso = process_get_thread_id();
    pcb->tamanioProceso = tamanioProceso;
    pcb->listaInstrucciones = listaInstrucciones;
    pcb->programCounter= instruccion->codigo_operacion;
    pcb->estimacionRafaga =1; // por ahora dejamos 1 como valor

}


