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
#if pluginDEBUG>0
        printf("Argument String '%s'\n",val->string_value);
#endif
        return val->string_value;
    }
#if pluginDEBUG>0
    printf("Argument String ''\n");
#endif
    return "";
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
#if pluginDEBUG>0
    printf("Argument INT '%d'\n",val->int_value);
#endif
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
#ifdef pluginDEBUG
    printf("Argument FLOAT '%g'\n",val->float_value);
#endif
    if (val) return val->float_value;
    else return 0.0;
}

/* Get the number of child attributes */
rxinteger rxpa_getnumattrs(rxpa_attribute_value attributeValue) {
    value* val = (value*)attributeValue;
#ifdef pluginDEBUG
    printf("Argument NUMATTRS '%d'\n",(int)val->num_attributes);
#endif
    if (val) return (rxinteger)val->num_attributes;
    else return 0;
}

/* Set the number of child attributes */
void rxpa_setnumattrs(rxpa_attribute_value attributeValue, rxinteger numAttrs) {
    value* val = (value*)attributeValue;
#ifdef pluginDEBUG
    printf("Argument SETNUMATTRS '%d'\n",(int)numAttrs);
#endif
    if (val) set_num_attributes(val, numAttrs);
}

/* Get the nth child attribute */
rxpa_attribute_value rxpa_getattr(rxpa_attribute_value attributeValue, rxinteger index) {
    value* val = (value*)attributeValue;
#ifdef pluginDEBUG
    printf("Argument GETATTR '%d'\n",(int)index);
#endif
    if (val && index >= 0 && index < val->num_attributes) {
        return (rxpa_attribute_value)val->attributes[index];
    }
    return NULL;
}

/* Insert a child attribute before the nth position */
rxpa_attribute_value rxpa_insertattr(rxpa_attribute_value attributeValue, rxinteger index) {
    value* val = (value*)attributeValue;
#ifdef pluginDEBUG
    printf("Argument INSERTATTR '%d'\n",(int)index);
#endif
    if (val && index >= 0 && index <= val->num_attributes) {
        // Increase the max number of attributes
        set_num_attributes(val, val->num_attributes + 1);
        val->num_attributes--;

        value* newval = value_f();
        if (newval) {
            if (index < val->num_attributes) {
                /* Move the attributes up */
                memmove(&val->attributes[index+1], &val->attributes[index], (val->num_attributes - index) * sizeof(value*));
            }
            val->attributes[index] = newval;
            val->num_attributes++;
            return (rxpa_attribute_value)newval;
        }
    }
    return NULL;
}

/* Remove the nth child attribute */
void rxpa_removeattr(rxpa_attribute_value attributeValue, rxinteger index) {
    value* val = (value*)attributeValue;
#ifdef pluginDEBUG
    printf("Argument REMOVEATTR '%d'\n",(int)index);
#endif
    if (val && index >= 0 && index < val->num_attributes) {
        value* oldval = val->attributes[index];
        if (oldval) {
            /* Move the attributes down */
            memmove(&val->attributes[index], &val->attributes[index+1], (val->num_attributes - index - 1) * sizeof(value*));
            val->num_attributes--;
            clear_value(oldval);
        }
    }
}

/* Swap the nth child attribute with the mth child attribute */
void rxpa_swapattrs(rxpa_attribute_value attributeValue, rxinteger index1, rxinteger index2) {
    value* val = (value*)attributeValue;
#ifdef pluginDEBUG
    printf("Argument SWAPATTRS '%d' '%d'\n",(int)index1,(int)index2);
#endif
    if (val && index1 >= 0 && index1 < val->num_attributes && index2 >= 0 && index2 < val->num_attributes) {
        value* temp = val->attributes[index1];
        val->attributes[index1] = val->attributes[index2];
        val->attributes[index2] = temp;
    }
}
