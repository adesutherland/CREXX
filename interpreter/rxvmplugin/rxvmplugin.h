
/*
 * rxvmplugin.h
 *
 * This file defines the plugin framework for the CREXX VM. It is used to define the plugin interface for the VM.
 *
 * Created by Adrian Sutherland on 15/09/2024.
 */
#ifndef CREXX_RXVMPLUGIN_H
#define CREXX_RXVMPLUGIN_H

#include "rxvalue.h" /* For register `value` */

/* Enum for the different types of plugins */
typedef enum rxvm_plugin_type {
    RXVM_PLUGIN_UNDEFINED = 0, // Undefined Plugin Type
    RXVM_PLUGIN_DECIMAL = 1, // Decimal Plugin
    RXVM_PLUGIN_UNICODE = 2, // Unicode Plugin
    RXVM_PLUGIN_MAX = 3 // Maximum value for the enum
} rxvm_plugin_type;

// Signal Codes - These need to be synced with the rxsigal enum in crexxpa.h */
#define RXVM_SIGNAL_NONE 0
#define RXVM_SIGNAL_ERROR 1                /* Triggered when a syntax error occurs during the execution of a REXX program */
#define RXVM_SIGNAL_OVERFLOW_UNDERFLOW 2   /* Triggered when a numeric overflow or underflow occurs during the execution of a REXX program */
#define RXVM_SIGNAL_CONVERSION_ERROR 3     /* Triggered when a conversion error between types occurs during the execution of a REXX program */
#define RXVM_SIGNAL_UNKNOWN_INSTRUCTION 4  /* Triggered when the REXX program attempts to execute an unknown RSAS instruction */
#define RXVM_SIGNAL_FUNCTION_NOT_FOUND 5   /* Triggered when the REXX program attempts to execute an unknown function */
#define RXVM_SIGNAL_OUT_OF_RANGE 6         /* Triggered when the REXX program attempts to access an array element that is out of range */
#define RXVM_SIGNAL_FAILURE 7              /* Triggered when an error occurs in an external function or subroutine called by the REXX program */
#define RXVM_SIGNAL_HALT 8                 /* Triggered when the REXX program receives an external request to halt its execution */
#define RXVM_SIGNAL_NOTREADY 9             /* Triggered when there is an input/output error, such as a file not being ready for reading or writing */
#define RXVM_SIGNAL_INVALID_ARGUMENTS 10   /* Triggered when invalid arguments are passed to a function or subroutine */
#define RXVM_SIGNAL_DIVISION_BY_ZERO 11    /* Triggered when the REXX program attempts to divide by zero */
#define RXVM_SIGNAL_UNICODE_ERROR 12       /* Triggered when an unicode error occurs */
#define RXVM_SIGNAL_OTHER 99

/* Base information block for all rxvm plugins - this is extended for each plugin type */
typedef struct rxvm_plugin rxvm_plugin;
struct rxvm_plugin {
    rxvm_plugin_type type; // Type of the plugin
    char *name; // Name of the plugin
    char *version; // Version of the plugin
    char *description; // Description of the plugin

    int signal_number; // Signal number of last operation
    char *signal_string; // Signal string (this is a pointer to a static buffer or NULL)

    void *private_context; // Private context for the plugin
    void (*free)(rxvm_plugin *plugin); // Function to free the plugin
    int (*set_option)(rxvm_plugin *plugin, char *option, char *value); // Function to set an option for the plugin
    int (*get_option)(rxvm_plugin *plugin, char *option, char **value); // Function to get an option for the plugin
};

/* Information block for rxvmplugin plugins with all the function pointers for rxvmplugin maths operations  */
typedef struct decplugin decplugin;
struct decplugin {
    rxvm_plugin base; // Base plugin information

    // Functions provided by the framework
    // Function number_to_simple_format as defined in rxvmplugin_framework.h
    void (*number_to_simple_format)(const char *input, char *output);

    // Functions provided by the plugin
    size_t (*getDigits)(decplugin *plugin); // Get the number of digits in the rxvmplugin context
    void (*setDigits)(decplugin *plugin, size_t digits); // Set the number of digits in the rxvmplugin context

    size_t (*getRequiredStringSize)(decplugin *plugin); // Get the required string size for the rxvmplugin context
    void (*decimalFromString)(decplugin *plugin, value *result, const char *input); // Convert a string to a rxvmplugin number
    void (*decimalToString)(decplugin *plugin, const value *input, char *result); // Convert a rxvmplugin number to a string
    void (*decimalFromInt)(decplugin *plugin, value *result, const rxinteger input); // Convert an int to a rxvmplugin number
    void (*decimalToInt)(decplugin *plugin, const value *input, rxinteger *result); // Convert a rxvmplugin number to an int
    void (*decimalFromDouble)(decplugin *plugin, value *result, const double input); // Convert a double to a rxvmplugin number
    void (*decimalToDouble)(decplugin *plugin, const value *input, double *result); // Convert a rxvmplugin number to a double
    void (*decimalExtract)(decplugin *plugin, char *coefficient, rxinteger *exponent, value *decimal); // Extract the decimal components from a float

    void (*decimalAdd)(decplugin *plugin, value *result, const value *op1, const value *op2); // Add two rxvmplugin numbers
    void (*decimalSub)(decplugin *plugin, value *result, const value *op1, const value *op2); // Subtract two rxvmplugin numbers
    void (*decimalMul)(decplugin *plugin, value *result, const value *op1, const value *op2); // Multiply two rxvmplugin numbers
    void (*decimalDiv)(decplugin *plugin, value *result, const value *op1, const value *op2); // Divide two rxvmplugin numbers
    void (*decimalPow)(decplugin *plugin, value *result, const value *op1, const value *op2); // Raise one rxvmplugin number to the power of another
    void (*decimalNeg)(decplugin *plugin, value *result, const value *op1); // Negate a rxvmplugin number
    int (*decimalCompare)(decplugin *plugin, const value *op1, const value *op2); // Compare two rxvmplugin numbers
    int (*decimalCompareString)(decplugin *plugin, const value *op1, const char *op2); // Compare an rxvmplugin number to a string representation of a number
};

/* Information block for unicode plugins */
typedef struct uniplugin uniplugin;
struct uniplugin {
    rxvm_plugin base; // Base plugin information

    /* Break functions */
    void (*unicodeGraphemeBreak)(uniplugin *plugin, value *result, const value *string); // Grapheme break
    void (*unicodeWordBreak)(uniplugin *plugin, value *result, const value *string); // Word break
    void (*unicodeSentenceBreak)(uniplugin *plugin, value *result, const value *string); // Sentence break

    /* Grapheme Type */
    void (*unicodeIsAlpha)(uniplugin *plugin, value *result, const value *string); // Check if a string is alphabetic
    void (*unicodeIsDigit)(uniplugin *plugin, value *result, const value *string); // Check if a string is numeric
    void (*unicodeIsSpace)(uniplugin *plugin, value *result, const value *string); // Check if a string is whitespace
    void (*unicodeIsPrint)(uniplugin *plugin, value *result, const value *string); // Check if a string is printable
    void (*unicodeIsUpper)(uniplugin *plugin, value *result, const value *string); // Check if a string is uppercase
    void (*unicodeIsLower)(uniplugin *plugin, value *result, const value *string); // Check if a string is lowercase

    /* Normalization functions */
    void (*unicodeNFC)(uniplugin *plugin, value *result, const value *string); // Normalize a string to NFC
    void (*unicodeNFD)(uniplugin *plugin, value *result, const value *string); // Normalize a string to NFD
    void (*unicodeNFKC)(uniplugin *plugin, value *result, const value *string); // Normalize a string to NFKC
    void (*unicodeNFKD)(uniplugin *plugin, value *result, const value *string); // Normalize a string to NFKD

    /* Conversion functions */
    void (*unicodeToUpper)(uniplugin *plugin, value *result, const value *string); // Convert a string to uppercase
    void (*unicodeToLower)(uniplugin *plugin, value *result, const value *string); // Convert a string to lowercase
    void (*unicodeToTitle)(uniplugin *plugin, value *result, const value *string); // Convert a string to title case
    void (*unicodeToSentence)(uniplugin *plugin, value *result, const value *string); // Convert a string to sentence case
};

/* typedef for factory functions which return a rxvm plugin structure */
typedef rxvm_plugin *(*rxvm_plugin_factory)();

/* typedef for the function used to register a rxvm plugin factory */
typedef void (*register_rxvmplugin_factory)(char* factory_name, rxvm_plugin_factory factory);

/* Function to register a rxvm plugin factory */
/* This is the external function provided to the plugin when linked statically */
void register_rxvmplugin(char* factory_name, rxvm_plugin_factory factory);

// These are designed to force the expansion of macros before concatenation
// and be compatible with both GCC and MSVC
#define CONCATENATE(a, b) a##b
#define EXPAND_AND_CONCATENATE(a, b) CONCATENATE(a, b)
#define STRINGIFY(x) #x
#define EXPAND_AND_STRINGIFY(x) STRINGIFY(x)
// Create a function name starting with _register_rxvm_plugin
#define UNIQUE_VMINIT_FUNCTION_NAME(plugin_id) EXPAND_AND_CONCATENATE(plugin_id, _register_rxvm_plugin)

// tyoedef for an initializer function - void f(void)
typedef void (*initializer_function)();

// Macro to manually call a plugin initializer which has been compiled with MANUAL_INIT
// This is used to manually call the initializer for a statically linked plugin without
// the need for linker 'magic'
#define CALL_PLUGIN_INITIALIZER(plugin_id) \
    void UNIQUE_VMINIT_FUNCTION_NAME(plugin_id)(void); \
    UNIQUE_VMINIT_FUNCTION_NAME(plugin_id)();

// RXVM_PLUGIN must have a value if we are building a plugin
#ifdef RXVM_PLUGIN

// Are we building a statically linked library or a dll?
#ifdef BUILD_DLL

// The plugin is being built as a DLL

// INITIALIZER is defined to be a simple function
#define INITIALIZER(f) \
    void _register_rxvm_plugin(register_rxvmplugin_factory register_func); \
    void _register_rxvm_plugin(register_rxvmplugin_factory register_func) {register_func(EXPAND_AND_STRINGIFY(RXVM_PLUGIN),f);}

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
    void f(void) {register_rxvmplugin(plugin_name,factory);}

#else

// With thanks to this Initializer/finalizer sample for MSVC and GCC/Clang. 2010-2016 Joe Lowe. Released into the public domain.
#ifdef __cplusplus
#define INITIALIZER(factory,f,plugin_name) \
        static void f(void); \
        struct f##_t_ { f##_t_(void) { f(); } }; static f##_t_ f##_; \
        static void f(void) {register_rxvmplugin(plugin_name,factory);}
#elif defined(_MSC_VER)
#pragma section(".CRT$XCU",read)
#define INITIALIZER2_(factory,f,p,plugin_name) \
        static void f(void); \
        __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
        __pragma(comment(linker,"/include:" p #f "_")) \
        static void f(void) {register_rxvmplugin(plugin_name,factory);}
#ifdef _WIN64
#define INITIALIZER(factory,f,plugin_name) INITIALIZER2_(factory,f,"",plugin_name) {
#else
#define INITIALIZER(factory,f,plugin_name) INITIALIZER2_(factory,f,"_",plugin_name) {
#endif

#else // Not _MSC_VER -> GCC/Clang

#define INITIALIZER(factory,f,plugin_name) \
    static void f(void) __attribute__((constructor)); \
    static void f(void) {register_rxvmplugin(plugin_name,factory);}

#endif // _MSC_VER

#endif // MANUAL_INIT

// Define the REGISTER_PLUGIN macro
#define REGISTER_PLUGIN(factory) INITIALIZER(factory, UNIQUE_VMINIT_FUNCTION_NAME(RXVM_PLUGIN),EXPAND_AND_STRINGIFY(RXVM_PLUGIN))

#endif // BUILD_DLL
#endif // RXVM_PLUGIN

#endif // CREXX_RXVMPLUGIN_H
