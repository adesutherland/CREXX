//
// RXPA (CREXX Plugin Architecture) support functions
//
#include "crexxpa.h"
#include "rxvmintp.h"
#include "rxvmvars.h"

/* Function to call a native RXPA (CREXX Plugin Architecture) function */
void rxpa_callfunc(void* function, int args, value** argv, value* ret, value* signal) {
    rxpa_libfunc native_function = (rxpa_libfunc)function;
    rxpa_attribute_value* arg_values = (rxpa_attribute_value*)argv;
    rxpa_attribute_value return_value = (rxpa_attribute_value)ret;
    rxpa_attribute_value signal_value = (rxpa_attribute_value)signal;
    native_function(args, arg_values, return_value, signal_value);
 }

 /* Function to get signal text from a signal code  */
char* rxpa_getsignaltext(rxsignal signal) {
     switch (signal) {
         case SIGNAL_NONE:
             return "OK";
         case SIGNAL_ERROR:
             return "ERROR";
         case SIGNAL_OVERFLOW_UNDERFLOW:
             return "OVERFLOW_UNDERFLOW";
         case SIGNAL_CONVERSION_ERROR:
             return "CONVERSION_ERROR";
         case SIGNAL_UNKNOWN_INSTRUCTION:
             return "UNKNOWN_INSTRUCTION";
         case SIGNAL_FUNCTION_NOT_FOUND:
             return "FUNCTION_NOT_FOUND";
         case SIGNAL_OUT_OF_RANGE:
             return "OUT_OF_RANGE";
         case SIGNAL_FAILURE:
             return "FAILURE";
         case SIGNAL_HALT:
             return "HALT";
         case SIGNAL_NOTREADY:
             return "NOTREADY";
         case SIGNAL_INVALID_ARGUMENTS:
             return "INVALID_ARGUMENTS";
         case SIGNAL_OTHER:
             return "OTHER";
         default:;
     }
     return "UNKNOWN";
 }

 /* Function to get a signal code from a signal text */
 rxsignal rxpa_getsignalcode(char* signalText) {
        if (strcmp(signalText, "OK") == 0) {
            return SIGNAL_NONE;
        } else if (strcmp(signalText, "ERROR") == 0) {
            return SIGNAL_ERROR;
        } else if (strcmp(signalText, "OVERFLOW_UNDERFLOW") == 0) {
            return SIGNAL_OVERFLOW_UNDERFLOW;
        } else if (strcmp(signalText, "CONVERSION_ERROR") == 0) {
            return SIGNAL_CONVERSION_ERROR;
        } else if (strcmp(signalText, "UNKNOWN_INSTRUCTION") == 0) {
            return SIGNAL_UNKNOWN_INSTRUCTION;
        } else if (strcmp(signalText, "FUNCTION_NOT_FOUND") == 0) {
            return SIGNAL_FUNCTION_NOT_FOUND;
        } else if (strcmp(signalText, "OUT_OF_RANGE") == 0) {
            return SIGNAL_OUT_OF_RANGE;
        } else if (strcmp(signalText, "FAILURE") == 0) {
            return SIGNAL_FAILURE;
        } else if (strcmp(signalText, "HALT") == 0) {
            return SIGNAL_HALT;
        } else if (strcmp(signalText, "NOTREADY") == 0) {
            return SIGNAL_NOTREADY;
        } else if (strcmp(signalText, "INVALID_ARGUMENTS") == 0) {
            return SIGNAL_INVALID_ARGUMENTS;
        } else if (strcmp(signalText, "OTHER") == 0) {
            return SIGNAL_OTHER;
        }
        return SIGNAL_OTHER;
 }

/* Get a string from an attribute value */
char* rxpa_getstring(rxpa_attribute_value attributeValue) {
    value* val = (value*)attributeValue;
    if (val) {
        null_terminate_string_buffer(val);
        return val->string_value;
    }
    else return "";
}

/* Set a string in an attribute value */
void rxpa_setstring(rxpa_attribute_value attributeValue, char* string){
    value* val = (value*)attributeValue;
    if (val) set_null_string(val, string);
}

/* Set an integer in an attribute value */
void rxpa_setint(rxpa_attribute_value attributeValue, rxinteger int_value) {
    value* val = (value*)attributeValue;
    if (val) set_int(val, int_value);
}

/* Get an integer from an attribute value */
rxinteger rxpa_getint(rxpa_attribute_value attributeValue) {
    value* val = (value*)attributeValue;
    if (val) return val->int_value;
    else return 0;
}

/* Set a float in an attribute value */
void rxpa_setfloat(rxpa_attribute_value attributeValue, double double_value) {
    value* val = (value*)attributeValue;
    if (val) set_float(val, double_value);
}

/* Get a float from an attribute value */
double rxpa_getfloat(rxpa_attribute_value attributeValue) {
    value* val = (value*)attributeValue;
    if (val) return val->float_value;
    else return 0.0;
}
