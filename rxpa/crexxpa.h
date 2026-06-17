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

// CREXX/PA (Plugin Architecture) Client Header
// Version:

#ifndef CREXX_PA_H_
#define CREXX_PA_H_

#include "crexx_version.h"
#include <stddef.h>

// plugin debug set to 1 if needed, else 0  added by pej 28. OCT 2024
//    debug is created in GETSTRING/GETINT/GETFLOAD calls and typically outputs the REXX input parameters
// #define pluginDEBUG 0

// Plugin Support Functions and Macros

// Define rxinteger type
#ifndef RXINTEGER_T
#define RXINTEGER_T
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L /* C99 */
#include <stdint.h>
typedef intmax_t rxinteger;
#else
#ifdef __32BIT__
typedef long rxinteger;
#else
typedef long long rxinteger;
#endif
#endif
#endif //RXINTEGER_T

// Typedef for attribute value which is an opaque pointer
typedef void* rxpa_attribute_value;

#ifndef RXVM_NATIVE_PAYLOAD_OPS_DEFINED
#define RXVM_NATIVE_PAYLOAD_OPS_DEFINED
#define RXVM_NATIVE_PAYLOAD_FLAG_BITCOPY_SAFE 0x00000001u

typedef struct rxvm_native_payload_ops {
    const char *type_name;
    void (*copy)(void *dest_value, void *source_value);
    void (*finalize)(void *value);
} rxvm_native_payload_ops;
#endif

typedef rxvm_native_payload_ops rxpa_native_payload_ops;

// Typedef definition of a library function
// Parameters are the number of arguments, an array of rxpa_attribute_value,
// and a rxpa_attribute_value return value, and signal value for error handling
// e.g.
//void myfunc(rxinteger _numargs, rxpa_attribute_value* _arg, rxpa_attribute_value _return, rxpa_attribute_value _signal)
typedef void (*rxpa_libfunc)(rxinteger, rxpa_attribute_value*, rxpa_attribute_value, rxpa_attribute_value);

// Enumeration of Signal Codes
// These are used to indicate the status of the REXX program
// NOTE These need to sync with the interpreter rxsignal.h file (they are separate to isolate the plugin from the interpreter)
typedef enum rxsignal {
    SIGNAL_NONE = 0,
    SIGNAL_ERROR = 3,                /* Triggered when a syntax error occurs during the execution of a REXX program */
    SIGNAL_OVERFLOW_UNDERFLOW = 4,   /* Triggered when a numeric overflow or underflow occurs during the execution of a REXX program */
    SIGNAL_CONVERSION_ERROR = 6,     /* Triggered when a conversion error between types occurs during the execution of a REXX program */
    SIGNAL_UNKNOWN_INSTRUCTION = 10, /* Triggered when the REXX program attempts to execute an unknown RXAS instruction */
    SIGNAL_FUNCTION_NOT_FOUND = 11,  /* Triggered when the REXX program attempts to execute an unknown function */
    SIGNAL_OUT_OF_RANGE = 8,         /* Triggered when the REXX program attempts to access an array element that is out of range */
    SIGNAL_REFERENCE_INVALID = 14,   /* Triggered when a reference value no longer points to live storage */
    SIGNAL_OBJECT_NOT_INITIALIZED = 16, /* Triggered when a typed object value is used before an instance is initialized */
    SIGNAL_FAILURE = 2,              /* Triggered when an error occurs in an external function or subroutine called by the REXX program */
    SIGNAL_HALT = 20,                /* Triggered when the REXX program receives an external request to halt (term) its execution */
    SIGNAL_NOTREADY = 15,            /* Triggered when there is an input/output error, such as a file not being ready for reading or writing */
    SIGNAL_INVALID_ARGUMENTS = 7,    /* Triggered when invalid arguments are passed to a function or subroutine */
    SIGNAL_DIVISION_BY_ZERO = 5,     /* Triggered when the REXX program attempts to divide by zero */
    SIGNAL_UNICODE_ERROR = 9,        /* Triggered when an unicode error occurs */
    SIGNAL_OTHER = 30                /* Triggered when an unknown error occurs */
} rxsignal;

// Plugin Helper Functions
// The following functions are used to interact with the REXX interpreter
typedef void (*rxpa_func_addfunc)(rxpa_libfunc func, char* name,
                                  char* option, char* type, char* args); /* Add a function to the REXX interpreter */
typedef void (*rxpa_func_addclass)(char* name, char* option, char* type); /* Add class metadata */
typedef void (*rxpa_func_addinterface)(char* name, char* option, char* type); /* Add interface metadata */
typedef void (*rxpa_func_addimplements)(char* name, char* interface_name); /* Add class/interface implementation metadata */
typedef void (*rxpa_func_addmember)(char* owner, char* kind, char* member,
                                    char* type, char* args); /* Add class/interface member metadata */
typedef char* (*rxpa_func_getstring)(rxpa_attribute_value attributeValue); /* Get a string from an attribute value */
typedef void (*rxpa_func_setstring)(rxpa_attribute_value attributeValue, char* string); /* Set a string in an attribute value */
typedef void (*rxpa_func_setint)(rxpa_attribute_value attributeValue, rxinteger value); /* Set an integer in an attribute value */
typedef rxinteger (*rxpa_func_getint)(rxpa_attribute_value attributeValue); /* Get an integer from an attribute value */
typedef void (*rxpa_func_setfloat)(rxpa_attribute_value attributeValue, double value); /* Set a float in an attribute value */
typedef double (*rxpa_func_getfloat)(rxpa_attribute_value attributeValue); /* Get a float from an attribute value */
typedef int (*rxpa_func_setnativepayload)(rxpa_attribute_value attributeValue,
                                          const void *payload,
                                          size_t length,
                                          const rxpa_native_payload_ops *ops,
                                          unsigned int flags); /* Set a native binary payload */
typedef void* (*rxpa_func_getnativepayload)(rxpa_attribute_value attributeValue,
                                            size_t *out_length,
                                            const rxpa_native_payload_ops **out_ops,
                                            unsigned int *out_flags); /* Get a native binary payload */

// Array / Object Functions - these access the child attributes of an attribute value
/* Get the number of child attributes */
typedef rxinteger (*rxpa_func_getnumattrs)(rxpa_attribute_value attributeValue);
/* Set the number of child attributes */
typedef void (*rxpa_func_setnumattrs)(rxpa_attribute_value attributeValue, rxinteger numAttrs);
/* Get the nth child attribute */
typedef rxpa_attribute_value (*rxpa_func_getattr)(rxpa_attribute_value attributeValue, rxinteger index);
/* Insert a child attribute before the nth position - a blank attribute is added that can then be accessed and changed */
typedef rxpa_attribute_value (*rxpa_func_insertattr)(rxpa_attribute_value attributeValue, rxinteger index);
/* Remove the nth child attribute */
typedef void (*rxpa_func_removeattr)(rxpa_attribute_value attributeValue, rxinteger index);
/* Swap the nth child attribute with the mth child attribute */
typedef void (*rxpa_func_swapattrs)(rxpa_attribute_value attributeValue, rxinteger index1, rxinteger index2);

// Exit Functions
typedef void (*say_exit_func)(char* message);
typedef void (*rxpa_set_say_exit)(say_exit_func sayExitFunc); /* Set Say exit function */
typedef void (*rxpa_reset_say_exit)(); /* Set Say exit function */

// The initialization context struct
typedef struct rxpa_initctxptr* rxpa_initctxptr;
struct rxpa_initctxptr {
    rxpa_func_addfunc addfunc;
    rxpa_func_addclass addclass;
    rxpa_func_addinterface addinterface;
    rxpa_func_addimplements addimplements;
    rxpa_func_addmember addmember;
    rxpa_func_getstring getstring;
    rxpa_func_setstring setstring;
    rxpa_func_setint setint;
    rxpa_func_getint getint;
    rxpa_func_setfloat setfloat;
    rxpa_func_getfloat getfloat;
    rxpa_func_setnativepayload setnativepayload;
    rxpa_func_getnativepayload getnativepayload;
    rxpa_func_getnumattrs getnumattrs;
    rxpa_func_setnumattrs setnumattrs;
    rxpa_func_getattr getattr;
    rxpa_func_insertattr insertattr;
    rxpa_func_removeattr removeattr;
    rxpa_func_swapattrs swapattrs;
    // Exit Function Management
    rxpa_set_say_exit setsayexit;
    rxpa_reset_say_exit resetsayexit;
};

// Are we building a statically linked library?
#ifdef BUILD_DLL

// Include header for memcpy
#include <string.h>

// Global context variable declaration
static struct rxpa_initctxptr _rxpa_initctx;
static rxpa_initctxptr _rxpa_context = &_rxpa_initctx;
// Macro is used to register a procedure - dynamic linkage
#define ADDPROC(func, name, option, type, args) _rxpa_context->addfunc((func),(name),(option),(type),(args))
#define ADDCLASS(name) _rxpa_context->addclass((name),"b",".unknown")
#define ADDCLASSX(name, option, type) _rxpa_context->addclass((name),(option),(type))
#define ADDINTERFACE(name) _rxpa_context->addinterface((name),"b",".unknown")
#define ADDINTERFACEX(name, option, type) _rxpa_context->addinterface((name),(option),(type))
#define ADDIMPLEMENTS(name, interface_name) _rxpa_context->addimplements((name),(interface_name))
#define ADDMEMBER(owner, kind, member, type, args) _rxpa_context->addmember((owner),(kind),(member),(type),(args))
#define ADDFACTORY(owner, member, type, args) ADDMEMBER((owner),"factory",(member),(type),(args))
#define ADDMETHOD(owner, member, type, args) ADDMEMBER((owner),"method",(member),(type),(args))
#define ADDDEFAULTMETHOD(owner, member, type, args) ADDMEMBER((owner),"final method",(member),(type),(args))
#define ENDPROC {back2caller: RESETSIGNAL}     // cleanup of ADDPROC
#define PROCRETURN {goto back2caller;}
#define GETSTRING(attr) _rxpa_context->getstring((attr))
#define GETSARRAY(pnum,index) GETSTRING(GETATTR(pnum, index))
#define SETSTRING(attr, str) _rxpa_context->setstring((attr),(str))
#define SETSARRAY(pnum,index,value) SETSTRING(GETATTR(pnum, index),value)
//  InsertSarray adds a new entry to an array at a certain position, the entry is added prior to the current entry which is shifted beyond
#define INSERTSARRAY(pnum,indx,value) {INSERTATTR(pnum,indx); \
                                       SETSARRAY(pnum,indx,value);};
// Push helpers append to the array handle passed by the caller.
#define PUSHSARRAY(pnum,indx,value) {SETARRAYHI(pnum, indx + 1); \
                                     SETSARRAY(pnum,indx,value);};
#define RETURNSTR(value) _rxpa_context->setstring(RETURN,(value))
#define RETURNSTRX(value) {_rxpa_context->setstring(RETURN,(value));PROCRETURN}
#define SETINT(attr, value) _rxpa_context->setint((attr),(value))
#define SETIARRAY(pnum,index,value) SETINT(GETATTR(pnum, index),value)
#define PUSHIARRAY(pnum,indx,value) {SETARRAYHI(pnum, indx + 1); \
                                     SETIARRAY(pnum,indx,value);};
#define RETURNINT(value) _rxpa_context->setint(RETURN,(value))
#define RETURNINTX(value) {RETURNINT(value); PROCRETURN}
#define GETINT(attr) _rxpa_context->getint((attr))
#define GETIARRAY(pnum,index) GETINT(GETATTR(pnum, index))
#define SETFLOAT(attr, value) _rxpa_context->setfloat((attr),(value))
#define RETURNFLOAT(value) _rxpa_context->setfloat(RETURN,(value))
#define GETFLOAT(attr) _rxpa_context->getfloat((attr))
#define GETFARRAY(pnum,index) GETFLOAT(GETATTR(pnum, index))
#define SETNATIVEPAYLOAD(attr, payload, length, ops, flags) _rxpa_context->setnativepayload((attr),(payload),(length),(ops),(flags))
#define GETNATIVEPAYLOAD(attr, out_length, out_ops, out_flags) _rxpa_context->getnativepayload((attr),(out_length),(out_ops),(out_flags))
#define GETNUMATTRS(attr) _rxpa_context->getnumattrs((attr))
#define GETARRAYHI(attr) _rxpa_context->getnumattrs((attr))
#define SETNUMATTRS(attr, num) _rxpa_context->setnumattrs((attr),(num))
#define SETARRAYHI(attr, num) _rxpa_context->setnumattrs((attr),(num))
#define GETATTR(attr, index) _rxpa_context->getattr((attr),(index))
#define INSERTATTR(attr, index) _rxpa_context->insertattr((attr),(index))
#define REMOVEATTR(attr, index) _rxpa_context->removeattr((attr),(index))
#define SWAPATTRS(attr, index1, index2) _rxpa_context->swapattrs((attr),(index1),(index2))
#define SWAPARRAY(attr, index1, index2) _rxpa_context->swapattrs((attr),(index1),(index2))
#define SET_SAY_EXIT(func) _rxpa_context->setsayexit((func))
#define RESET_SAY_EXIT() _rxpa_context->resetsayexit()

// The plugin is being built as a DLL
// INITIALIZER is redefined to be a simple function
#define INITIALIZER(f) \
    void f(rxpa_initctxptr context); \
    void f(rxpa_initctxptr context) { memcpy(&_rxpa_initctx, context, sizeof(_rxpa_initctx));
// Define EXPORT appropriately for windows
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
// Define EXPORT For OSX
#elif __APPLE__
#define EXPORT __attribute__((visibility("default")))
// Define EXPORT for Linux
#else
#define EXPORT
#endif
#define LOADFUNCS EXPORT INITIALIZER(_initfuncs)
#define FINALIZER(f) \
    static void f(void) __attribute__((destructor)); \
    static void f(void) {

#else

// Else Static build

// With thanks to this Initializer/finalizer sample for MSVC and GCC/Clang. 2010-2016 Joe Lowe. Released into the public domain.
#ifdef __cplusplus
#define INITIALIZER(f) \
        static void f(void); \
        struct f##_t_ { f##_t_(void) { f(); } }; static f##_t_ f##_; \
        static void f(void)
#elif defined(_MSC_VER)
#pragma section(".CRT$XCU",read)
#define INITIALIZER2_(f,p) \
        static void f(void); \
        __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
        __pragma(comment(linker,"/include:" p #f "_")) \
        static void f(void)
#ifdef _WIN64
#define INITIALIZER(f) INITIALIZER2_(f,"") {
#else
#define INITIALIZER(f) INITIALIZER2_(f,"_") {
#endif
#else
#define INITIALIZER(f) \
        static void f(void) __attribute__((constructor)); \
        static void f(void) {
#define FINALIZER(f) \
        static void f(void) __attribute__((destructor)); \
        static void f(void) {
#endif

// Give PLUGIN_ID a default value
#ifndef PLUGIN_ID
#define PLUGIN_ID rxplugin
#endif

// The LOADFUNCS macro is used to define the initialization function for the library.
// Create a function name based on the PLUGIN_ID
// These are designed to force the expansion of macros before concatenation
// and be compatible with both GCC and MSVC
#define CONCATENATE(a, b) a##b
#define EXPAND_AND_CONCATENATE(a, b) CONCATENATE(a, b)
// Now create the function name
#define UNIQUE_INIT_FUNCTION_NAME(plugin_id) EXPAND_AND_CONCATENATE(plugin_id, _init)
// Create a unique anchor symbol name for static libraries
#define UNIQUE_ANCHOR_NAME(plugin_id) EXPAND_AND_CONCATENATE(plugin_id, _anchor)
// Define the LOADFUNCS macro (function prologue)
#define LOADFUNCS INITIALIZER(UNIQUE_INIT_FUNCTION_NAME(PLUGIN_ID))

// Helper functions provided by the REXX interpreter
void rxpa_addfunc(rxpa_libfunc func, char* name, __attribute__((unused)) char* option, char* type, char* args); /* Add a function to the REXX interpreter */
void rxpa_addclass(char* name, char* option, char* type); /* Add class metadata */
void rxpa_addinterface(char* name, char* option, char* type); /* Add interface metadata */
void rxpa_addimplements(char* name, char* interface_name); /* Add class/interface implementation metadata */
void rxpa_addmember(char* owner, char* kind, char* member, char* type, char* args); /* Add class/interface member metadata */
char* rxpa_getstring(rxpa_attribute_value attributeValue); /* Get a string from an attribute value */
void rxpa_setstring(rxpa_attribute_value attributeValue, char* string); /* Set a string in an attribute value */
void rxpa_setint(rxpa_attribute_value attributeValue, rxinteger value); /* Set an integer in an attribute value */
rxinteger rxpa_getint(rxpa_attribute_value attributeValue); /* Get an integer from an attribute value */
void rxpa_setfloat(rxpa_attribute_value attributeValue, double value); /* Set a float in an attribute value */
double rxpa_getfloat(rxpa_attribute_value attributeValue); /* Get a float from an attribute value */
int rxpa_setnativepayload(rxpa_attribute_value attributeValue, const void *payload, size_t length,
                          const rxpa_native_payload_ops *ops, unsigned int flags); /* Set a native binary payload */
void* rxpa_getnativepayload(rxpa_attribute_value attributeValue, size_t *out_length,
                            const rxpa_native_payload_ops **out_ops,
                            unsigned int *out_flags); /* Get a native binary payload */
rxinteger rxpa_getnumattrs(rxpa_attribute_value attributeValue); /* Get the number of child attributes */
void rxpa_setnumattrs(rxpa_attribute_value attributeValue, rxinteger numAttrs); /* Set the number of child attributes */
rxpa_attribute_value rxpa_getattr(rxpa_attribute_value attributeValue, rxinteger index); /* Get the nth child attribute */
rxpa_attribute_value rxpa_insertattr(rxpa_attribute_value attributeValue, rxinteger index); /* Insert a child attribute before the nth position */
void rxpa_removeattr(rxpa_attribute_value attributeValue, rxinteger index); /* Remove the nth child attribute */
void rxpa_swapattrs(rxpa_attribute_value attributeValue, rxinteger index1, rxinteger index2); /* Swap the nth child attribute with the mth child attribute */

// Exit Functions
void rxpa_setsayexit(say_exit_func sayExitFunc); /* Set Say exit function */
void rxpa_resetsayexit(); /* Set Say exit function */

// Macro is used to register a procedure - static linkage
#ifndef DECL_ONLY
#define ADDPROC(func, name, option, type, args) rxpa_addfunc((func),(name),(option),(type),(args))
#else
#define ADDPROC(func, name, option, type, args) rxpa_addfunc(0,(name),(option),(type),(args))
#endif
#define ADDCLASS(name) rxpa_addclass((name),"b",".unknown")
#define ADDCLASSX(name, option, type) rxpa_addclass((name),(option),(type))
#define ADDINTERFACE(name) rxpa_addinterface((name),"b",".unknown")
#define ADDINTERFACEX(name, option, type) rxpa_addinterface((name),(option),(type))
#define ADDIMPLEMENTS(name, interface_name) rxpa_addimplements((name),(interface_name))
#define ADDMEMBER(owner, kind, member, type, args) rxpa_addmember((owner),(kind),(member),(type),(args))
#define ADDFACTORY(owner, member, type, args) ADDMEMBER((owner),"factory",(member),(type),(args))
#define ADDMETHOD(owner, member, type, args) ADDMEMBER((owner),"method",(member),(type),(args))
#define ADDDEFAULTMETHOD(owner, member, type, args) ADDMEMBER((owner),"final method",(member),(type),(args))
#define ENDPROC {back2caller: RESETSIGNAL}     // cleanup of ADDPROC
#define PROCRETURN {goto back2caller;}
#define GETSTRING(attr) rxpa_getstring((attr))
#define SETSTRING(attr, str) rxpa_setstring((attr),(str))
#define SETINT(attr, value) rxpa_setint((attr),(value))
#define GETINT(attr) rxpa_getint((attr))
#define SETFLOAT(attr, value) rxpa_setfloat((attr),(value))
#define GETFLOAT(attr) rxpa_getfloat((attr))
#define SETNATIVEPAYLOAD(attr, payload, length, ops, flags) rxpa_setnativepayload((attr),(payload),(length),(ops),(flags))
#define GETNATIVEPAYLOAD(attr, out_length, out_ops, out_flags) rxpa_getnativepayload((attr),(out_length),(out_ops),(out_flags))
#define GETNUMATTRS(attr) rxpa_getnumattrs((attr))
#define SETNUMATTRS(attr, num) rxpa_setnumattrs((attr),(num))
#define GETATTR(attr, index) rxpa_getattr((attr),(index))
#define INSERTATTR(attr, index) rxpa_insertattr((attr),(index))
#define REMOVEATTR(attr, index) rxpa_removeattr((attr),(index))
#define SWAPATTRS(attr, index1, index2) rxpa_swapattrs((attr),(index1),(index2))
#define SET_SAY_EXIT(func) rxpa_setsayexit((func))
#define RESET_SAY_EXIT() rxpa_resetsayexit()
#define RETURNSTR(value) rxpa_setstring(RETURN,(value))
#define RETURNSTRX(value) {rxpa_setstring(RETURN,(value));PROCRETURN}
#define RETURNINT(value) rxpa_setint(RETURN,(value))
#define RETURNFLOAT(value) rxpa_setfloat(RETURN,(value))
#define GETARRAYHI(attr) rxpa_getnumattrs((attr))
#define SETARRAYHI(attr, num) rxpa_setnumattrs((attr),(num))
#define SWAPARRAY(attr, index1, index2) rxpa_swapattrs((attr),(index1),(index2))
#define GETSARRAY(pnum,index) GETSTRING(GETATTR(pnum, index))
#define SETSARRAY(pnum,index,value) SETSTRING(GETATTR(pnum, index),value)
#define INSERTSARRAY(pnum,indx,value) {INSERTATTR(pnum,indx); \
SETSARRAY(pnum,indx,value);};
// Push helpers append to the array handle passed by the caller.
#define PUSHSARRAY(pnum,indx,value) {SETARRAYHI(pnum, indx + 1); \
SETSARRAY(pnum,indx,value);};
#define SETIARRAY(pnum,index,value) SETINT(GETATTR(pnum, index),value)
#define PUSHIARRAY(pnum,indx,value) {SETARRAYHI(pnum, indx + 1); \
SETIARRAY(pnum,indx,value);};
#define RETURNINTX(value) {RETURNINT(value); PROCRETURN}
#define GETIARRAY(pnum,index) GETINT(GETATTR(pnum, index))
#define GETFARRAY(pnum,index) GETFLOAT(GETATTR(pnum, index))

#endif

// Plugin Function Tags
#define PROCEDURE(p) \
        static void p(rxinteger _numargs, rxpa_attribute_value* _arg, rxpa_attribute_value _return, rxpa_attribute_value _signal)
// Arguments
#define NUM_ARGS _numargs
#define ARG(n) _arg[(n)]
// some laziness definitions
#define ARG0   _arg[(0)]
#define ARG1   _arg[(1)]
#define ARG2   _arg[(2)]
#define ARG3   _arg[(3)]
#define ARG4   _arg[(4)]
#define ARG5   _arg[(5)]
#define ARG6   _arg[(6)]
#define ARG7   _arg[(7)]
#define ARG8   _arg[(8)]
#define ARG9   _arg[(9)]

#define RETURN _return
#define SIGNAL _signal
#define REGISTER rxpa_attribute_value

// End of LOADFUNCS
// Also define a global anchor symbol to ensure the object provides at least one global
// This avoids empty-TOC warnings when creating static archives on some toolchains
#ifdef BUILD_DLL
#define ENDLOADFUNCS }
#else
  #if defined(_MSC_VER)
    #define ENDLOADFUNCS } __declspec(selectany) int UNIQUE_ANCHOR_NAME(PLUGIN_ID) = 0;
  #else
    #define ENDLOADFUNCS } __attribute__((used)) int UNIQUE_ANCHOR_NAME(PLUGIN_ID) = 0;
  #endif
#endif

// Macro to set the SIGNAL ATTR
#define RETURNSIGNAL(signal, details) {SETINT(SIGNAL,(signal)); SETSTRING(SIGNAL,(details)); return;}

// Macro to Reset Signal
#define RESETSIGNAL {SETINT(SIGNAL,SIGNAL_NONE); SETSTRING(SIGNAL, "");}

#endif // CREXX_PA_H_
