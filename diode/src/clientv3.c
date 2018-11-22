#include "open62541.h"
/* Pruebas con distintos tipos de datos
*/
int main(void) {
    UA_Client *client = UA_Client_new(UA_ClientConfig_default);
    UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://localhost:53530/OPCUA/SimulationServer"), retname,retdata;
    if(retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        return (int)retval;
    }

    /* Read the value attribute of the node. UA_Client_readValueAttribute is a
     * wrapper for the raw read service available as UA_Client_Service_read. */
    UA_Variant value; /* Variants can hold scalar values and arrays of any type */
    UA_Variant_init(&value);
    UA_QualifiedName my_string_name;
    UA_NodeId prueba;
    /* NodeId of the variable holding the current time */
    const UA_NodeId nodeId = UA_NODEID_STRING(5,"Counter1");
    retval = UA_Client_readValueAttribute(client, nodeId, &value);
    retname = UA_Client_readBrowseNameAttribute(client, nodeId, &my_string_name);
    retdata = UA_Client_readDataTypeAttribute(client, nodeId, &prueba);

    if(retval == UA_STATUSCODE_GOOD && UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_DATETIME])) {
        UA_DateTime raw_date = *(UA_DateTime *) value.data;
        UA_DateTimeStruct dts = UA_DateTime_toStruct(raw_date);
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "date is: %u-%u-%u %u:%u:%u.%03u\n",
                    dts.day, dts.month, dts.year, dts.hour, dts.min, dts.sec, dts.milliSec);
    } else {
        
        if(retval == UA_STATUSCODE_GOOD && UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT32])){
        UA_Int32  valor;
        valor = *(UA_Int32*)value.data;
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "the data is: %d\n",valor);      }

        if(retname == UA_STATUSCODE_GOOD){
        UA_String  namestring;
        //dentro del tipo de dato UA_QualifiedName (es una estructura), el campo name es de tipo ua_string (que es otra estructura). Por ello, dentro de ua_string, nos interesa el valor data
        // de ahí el .name.data
        namestring = *(UA_String*)my_string_name.name.data;
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "the name is: %s\n",&namestring);      }

        if(retdata == UA_STATUSCODE_GOOD){
        UA_String  datatype;
        UA_UInt32   numero;

        numero = prueba.identifier.numeric;
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "the numeric datatype is: %u\n",numero); 
        datatype = prueba.identifier.string;
        //UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "the string datatype is: %.*s\n",datatype.length,datatype.data);  
        //printf("prubea %.*s", datatype.length,&datatype.data);
             }
    }

    /* Clean up */
    UA_Variant_deleteMembers(&value);
    UA_Client_delete(client); /* Disconnects the client internally */
    return UA_STATUSCODE_GOOD;
}