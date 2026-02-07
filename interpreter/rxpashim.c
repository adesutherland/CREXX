/* RXPA Shim Layer - provides global rxpa_ names for standalone VM */
#include "rxpa.h"

/* Forward declarations of VM implementation */
void rxvm_addfunc(rxpa_libfunc func, char* name, char* option, char* type, char* args);
char* rxvm_getstring(rxpa_attribute_value attributeValue);
void rxvm_setstring(rxpa_attribute_value attributeValue, char* string);
void rxvm_setint(rxpa_attribute_value attributeValue, rxinteger int_value);
rxinteger rxvm_getint(rxpa_attribute_value attributeValue);
void rxvm_setfloat(rxpa_attribute_value attributeValue, double double_value);
double rxvm_getfloat(rxpa_attribute_value attributeValue);
rxinteger rxvm_getnumattrs(rxpa_attribute_value attributeValue);
void rxvm_setnumattrs(rxpa_attribute_value attributeValue, rxinteger numAttrs);
rxpa_attribute_value rxvm_getattr(rxpa_attribute_value attributeValue, rxinteger index);
rxpa_attribute_value rxvm_insertattr(rxpa_attribute_value attributeValue, rxinteger index);
void rxvm_removeattr(rxpa_attribute_value attributeValue, rxinteger index);
void rxvm_swapattrs(rxpa_attribute_value attributeValue, rxinteger index1, rxinteger index2);
void rxvm_setsayexit(say_exit_func sayExitFunc);
void rxvm_resetsayexit();

/* Shims */
void rxpa_addfunc(rxpa_libfunc func, char* name, char* option, char* type, char* args) {
    rxvm_addfunc(func, name, option, type, args);
}

char* rxpa_getstring(rxpa_attribute_value attributeValue) {
    return rxvm_getstring(attributeValue);
}

void rxpa_setstring(rxpa_attribute_value attributeValue, char* string) {
    rxvm_setstring(attributeValue, string);
}

void rxpa_setint(rxpa_attribute_value attributeValue, rxinteger value) {
    rxvm_setint(attributeValue, value);
}

rxinteger rxpa_getint(rxpa_attribute_value attributeValue) {
    return rxvm_getint(attributeValue);
}

void rxpa_setfloat(rxpa_attribute_value attributeValue, double value) {
    rxvm_setfloat(attributeValue, value);
}

double rxpa_getfloat(rxpa_attribute_value attributeValue) {
    return rxvm_getfloat(attributeValue);
}

rxinteger rxpa_getnumattrs(rxpa_attribute_value attributeValue) {
    return rxvm_getnumattrs(attributeValue);
}

void rxpa_setnumattrs(rxpa_attribute_value attributeValue, rxinteger numAttrs) {
    rxvm_setnumattrs(attributeValue, numAttrs);
}

rxpa_attribute_value rxpa_getattr(rxpa_attribute_value attributeValue, rxinteger index) {
    return rxvm_getattr(attributeValue, index);
}

rxpa_attribute_value rxpa_insertattr(rxpa_attribute_value attributeValue, rxinteger index) {
    return rxvm_insertattr(attributeValue, index);
}

void rxpa_removeattr(rxpa_attribute_value attributeValue, rxinteger index) {
    rxvm_removeattr(attributeValue, index);
}

void rxpa_swapattrs(rxpa_attribute_value attributeValue, rxinteger index1, rxinteger index2) {
    rxvm_swapattrs(attributeValue, index1, index2);
}

void rxpa_setsayexit(say_exit_func sayExitFunc) {
    rxvm_setsayexit(sayExitFunc);
}

void rxpa_resetsayexit() {
    rxvm_resetsayexit();
}
