/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* RXPA Shim Layer - provides global rxpa_ names for standalone VM */
#include "rxpa.h"

/* Forward declarations of VM implementation */
void rxvm_addfunc(rxpa_libfunc func, char* name, char* option, char* type, char* args);
void rxvm_addclass(char* name, char* option, char* type);
void rxvm_addinterface(char* name, char* option, char* type);
void rxvm_addimplements(char* name, char* interface_name);
void rxvm_addmember(char* owner, char* kind, char* member, char* type, char* args);
char* rxvm_getstring(rxpa_attribute_value attributeValue);
void rxvm_setstring(rxpa_attribute_value attributeValue, char* string);
void rxvm_setint(rxpa_attribute_value attributeValue, rxinteger int_value);
rxinteger rxvm_getint(rxpa_attribute_value attributeValue);
void rxvm_setfloat(rxpa_attribute_value attributeValue, double double_value);
double rxvm_getfloat(rxpa_attribute_value attributeValue);
int rxvm_setnativepayload(rxpa_attribute_value attributeValue, const void *payload, size_t length,
                          const rxpa_native_payload_ops *ops, unsigned int flags);
void* rxvm_getnativepayload(rxpa_attribute_value attributeValue, size_t *out_length,
                            const rxpa_native_payload_ops **out_ops, unsigned int *out_flags);
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

void rxpa_addclass(char* name, char* option, char* type) {
    rxvm_addclass(name, option, type);
}

void rxpa_addinterface(char* name, char* option, char* type) {
    rxvm_addinterface(name, option, type);
}

void rxpa_addimplements(char* name, char* interface_name) {
    rxvm_addimplements(name, interface_name);
}

void rxpa_addmember(char* owner, char* kind, char* member, char* type, char* args) {
    rxvm_addmember(owner, kind, member, type, args);
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

int rxpa_setnativepayload(rxpa_attribute_value attributeValue, const void *payload, size_t length,
                          const rxpa_native_payload_ops *ops, unsigned int flags) {
    return rxvm_setnativepayload(attributeValue, payload, length, ops, flags);
}

void* rxpa_getnativepayload(rxpa_attribute_value attributeValue, size_t *out_length,
                            const rxpa_native_payload_ops **out_ops, unsigned int *out_flags) {
    return rxvm_getnativepayload(attributeValue, out_length, out_ops, out_flags);
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
