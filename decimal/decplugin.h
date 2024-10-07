//
// Created by Adrian Sutherland on 15/09/2024.
//
#ifndef CREXX_DECPLUGIN_H
#define CREXX_DECPLUGIN_H

#include "rxvalue.h" /* For value */

/* Structure for the decimal plugin with all the function pointers for decimal
 * maths operatoins and a private pointer for the decimal context to be used by
 * the different implementations of the decimal plugin */
typedef struct decpugin decplugin;
struct decpugin {
    void *private_context; // Private context for the decimal plugin

    void (*free)(decplugin *plugin); // Function to free the decplugin

    size_t (*getDigits)(decplugin *plugin); // Get the number of digits in the decimal context
    void (*setDigits)(decplugin *plugin, size_t digits); // Set the number of digits in the decimal context

    size_t (*getRequiredStringSize)(decplugin *plugin); // Get the required string size for the decimal context
    void (*decFloatFromString)(decplugin *plugin, value *result, const char *string); // Convert a string to a decimal number
    void (*decFloatToString)(decplugin *plugin, const value *number, char *string); // Convert a decimal number to a string

    void (*decFloatAdd)(decplugin *plugin, value *result, const value *op1, const value *op2); // Add two decimal numbers
    void (*decFloatSub)(decplugin *plugin, value *result, const value *op1, const value *op2); // Subtract two decimal numbers
    void (*decFloatMul)(decplugin *plugin, value *result, const value *op1, const value *op2); // Multiply two decimal numbers
    void (*decFloatDiv)(decplugin *plugin, value *result, const value *op1, const value *op2); // Divide two decimal numbers
};

/* typedef for factory functions which return a decimal plugin structure */
typedef decplugin *(*decplugin_factory)();

/* typedef for the function used to register a decimal plugin factory */
typedef void (*register_decplugin_factory)(char* factory_name, decplugin_factory factory);

/* Function to register a decimal plugin factory */
/* This is the external function provided to the plugin when linked statically */
void register_decplugin(char* factory_name, decplugin_factory factory);

// These are designed to force the expansion of macros before concatenation
// and be compatible with both GCC and MSVC
#define CONCATENATE(a, b) a##b
#define EXPAND_AND_CONCATENATE(a, b) CONCATENATE(a, b)
#define STRINGIFY(x) #x
#define EXPAND_AND_STRINGIFY(x) STRINGIFY(x)
// Create a function name starting with _register_dec_plugin
#define UNIQUE_INIT_FUNCTION_NAME(plugin_id) EXPAND_AND_CONCATENATE(plugin_id, _register_dec_plugin)

// tyoedef for an initializer function - void f(void)
typedef void (*initializer_function)();

// Macro to manually call a plugin initializer which has been compiled with MANUAL_INIT
// This is used to manually call the initializer for a statically linked plugin without
// the need for linker 'magic'
#define CALL_PLUGIN_INITIALIZER(plugin_id) \
    void UNIQUE_INIT_FUNCTION_NAME(plugin_id)(void); \
    UNIQUE_INIT_FUNCTION_NAME(plugin_id)();

// DEC_PLUGIN must have a value if we are building a plugin
#ifdef DEC_PLUGIN

// Are we building a statically linked library or a dll?
#ifdef BUILD_DLL

// The plugin is being built as a DLL

// INITIALIZER is defined to be a simple function
#define INITIALIZER(f) \
    void _register_dec_plugin(register_decplugin_factory register_func); \
    void _register_dec_plugin(register_decplugin_factory register_func) {register_func(EXPAND_AND_STRINGIFY(DEC_PLUGIN),f);}

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
#define REGISTER_PLUGIN(factory) EXPORT INITIALIZER(factory)

#else

// Else Static build

#ifdef MANUAL_PLUGIN_LINK
// Need to manually call the initializer via macro CALL_PLUGIN_INITIALIZER(plugin_id)
#define INITIALIZER(factory,f,plugin_name) \
    void f(void) {register_decplugin(plugin_name,factory);}

#else

// With thanks to this Initializer/finalizer sample for MSVC and GCC/Clang. 2010-2016 Joe Lowe. Released into the public domain.
#ifdef __cplusplus
#define INITIALIZER(factory,f,plugin_name) \
        static void f(void); \
        struct f##_t_ { f##_t_(void) { f(); } }; static f##_t_ f##_; \
        static void f(void) {register_decplugin(plugin_name,factory);}
#elif defined(_MSC_VER)
#pragma section(".CRT$XCU",read)
#define INITIALIZER2_(factory,f,p,plugin_name) \
        static void f(void); \
        __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
        __pragma(comment(linker,"/include:" p #f "_")) \
        static void f(void) {register_decplugin(plugin_name,factory);}
#ifdef _WIN64
#define INITIALIZER(factory,f,plugin_name) INITIALIZER2_(factory,f,"",plugin_name) {
#else
#define INITIALIZER(factory,f,plugin_name) INITIALIZER2_(factory,f,"_",plugin_name) {
#endif

#else // Not _MSC_VER -> GCC/Clang

#define INITIALIZER(factory,f,plugin_name) \
    static void f(void) __attribute__((constructor)); \
    static void f(void) {register_decplugin(plugin_name,factory);}

#endif // _MSC_VER

#endif // MANUAL_INIT

// Define the REGISTER_PLUGIN macro
#define REGISTER_PLUGIN(factory) INITIALIZER(factory, UNIQUE_INIT_FUNCTION_NAME(DEC_PLUGIN),EXPAND_AND_STRINGIFY(DEC_PLUGIN))

#endif // BUILD_DLL
#endif // DEC_PLUGIN

#endif //CREXX_DECPLUGIN_FRAMEWORK_H
