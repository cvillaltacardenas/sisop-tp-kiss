#include "consola.h"

int main(int argc, char* argv[]) {

  if(argc < 3){
		printf("Cantidad de parametros incorrectos. Debe informar 2 parametros.\n");
		printf("1- Ruta al archivo con instrucciones a ejecutar.\n2- Tamaño del proceso\n");
		return argc;
	}
    t_log* logger = iniciarLogger(LOG_FILE, LOG_NAME);

    t_config* config = iniciarConfig(CONFIG_FILE);
    char* ip = config_get_string_value(config,"IP_KERNEL");
    uint32_t puerto = config_get_int_value(config,"PUERTO_KERNEL");

    uint32_t conexion = crearConexion(ip, puerto, "Consola");

	char* rutaArchivo = argv[1];
	int tamanioProceso = atoi(argv[2]);

	recibirInstrucciones(conexion, logger, rutaArchivo, tamanioProceso);
//    terminarPrograma(conexion, logger, config);
	return EXIT_SUCCESS;
}

int recibirInstrucciones(uint32_t conexion, t_log* logger, char* rutaArchivo, int tamanioProceso) {

	t_list* listaInstrucciones = list_create();

	char** lineasPseudocodigo = leer_archivo_pseudocodigo(rutaArchivo, logger);
	if(lineasPseudocodigo == NULL) {
		log_error(logger,"Lineas Pseudocodigo -> NULL");
		return EXIT_FAILURE;
	}

    for(uint32_t i=0; i< string_array_size(lineasPseudocodigo); i++){
    	printf("%s",lineasPseudocodigo[i]);
    }

    generarListaInstrucciones(&listaInstrucciones, lineasPseudocodigo);

	for(int i=0; i<list_size(listaInstrucciones); i++){
    	t_instruccion* instr = list_get(listaInstrucciones,i);
    	printf("instrucciones\n%d:%d:%d\n",instr->codigo_operacion,instr->parametros[0],instr->parametros[1]);
    }

    enviarListaInstrucciones(conexion, tamanioProceso, listaInstrucciones);
	string_array_destroy(lineasPseudocodigo);
	list_destroy(listaInstrucciones);
    recibirMensaje(conexion, logger);
	//terminar_programa(conexion, logger, config);
	return EXIT_SUCCESS;
}


void generarListaInstrucciones(t_list** instrucciones, char** pseudocodigo){
	while(!string_array_is_empty(pseudocodigo)){
		char* instr = string_array_pop(pseudocodigo);
        agregarInstrucciones(instrucciones, instr);
	    free(instr);
	}
}

void agregarInstrucciones(t_list** instrucciones, char* itr){
	uint32_t posicion = 0;
	t_instruccion* instruccion = malloc(sizeof(t_instruccion));
	char* operacion = strtok_r(itr," ",&itr);
	instruccion->codigo_operacion = obtener_cop(operacion);
	switch(instruccion->codigo_operacion){
		case NO_OP:
			instruccion->parametros[0]=0;
			instruccion->parametros[1]=0;
			char* str = strtok_r(NULL,"\n",&itr);
			uint32_t repeticiones = atoi(str);
			for(uint32_t i=0; i<repeticiones; i++) list_add_in_index(*instrucciones,posicion,instruccion);
			break;
		case IO: case READ:
			instruccion->parametros[0]=atoi(strtok_r(NULL,"\n",&itr));
			instruccion->parametros[1]=0;
			list_add_in_index(*instrucciones,posicion,instruccion);
			break;
		case COPY: case WRITE:
			instruccion->parametros[0]=atoi(strtok_r(NULL," ",&itr));
			instruccion->parametros[1]=atoi(strtok_r(NULL,"\n",&itr));
			list_add_in_index(*instrucciones,posicion,instruccion);
			break;
		case EXIT:
			instruccion->parametros[0]=0;
			instruccion->parametros[1]=0;
			list_add_in_index(*instrucciones,posicion,instruccion);
			break;
	}
}

char** leer_archivo_pseudocodigo(char* ruta, t_log* logger){
	uint32_t tamMaximo = 18;
	char** lineas = string_array_new();
	char linea[tamMaximo];
	FILE* archivo = fopen(ruta,"r");
	if(archivo!=NULL){
		log_info(logger,"Archivo abierto exitosamente");
		uint32_t i=0;
	    while (fgets(linea,tamMaximo,archivo) != NULL) {
	        string_array_push(&lineas,linea);
	        lineas[i] = string_duplicate(linea);
	        i++;
	    }
	    fclose(archivo);
	    return lineas;
	}else{
		log_error(logger,"ERROR al abrir archivo PSEUDOCODIGO - Posible error en la ruta");
		return NULL;
	}
}


instr_code obtener_cop(char* operacion){
	if(string_contains(operacion,"NO_OP")) 		return NO_OP;
	else if(string_contains(operacion,"I/O")) 	return IO;
	else if(string_contains(operacion,"READ")) 	return READ;
	else if(string_contains(operacion,"COPY")) 	return COPY;
	else if(string_contains(operacion,"WRITE")) return WRITE;
	else return EXIT;
}

void leer_consola(t_log* logger) {
	char* leido;
	uint32_t leiCaracterSalida;

	do {
		leido = readline("> ");
		leiCaracterSalida = strcmp(leido, CARACTER_SALIDA);
		if(leiCaracterSalida!=0) log_info(logger,"> %s",leido);
		free(leido);

	} while(leiCaracterSalida);

}

void enviarListaInstrucciones(uint32_t conexion, int tamanioProceso, t_list* instrucciones) {
	t_paquete* paquete = crearPaquete();
	paquete->codigo_operacion = LISTA_INSTRUCCIONES;

	for(uint32_t i=0; i<list_size(instrucciones); i++){
	    t_instruccion *instruccion = list_get(instrucciones, i);
        agregarInstruccion(paquete, (void *) instruccion);
        printf("instruccion-->codigoInstruccion->%d\toperando1-> %d\toperando2-> %d\n",
               instruccion->codigo_operacion,
               instruccion->parametros[0],
               instruccion->parametros[1]);	}
    agregarTamanioProceso(paquete, tamanioProceso);
    enviarPaquete(paquete, conexion);
    eliminarPaquete(paquete);

}

int tamanioCodigoOperacion(instr_code codigoOperacion) {
    if (codigoOperacion==NO_OP || codigoOperacion== IO || codigoOperacion == READ) {
        return sizeof(instr_code) + sizeof(operando);
    }
    else if (codigoOperacion==WRITE || codigoOperacion == COPY) {
        return  sizeof(instr_code)+sizeof(operando)*2;
    }
      else {
          return sizeof(instr_code);
      }
}



