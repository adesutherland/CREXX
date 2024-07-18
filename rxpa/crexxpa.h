// CREXX/PA (Plugin Architecture) Client Header

#ifndef CREXX_PA_H_
#define CREXX_PA_H_

// Plugin Support Functions and Macros

// Typedef for attribute value which is an opaque pointer
typedef void* rxpa_attribute_value;

// Typedef definition of a library function
// Parameters are the number of arguments, an array of rxpa_attribute_value,
// and an rxpa_attribute_value return value
typedef void (*rxpa_libfunc)(int, rxpa_attribute_value*, rxpa_attribute_value, rxpa_attribute_value);

// Enumeration of Signal Codes
typedef enum rxsignal {
    SIGNAL_NONE = 0,
    SIGNAL_ERROR = 1,                /* Triggered when a syntax error occurs during the execution of a REXX program */
    SIGNAL_OVERFLOW_UNDERFLOW = 2,   /* Triggered when a numeric overflow or underflow occurs during the execution of a REXX program */
    SIGNAL_CONVERSION_ERROR = 3,     /* Triggered when a conversion error between types occurs during the execution of a REXX program */
    SIGNAL_UNKNOWN_INSTRUCTION = 4,  /* Triggered when the REXX program attempts to execute an unknown RSAS instruction */
    SIGNAL_FUNCTION_NOT_FOUND = 5,   /* Triggered when the REXX program attempts to execute an unknown function */
    SIGNAL_OUT_OF_RANGE = 6,         /* Triggered when the REXX program attempts to access an array element that is out of range */
    SIGNAL_FAILURE = 7,              /* Triggered when an error occurs in an external function or subroutine called by the REXX program */
    SIGNAL_HALT = 8,                 /* Triggered when the REXX program receives an external request to halt its execution */
    SIGNAL_NOTREADY = 9,             /* Triggered when there is an input/output error, such as a file not being ready for reading or writing */
    SIGNAL_INVALID_ARGUMENTS = 10    /* Triggered when invalid arguments are passed to a function or subroutine */
} rxsignal;

// Plugin Helper Functions
// The following functions are used to interact with the REXX interpreter
typedef void (*rxpa_func_addfunc)(rxpa_libfunc func, char* name,
                                  char* option, char* type, char* args); /* Add a function to the REXX interpreter */
typedef char* (*rxpa_func_getstring)(rxpa_attribute_value attributeValue); /* Get a string from an attribute value */
typedef void (*rxpa_func_setstring)(rxpa_attribute_value attributeValue, char* string); /* Set a string in an attribute value */
typedef void (*rxpa_func_setint)(rxpa_attribute_value attributeValue, int value); /* Set an integer in an attribute value */
typedef int (*rxpa_func_getint)(rxpa_attribute_value attributeValue); /* Get an integer from an attribute value */
typedef void (*rxpa_func_setfloat)(rxpa_attribute_value attributeValue, double value); /* Set a float in an attribute value */
typedef double (*rxpa_func_getfloat)(rxpa_attribute_value attributeValue); /* Get a float from an attribute value */

// The initialization context struct
typedef struct rxpa_initctxptr* rxpa_initctxptr;
struct rxpa_initctxptr {
    rxpa_func_addfunc addfunc;
    rxpa_func_getstring getstring;
    rxpa_func_setstring setstring;
    rxpa_func_setint setint;
    rxpa_func_getint getint;
    rxpa_func_setfloat setfloat;
    rxpa_func_getfloat getfloat;
};

// Global context variable declaration
extern rxpa_initctxptr _rxpa_context;

// Are we building a statically linked library?
#ifdef BUILD_DLL

// Macro is used to register a procedure - dynamic linkage
#define ADDPROC(func, name, option, type, args) _rxpa_context->addfunc((func),(name),(option),(type),(args))
#define GETSTRING(attr) _rxpa_context->getstring((attr))
#define SETSTRING(attr, str) _rxpa_context->setstring((attr),(str))
#define SETINT(attr, value) _rxpa_context->setint((attr),(value))
#define GETINT(attr) _rxpa_context->getint((attr))
#define SETFLOAT(attr, value) _rxpa_context->setfloat((attr),(value))
#define GETFLOAT(attr) _rxpa_context->getfloat((attr))

// The plugin is being built as a DLL
// INITIALIZER is redefined to be a simple function
#define INITIALIZER(f) \
    void f(rxpa_initctxptr context); \
    rxpa_initctxptr _rxpa_context = NULL;                   \
    void f(rxpa_initctxptr context) { _rxpa_context = context;
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
// Define the LOADFUNCS macro
#define LOADFUNCS INITIALIZER(UNIQUE_INIT_FUNCTION_NAME(PLUGIN_ID))

// Helper functions provided by the REXX interpreter
void rxpa_addfunc(rxpa_libfunc func, char* name, char* option, char* type, char* args); /* Add a function to the REXX interpreter */
char* rxpa_getstring(rxpa_attribute_value attributeValue); /* Get a string from an attribute value */
void rxpa_setstring(rxpa_attribute_value attributeValue, char* string); /* Set a string in an attribute value */
void rxpa_setint(rxpa_attribute_value attributeValue, int value); /* Set an integer in an attribute value */
int rxpa_getint(rxpa_attribute_value attributeValue); /* Get an integer from an attribute value */
void rxpa_setfloat(rxpa_attribute_value attributeValue, double value); /* Set a float in an attribute value */
double rxpa_getfloat(rxpa_attribute_value attributeValue); /* Get a float from an attribute value */

// Macro is used to register a procedure - static linkage
#define ADDPROC(func, name, option, type, args) rxpa_addfunc((func),(name),(option),(type),(args))
#define GETSTRING(attr) rxpa_getstring((attr))
#define SETSTRING(attr, str) rxpa_setstring((attr),(str))
#define SETINT(attr, value) rxpa_setint((attr),(value))
#define GETINT(attr) rxpa_getint((attr))
#define SETFLOAT(attr, value) rxpa_setfloat((attr),(value))
#define GETFLOAT(attr) rxpa_getfloat((attr))

#endif

// Plugin Function Tags
#define PROCEDURE(p) \
        static void p(int _numargs, rxpa_attribute_value* _arg, rxpa_attribute_value _return, rxpa_attribute_value _signal)

// Arguments
#define NUM_ARGS _numargs
#define ARG(n) _arg[(n)]
#define RETURN _return
#define SIGNAL _signal

// End of LOADFUNCS
#define ENDLOADFUNCS }

// Macro to set the SIGNAL ATTR
#define RETURNSIGNAL(signal, details) {SETINT(SIGNAL,(signal)); SETSTRING(SIGNAL,(details)); return;}

// Macro to Reset Signal
#define RESETSIGNAL {SETINT(SIGNAL,SIGNAL_NONE); SETSTRING(SIGNAL, "");}

#endif // CREXX_PA_H_
