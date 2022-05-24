#include "utils.h"

int iniciar_cpu(void) {
	int socket_cpu;

	struct addrinfo hints, *cpuinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(IP, PUERTO, &hints, &cpuinfo);

	// Creamos el socket de escucha de la cpu
	socket_cpu = socket(cpuinfo->ai_family,cpuinfo->ai_socktype,cpuinfo->ai_protocol);
	// Asociamos el socket a un puerto
    verificarBind(socket_cpu, cpuinfo);
    // Escuchamos las conexiones entrantes
    verificarListen(socket_cpu);

    freeaddrinfo(cpuinfo);
	log_trace(logger, "Listo para escuchar a la memoria");

	return socket_cpu;
}


void verificarBind(int socket_cpu, const struct addrinfo *cpuinfo) {
    if(bind(socket_cpu, cpuinfo->ai_addr, cpuinfo->ai_addrlen) == -1) {
        perror("Hubo un error en el bind: ");
        close(socket_cpu);
        exit(-1);
    }
}

void verificarListen(int socket_cpu) {
    if (listen(socket_cpu, SOMAXCONN) == -1) {
        perror("Hubo un error en el listen: ");
        close(socket_cpu);
        exit(-1);
    }
}

int esperar_memoria(int socket_cpu)
{
	// Aceptamos una memoria
    int socket_memoria = accept(socket_cpu, NULL, NULL);

	if (socket_memoria == -1) {
	    perror("Hubo un error en aceptar una conexión de la memoria: ");
	    close(socket_cpu);
	    exit(-1);
	}

	log_info(logger, "Se conecto una memoria!");
	return socket_memoria;
}

op_code recibirOperacion(int socket_memoria) {
    op_code cod_op;

    if(recv(socket_memoria, &cod_op, sizeof(op_code), MSG_WAITALL) > 0) {
        printf("recibirOperacion --> cod_op: %d\n", cod_op);
        return cod_op;
    }
    else {
        close(socket_memoria);
        return -1;
    }
}

void* recibirBuffer(size_t size, int socket_memoria)
{
	void * buffer;
	buffer = malloc(size);
	recv(socket_memoria, buffer, size, MSG_WAITALL);

	return buffer;
}

void recibirMensaje(int socket_memoria)
{
	int size;
	char* buffer = recibirBuffer(size, socket_memoria);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}