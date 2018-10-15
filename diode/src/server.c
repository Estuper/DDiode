#include <signal.h>
#include "open62541.h" 

//La librería dispone de varios tipos básicos usados dentro del estándar OPC-UA).En la medida de lo posible lo ideal es usar estos tipos,
//además de cuando los solicita la propia librería en la firma de las funciones como en este caso.
UA_Boolean running = true; 

//Handler que nos permite parar la ejecución del servidor al hacer un ctrl-c en la línea de comandos
static void stopHandler(int sig) {
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Recibido ctrl-c");
	running = false;
}

int main(void) { 
    //FJMS sirve para poder parar el main con un ctrl-c. Utiliza la funcion declarada con anterioridad.
	signal(SIGINT, stopHandler);
	signal(SIGTERM, stopHandler); 

	//Creamos un nuevo servidor con la configuración por defecto.
	UA_ServerConfig *config = UA_ServerConfig_new_default();
	UA_Server *server = UA_Server_new(config); 

	//Esta línea lanza el servidor en un bucle mientras la variable running permanece a true.
	//Es importante que las inicializaciones u otras cosas que hagamos en nuestro código se hagan antes de esta llamada.
	UA_StatusCode retval = UA_Server_run(server, &running); 

	//Una vez parada la ejecución limpiamos el estado del servidor
	UA_Server_delete(server);
	UA_ServerConfig_delete(config);

	return (int)retval;
}