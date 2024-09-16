/*
 * Description: This file contains the implementation of the decimal plugin using the ICU
 *              decNumnerlibrary.
 *
 * Created by Adrian Sutherland on 15/09/2024.
 *
 * -------------------------------------------------------------------------------
 * The following parameters have been tuned in icu decNumber for CREXX:
 * DECDPUN = 8       # For 64-bit (Overriding 3)
 * DECNUMDIGITS = 64 # Default (Overriding 100)
 * DECBUFFER = 64    # Internal Buffer size avoiding malloc (Overriding 36)
 * DECUSE64 = 1      # 64-bit (Default)
 * DECEXTFLAG  = 1   # Extended set of flags (Default)
 * DECSUBSET = 0     # No subset - possibly higher performance (Default)
 * -------------------------------------------------------------------------------
*/
#include "decplugin.h"
#include "decNumber.h"
#include "decNumberLocal.h"

/* Ensure that the decNumber is big enough to hold the number */
static void EnsureCapacity(value *number, size_t digits) {
    /* Calculate the size of the decNumber */

    /* How many units are necessary, this is the ceil(digits/DECDPUN) */
    size_t units = (digits + DECDPUN - 1) / DECDPUN;
    size_t size;

    /* DECNUMUNITS is the default (minimum) number of units needed - do we need more? */
    if (units > DECNUMUNITS) {
        size = sizeof(decNumber) + ((units - DECNUMUNITS) * sizeof(decNumberUnit));
    }
    else {
        size = sizeof(decNumber);
    }

    /* If the value has a decimal buffer already */
    if (number->decimal_value) {

        /* If the size is bigger than the current size, reallocate the memory */
        if (size > number->decimal_value_length) {
            /* Allocate the new memory */
            decNumber *new_value = realloc(number->decimal_value, size);

            /* If the reallocation was unsuccessful, panic as per crexx standard behavior */
            if (new_value == NULL) {
                fprintf(stderr, "PANIC: Unable to reallocate memory for decNumber\n");
                exit(1);
            }
            number->decimal_value = new_value;
            number->decimal_value_length = size;
        }
    }
    else {
        /* Allocate the memory for the decNumber */
        number->decimal_value = malloc(size);
        number->decimal_value_length = size;
    }
}

/* Definitions of each of the decimal functions */

/* Get the number of digits in the decimal context */
size_t getDigits(decplugin *plugin) {
    return ((decContext*)(plugin->private_context))->digits;
}

/* Set the number of digits in the decimal context */
void setDigits(decplugin *plugin, size_t digits) {
    ((decContext*)(plugin->private_context))->digits = (int32_t)digits;
}

/* Get the required string size for the decimal context */
size_t getRequiredStringSize(decplugin *plugin) {
    return ((decContext*)(plugin->private_context))->digits + 14;
}

/* Convert a string to a decimal number */
void decFloatFromString(decplugin *plugin, value *result, const char *string) {
    decContext *context = (decContext*)(plugin->private_context);
    EnsureCapacity(result, context->digits);
    decNumberFromString(result->decimal_value, string, context);
}

/* Convert a decimal number to a string */
/* The sttring must be allocated by the caller and should be at least
 * getRequiredStringSize() bytes */
void decFloatToString(decplugin *plugin, const value *number, char *string) {
    decContext *context = (decContext*)(plugin->private_context);
    decNumberReduce(number->decimal_value, number->decimal_value, context);
    decNumberToString(number->decimal_value, string);
}

/* Add two decimal numbers */
void decFloatAdd(decplugin *plugin, value *result, const value *op1, const value *op2) {
    EnsureCapacity(result, ((decContext*)(plugin->private_context))->digits);
    decNumberAdd(result->decimal_value, op1->decimal_value, op2->decimal_value, (decContext*)(plugin->private_context));
}

/* Subtract two decimal numbers */
void decFloatSub(decplugin *plugin, value *result, const value *op1, const value *op2) {
    EnsureCapacity(result, ((decContext*)(plugin->private_context))->digits);
    decNumberSubtract(result->decimal_value, op1->decimal_value, op2->decimal_value, (decContext*)(plugin->private_context));
}

/* Multiply two decimal numbers */
void decFloatMul(decplugin *plugin, value *result, const value *op1, const value *op2) {
    EnsureCapacity(result, ((decContext*)(plugin->private_context))->digits);
    decNumberMultiply(result->decimal_value, op1->decimal_value, op2->decimal_value, (decContext*)(plugin->private_context));
}

/* Divide two decimal numbers */
void decFloatDiv(decplugin *plugin, value *result, const value *op1, const value *op2) {
    EnsureCapacity(result, ((decContext*)(plugin->private_context))->digits);
    decNumberDivide(result->decimal_value, op1->decimal_value, op2->decimal_value, (decContext*)(plugin->private_context));
}

/* Function to create a new decimal plugin */
decplugin *new_decplugin() {
    /* Allocate memory for the context */
    decContext* context = malloc(sizeof(decContext));
    decContextDefault(context, DEC_INIT_BASE); // initialize
    context->traps = 0; // no traps
    context->digits = DECNUMDIGITS; // set precision

    /* Allocate memory for the plugin */
    decplugin *plugin = malloc(sizeof(decplugin));
    plugin->private_context = context;
    plugin->getDigits = getDigits;
    plugin->setDigits = setDigits;
    plugin->getRequiredStringSize = getRequiredStringSize;
    plugin->decFloatFromString = decFloatFromString;
    plugin->decFloatToString = decFloatToString;
    plugin->decFloatAdd = decFloatAdd;
    plugin->decFloatSub = decFloatSub;
    plugin->decFloatMul = decFloatMul;
    plugin->decFloatDiv = decFloatDiv;


    return plugin;
}

/* Function to destroy a decimal plugin */
void destroy_decplugin(decplugin *plugin) {
    free(plugin->private_context);
    free(plugin);
}

