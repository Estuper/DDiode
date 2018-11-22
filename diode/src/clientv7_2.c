#include "open62541.h"
#include <signal.h>
#include <string.h>
#include <stdlib.h>

UA_Boolean running = true;
UA_Logger logger = UA_Log_Stdout;

static void stopHandler(int sign) {
    UA_LOG_INFO(logger, UA_LOGCATEGORY_CLIENT, "Received Ctrl-C");
    running = 0;
}

char *encapsular(UA_UInt32 *numero, char *namestring, UA_Int32 *valor){

    char *out,c_nombre[20],c_valor[20],c_tipodato[10];
    out=(char*)malloc(50);
    memset(out,0,sizeof out);
    sprintf(c_nombre,"%s",namestring);
    strcat(c_nombre,";");
    sprintf(c_valor,"%d",*valor);
    strcat(c_valor,";");
    sprintf(c_tipodato,"%u",*numero);
    strcat(out,c_nombre);
    strcat(out,c_valor);
    strcat(out,c_tipodato);
    return (char*)out;
}

int main(void) {
    signal(SIGINT, stopHandler); /* catches ctrl-c */
    UA_ClientConfig config = UA_ClientConfig_default;
    config.timeout = 1000;
    UA_Client *client = UA_Client_new(config);
    UA_Variant value; /* Variants can hold scalar values and arrays of any type */
    UA_Variant_init(&value);
    UA_QualifiedName my_string_name;
    UA_NodeId prueba;
    while(running) {

        UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://localhost:53530/OPCUA/SimulationServer"), retname,retdata;
        if(retval != UA_STATUSCODE_GOOD) {
                       UA_LOG_ERROR(logger, UA_LOGCATEGORY_CLIENT, "Not connected. Retrying to connect in 1 second");
            /* The connect may timeout after 1 second (see above) or it may fail immediately on network errors */
            /* E.g. name resolution errors or unreachable network. Thus there should be a small sleep here */
             UA_sleep_ms(1000);
             continue;
    }

    /* Read the value attribute of the node. UA_Client_readValueAttribute is a
     * wrapper for the raw read service available as UA_Client_Service_read. */
    
    /* NodeId of the variable holding the current time */
    const UA_NodeId nodeId = UA_NODEID_STRING(5,"Counter1");
    retval = UA_Client_readValueAttribute(client, nodeId, &value);
    retname = UA_Client_readBrowseNameAttribute(client, nodeId, &my_string_name);
    retdata = UA_Client_readDataTypeAttribute(client, nodeId, &prueba);

    if(retval == UA_STATUSCODE_BADCONNECTIONCLOSED) {
            UA_LOG_ERROR(logger, UA_LOGCATEGORY_CLIENT, "Connection was closed. Reconnecting ...");
            continue;
        }

        if(retval == UA_STATUSCODE_GOOD && UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_DATETIME])) {
        UA_DateTime raw_date = *(UA_DateTime *) value.data;
        UA_DateTimeStruct dts = UA_DateTime_toStruct(raw_date);
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "date is: %u-%u-%u %u:%u:%u.%03u\n", dts.day, dts.month, dts.year, dts.hour, dts.min, dts.sec, dts.milliSec); }
        
        if((retval == UA_STATUSCODE_GOOD && UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT32]))&&(retname == UA_STATUSCODE_GOOD)&&(retdata == UA_STATUSCODE_GOOD)){
        UA_Int32  *valor;
        *valor = *(UA_Int32*)value.data;
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "the data is: %d\n",*valor);      
        UA_String  namestring;
        UA_Byte *nstring;
        *nstring = *(UA_Byte*)my_string_name.name.*data;
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "the name is: %s\n",*nstring);   
        UA_UInt32   *numero;
        *numero = prueba.identifier.numeric;
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "the numeric datatype is: %u\n",*numero); 
      //printf("\n%s",encapsular(numero,&namestring,valor));
             }
        
            UA_Variant_deleteMembers(&value);
            UA_sleep_ms(1000);
    }
    /* Clean up */
    UA_Variant_deleteMembers(&value);
    UA_Client_delete(client); /* Disconnects the client internally */
    return UA_STATUSCODE_GOOD;
}