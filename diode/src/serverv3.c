#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "open62541.h" 
#include <pthread.h>
//La librería dispone de varios tipos básicos usados dentro del estándar OPC-UA).En la medida de lo posible lo ideal es usar estos tipos,
//además de cuando los solicita la propia librería en la firma de las funciones como en este caso.
UA_Boolean running = true; 

//Handler que nos permite parar la ejecución del servidor al hacer un ctrl-c en la línea de comandos
static void stopHandler(int sig) {
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Recibido ctrl-c");
	running = false;
}

//Función para crear variable random
static void addValue(UA_Server * server, char * Valuename, UA_Int32 Valuedata, int nodeid){

//En este caso le estamos fijando un ID numérico específico al nodo pero podemos hacer que la librería lo asigne automáticamente.
	UA_NodeId counterNodeId = UA_NODEID_NUMERIC(1, nodeid);
//Fijamos también el nombre que tendrá el nodo OPC-UA
	UA_QualifiedName counterName = UA_QUALIFIEDNAME(1, Valuename);
    UA_VariableAttributes attr = UA_VariableAttributes_default;
	attr.description = UA_LOCALIZEDTEXT("en_US",Valuename);
	attr.displayName = UA_LOCALIZEDTEXT("en_US",Valuename);
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    //Para hacer la variable escribible (necesario para actualizarla después)
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    //Valor de la variable
    //UA_Int32 randomvalue = rand() % 11;
    UA_Variant_setScalarCopy(&attr.value, &Valuedata, &UA_TYPES[UA_TYPES_INT32]);
    UA_Server_addVariableNode(server, counterNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
		UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
		counterName, UA_NODEID_NULL, attr, NULL, NULL);
}

static void writeVariable(UA_Server *server,int nodeid, int value ) {
    //UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, nodeid);
	UA_NodeId counterNodeId = UA_NODEID_NUMERIC(1, nodeid);
    /* Write a different integer value */
    UA_Int32 myInteger = value;
    UA_Variant myVar;
    UA_Variant_init(&myVar);
    UA_Variant_setScalar(&myVar, &myInteger, &UA_TYPES[UA_TYPES_INT32]);
    UA_Server_writeValue(server, counterNodeId, myVar);

    /* Set the status code of the value to an error code. The function
     * UA_Server_write provides access to the raw service. The above
     * UA_Server_writeValue is syntactic sugar for writing a specific node
     * attribute with the write service. */
    UA_WriteValue wv;
    UA_WriteValue_init(&wv);
    wv.nodeId = counterNodeId;
    wv.attributeId = UA_ATTRIBUTEID_VALUE;
    wv.value.status = UA_STATUSCODE_BADNOTCONNECTED;
    wv.value.hasStatus = true;
    UA_Server_write(server, &wv);

    /* Reset the variable to a good statuscode with a value */
    wv.value.hasStatus = false;
    wv.value.value = myVar;
    wv.value.hasValue = true;
    UA_Server_write(server, &wv);
}
static void addCurrentTimeVariable(UA_Server *server) {
    UA_DateTime now = 0;
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "Current time");
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_Variant_setScalar(&attr.value, &now, &UA_TYPES[UA_TYPES_DATETIME]);

    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "current-time");
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "current-time");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
    UA_Server_addVariableNode(server, currentNodeId, parentNodeId,
                              parentReferenceNodeId, currentName,
                              variableTypeNodeId, attr, NULL, NULL);

    //updateCurrentTime(server);
}
static void beforeReadTime(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeid, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data) {
    UA_Int32 now = rand()%101;
    UA_Variant value;
    UA_Variant_setScalar(&value, &now, &UA_TYPES[UA_TYPES_INT32]);
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "current-time");
    UA_Server_writeValue(server, currentNodeId, value);
}

static void afterWriteTime(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeId, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "The variable was updated");
}

static void addValueCallbackToCurrentTimeVariable(UA_Server *server) {
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "current-time");
    UA_ValueCallback callback ;
    callback.onRead = beforeReadTime;
    callback.onWrite = afterWriteTime;
    UA_Server_setVariableNode_valueCallback(server, currentNodeId, callback);
}

int main(void) { 
    //FJMS sirve para poder parar el main con un ctrl-c. Utiliza la funcion declarada con anterioridad.
	signal(SIGINT, stopHandler);
	signal(SIGTERM, stopHandler); 

	//Creamos un nuevo servidor con la configuración por defecto.
	UA_ServerConfig *config = UA_ServerConfig_new_default();
	UA_Server *server = UA_Server_new(config);
    //Añadimos la variable random
    /*char value1[]="random1";
    char value2[]="random2";
    char value3[]="random3";
    char value4[]="random4";
    char value5[]="random5";
     addValue(server,value1,rand()%11,2035);
     writeVariable(server,2035,rand()%81);
     addValue(server,value2,rand()%11,2036);
     writeVariable(server,2036,rand()%81);
     addValue(server,value3,rand()%11,2037);
     writeVariable(server,2037,rand()%81);
     addValue(server,value4,rand()%11,2038);
     writeVariable(server,2038,rand()%81);
     addValue(server,value5,rand()%11,2039);
     writeVariable(server,2039,rand()%81);
*/
    addCurrentTimeVariable(server);
    addValueCallbackToCurrentTimeVariable(server);
    //addCurrentTimeDataSourceVariable(server);

	//Esta línea lanza el servidor en un bucle mientras la variable running permanece a true.
	//Es importante que las inicializaciones u otras cosas que hagamos en nuestro código se hagan antes de esta llamada.
	UA_StatusCode retval = UA_Server_run(server, &running); 

	//Una vez parada la ejecución limpiamos el estado del servidor
	UA_Server_delete(server);
	UA_ServerConfig_delete(config);

	return (int)retval;
}