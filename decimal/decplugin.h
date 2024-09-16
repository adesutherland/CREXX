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
    void *private_context;
    size_t (*getDigits)(decplugin *plugin);
    void (*setDigits)(decplugin *plugin, size_t digits);
    size_t (*getRequiredStringSize)(decplugin *plugin);
    void (*decFloatFromString)(decplugin *plugin, value *result, const char *string);
    void (*decFloatToString)(decplugin *plugin, const value *number, char *string);

    void (*decFloatAdd)(decplugin *plugin, value *result, const value *op1, const value *op2);
    void (*decFloatSub)(decplugin *plugin, value *result, const value *op1, const value *op2);
    void (*decFloatMul)(decplugin *plugin, value *result, const value *op1, const value *op2);
    void (*decFloatDiv)(decplugin *plugin, value *result, const value *op1, const value *op2);
};

/* Function to create a new decimal plugin */
decplugin *new_decplugin();

/* Function to destroy a decimal plugin */
void destroy_decplugin(decplugin *plugin);


#endif //CREXX_DECPLUGIN_H
