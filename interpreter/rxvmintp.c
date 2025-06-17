//
// Created by adrian on 29/03/2021.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#endif
#ifdef __APPLE__
#include <mach/thread_policy.h>
#endif
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>
#include "platform.h"
#include "rxas.h"
#include "rxvminst.h"
#include "rxastree.h"
#include "rxvmintp.h"
// #include <complex.h>

#include <signal.h>


#include "rxvmvars.h"
#include "rxvmplugin_framework.h"

/* This defines the expected max number of args - if a call has more args than
 * this then an oversized block will be malloced
 * In terms of memory usage / waste each one is only 2 x pointer size */
#define NOMINAL_NUM_ARGS 20

/* Define this to use a safe stack frame recycling mechanism - zeros registers in the stack frame */
//#define SAFE_RECYCLED_STACKFRAMES
#undef SAFE_RECYCLED_STACKFRAMES

/* Misc. Utilities here */

/* Constant to get create the compile time data in ta "iso" like format */
/* __DATE__ format "Mmm dd yyyy" -> Convert to yyyymmdd */
const char compile_date[8+1] =
        {
                // yyyy year
                __DATE__[7], __DATE__[8],
                __DATE__[9], __DATE__[10],

                // First month letter, Oct Nov Dec = '1' otherwise '0'
                (__DATE__[0] == 'O' || __DATE__[0] == 'N' || __DATE__[0] == 'D') ? '1' : '0',

                // Second month letter
                (__DATE__[0] == 'J') ? ( (__DATE__[1] == 'a') ? '1' :       // Jan, Jun or Jul
                                         ((__DATE__[2] == 'n') ? '6' : '7') ) :
                (__DATE__[0] == 'F') ? '2' :                                // Feb
                (__DATE__[0] == 'M') ? (__DATE__[2] == 'r') ? '3' : '5' :   // Mar or May
                (__DATE__[0] == 'A') ? (__DATE__[1] == 'p') ? '4' : '8' :   // Apr or Aug
                (__DATE__[0] == 'S') ? '9' :                                // Sep
                (__DATE__[0] == 'O') ? '0' :                                // Oct
                (__DATE__[0] == 'N') ? '1' :                                // Nov
                (__DATE__[0] == 'D') ? '2' :                                // Dec
                0,

                // First day letter, replace space with digit
                __DATE__[4]==' ' ? '0' : __DATE__[4],

                // Second day letter
                __DATE__[5],

                '\0'
        };

/* Fast integer pow calculation - loop unwound - based / from https://gist.github.com/orlp/3551590
 * by Orson Peters / orlp / Leiden, Netherlands / orsonpeters@gmail.com
 * Returns the result or 0 for overflow / underflow
 * todo can overflow on 32 bit (update the table) */
RX_INLINE rxinteger ipow(rxinteger base, rxinteger exp_int) {
    static const uint8_t highest_bit_set[] = {
            0, 1, 2, 2, 3, 3, 3, 3,
            4, 4, 4, 4, 4, 4, 4, 4,
            5, 5, 5, 5, 5, 5, 5, 5,
            5, 5, 5, 5, 5, 5, 5, 5,
            6, 6, 6, 6, 6, 6, 6, 6,
            6, 6, 6, 6, 6, 6, 6, 6,
            6, 6, 6, 6, 6, 6, 6, 6,
            6, 6, 6, 6, 6, 6, 6, 255, // anything past 63 is a guaranteed overflow with base > 1
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
    };

    /* Adrian added this bit to check for negative / oversize exp
     * the original used uint8 for exp so not needed */
    if (exp_int > 255 || exp_int < 0) {
        if (base == 1) {
            return 1;
        }

        if (base == -1) {
            return 1 - 2 * (exp_int & 1);
        }

        return 0; /* Overflow / underflow */
    }

    rxinteger result = 1;
    uint8_t exp = (uint8_t) exp_int;

    switch (highest_bit_set[exp]) {
        case 255: // we use 255 as an overflow marker and return 0 on overflow/underflow
            if (base == 1) {
                return 1;
            }

            if (base == -1) {
                return 1 - 2 * (exp & 1);
            }

            return 0; /* Overflow / underflow */
        case 6:
            if (exp & 1) result *= base;
            exp >>= 1;
            base *= base;
        case 5:
            if (exp & 1) result *= base;
            exp >>= 1;
            base *= base;
        case 4:
            if (exp & 1) result *= base;
            exp >>= 1;
            base *= base;
        case 3:
            if (exp & 1) result *= base;
            exp >>= 1;
            base *= base;
        case 2:
            if (exp & 1) result *= base;
            exp >>= 1;
            base *= base;
        case 1:
            if (exp & 1) result *= base;
        default:
            return result;
    }
}

/* Function to convert an interrupt to a string: interrupt_entry -> Code Description Massage */
const char *interrupt_to_string(unsigned char interrupt) {
    switch (interrupt) {
        case RXSIGNAL_KILL:
            return "KILL";
        case RXSIGNAL_ERROR:
            return "ERROR";
        case RXSIGNAL_OVERFLOW_UNDERFLOW:
            return"OVERFLOW_UNDERFLOW";
        case RXSIGNAL_CONVERSION_ERROR:
            return "CONVERSION_ERROR";
        case RXSIGNAL_UNKNOWN_INSTRUCTION:
            return "UNKNOWN_INSTRUCTION";
        case RXSIGNAL_FUNCTION_NOT_FOUND:
            return "FUNCTION_NOT_FOUND";
        case RXSIGNAL_NOT_IMPLEMENTED:
            return "NOT_IMPLEMENTED";
        case RXSIGNAL_INVALID_SIGNAL_CODE:
            return "INVALID_SIGNAL_CODE";
        case RXSIGNAL_OUT_OF_RANGE:
            return "OUT_OF_RANGE";
        case RXSIGNAL_FAILURE:
            return "FAILURE";
        case RXSIGNAL_QUIT:
            return "QUIT";
        case RXSIGNAL_TERM:
            return "TERM";
        case RXSIGNAL_NOTREADY:
            return "NOTREADY";
        case RXSIGNAL_INVALID_ARGUMENTS:
            return "INVALID_ARGUMENTS";
        case RXSIGNAL_DIVISION_BY_ZERO:
            return "DIVISION_BY_ZERO";
        case RXSIGNAL_UNICODE_ERROR:
            return "UNICODE_ERROR";
        case RXSIGNAL_POSIX_HUP:
            return "POSIX_HUP";
        case RXSIGNAL_POSIX_INT:
            return "POSIX_INT";
        case RXSIGNAL_POSIX_USR1:
            return "POSIX_USR1";
        case RXSIGNAL_POSIX_USR2:
            return "POSIX_USR2";;
        case RXSIGNAL_POSIX_CHLD:
            return "POSIX_CHLD";
        case RXSIGNAL_BREAKPOINT:
            return "BREAKPOINT";
        case RXSIGNAL_OTHER:
            return "OTHER";
        default:
            return 0; // Invalid Signal Code
    }
}

/* Function to convert a string to an interrupt: Code Description Massage -> interrupt_entry */
unsigned char string_to_interrupt(const char *interrupt) {
    if (strcmp(interrupt, "KILL") == 0) return RXSIGNAL_KILL;
    if (strcmp(interrupt, "ERROR") == 0) return RXSIGNAL_ERROR;
    if (strcmp(interrupt, "OVERFLOW_UNDERFLOW") == 0) return RXSIGNAL_OVERFLOW_UNDERFLOW;
    if (strcmp(interrupt, "CONVERSION_ERROR") == 0) return RXSIGNAL_CONVERSION_ERROR;
    if (strcmp(interrupt, "UNKNOWN_INSTRUCTION") == 0) return RXSIGNAL_UNKNOWN_INSTRUCTION;
    if (strcmp(interrupt, "FUNCTION_NOT_FOUND") == 0) return RXSIGNAL_FUNCTION_NOT_FOUND;
    if (strcmp(interrupt, "NOT_IMPLEMENTED") == 0) return RXSIGNAL_NOT_IMPLEMENTED;
    if (strcmp(interrupt, "INVALID_SIGNAL_CODE") == 0) return RXSIGNAL_INVALID_SIGNAL_CODE;
    if (strcmp(interrupt, "OUT_OF_RANGE") == 0) return RXSIGNAL_OUT_OF_RANGE;
    if (strcmp(interrupt, "FAILURE") == 0) return RXSIGNAL_FAILURE;
    if (strcmp(interrupt, "QUIT") == 0) return RXSIGNAL_QUIT;
    if (strcmp(interrupt, "TERM") == 0) return RXSIGNAL_TERM;
    if (strcmp(interrupt, "NOTREADY") == 0) return RXSIGNAL_NOTREADY;
    if (strcmp(interrupt, "INVALID_ARGUMENTS") == 0) return RXSIGNAL_INVALID_ARGUMENTS;
    if (strcmp(interrupt, "DIVISION_BY_ZERO") == 0) return RXSIGNAL_DIVISION_BY_ZERO;
    if (strcmp(interrupt, "UNICODE_ERROR") == 0) return RXSIGNAL_UNICODE_ERROR;
    if (strcmp(interrupt, "POSIX_HUP") == 0) return RXSIGNAL_POSIX_HUP;
    if (strcmp(interrupt, "POSIX_INT") == 0) return RXSIGNAL_POSIX_INT;
    if (strcmp(interrupt, "POSIX_USR1") == 0) return RXSIGNAL_POSIX_USR1;
    if (strcmp(interrupt, "POSIX_USR2") == 0) return RXSIGNAL_POSIX_USR2;
    if (strcmp(interrupt, "POSIX_CHLD") == 0) return RXSIGNAL_POSIX_CHLD;
    if (strcmp(interrupt, "BREAKPOINT") == 0) return RXSIGNAL_BREAKPOINT;
    if (strcmp(interrupt, "OTHER") == 0) return RXSIGNAL_OTHER;
    return RXSIGNAL_MAX; // Invalid Signal Code
}

/* Stack Frame Factory */
RX_INLINE stack_frame *frame_f(
                    proc_constant *procedure,
                    int no_args,
                    stack_frame *parent,
                    bin_code *return_pc,
                    value *return_reg) {
    stack_frame *this;
    int num_locals;
    int nominal_num_locals;
    int i, j;
    size_t frame_size;
    value *value_buffer;

    num_locals = procedure->locals + procedure->binarySpace->globals + no_args + 1;
    nominal_num_locals = procedure->locals + procedure->binarySpace->globals + NOMINAL_NUM_ARGS + 1;

    /* Do we need an oversized block */
    if (num_locals > nominal_num_locals) nominal_num_locals = num_locals;

    if (*procedure->frame_free_list &&
        (*procedure->frame_free_list)->nominal_number_locals >= num_locals) {

        /* We can reuse this stack frame */
        this = *procedure->frame_free_list;
        *procedure->frame_free_list = this->prev_free;
        this->prev_free = 0;

        /* Reset Local Registers */
        for (i = 0; i < procedure->locals; i++) {
            this->locals[i] = this->baselocals[i];
#ifdef SAFE_RECYCLED_STACKFRAMES
            value_zero(this->locals[i]);
#endif
        }
        /* Make sure global registers are linked correctly */
        for (j = 0; j < procedure->binarySpace->globals; i++, j++) {
            this->locals[i] = this->baselocals[i];
        }
        /* Reset register a0 - number of arguments */
        this->locals[i] = this->baselocals[i];
        value_zero(this->locals[i]);
        this->locals[i]->int_value = no_args;
    }
    else {
        /* Need a new stack frame - allocate all the memory in one go */
        frame_size = sizeof(stack_frame) +
                     ( sizeof(value*) * nominal_num_locals * 2 ) +
                     ( sizeof(value) * (procedure->locals + 1)); /* +1 is for a0 */

        this = (stack_frame *) malloc( frame_size );
        this->prev_free = 0;

        this->baselocals = (value**)(this + 1);
        this->locals = this->baselocals + nominal_num_locals;
        value_buffer = (value*)(this->locals + nominal_num_locals);

        /* Link Locals */
        for (i = 0; i < procedure->locals; i++, value_buffer++) {
            value_init(value_buffer);
            this->locals[i] = value_buffer;
            this->baselocals[i] = value_buffer;
        }

        /* Link Globals */
        for (j = 0; j < procedure->binarySpace->globals; i++, j++) {
            this->baselocals[i] =  procedure->binarySpace->module->globals[j];
            this->locals[i] = procedure->binarySpace->module->globals[j];
        }

        /* Link a0 */
        value_init(value_buffer);
        this->locals[i] = value_buffer;
        this->baselocals[i] = value_buffer;
        this->locals[i]->int_value = no_args;

        this->nominal_number_locals = nominal_num_locals;
    }
    this->parent = parent;
    if (parent) {
        /* Set the interrupt mask based on parent settings */
        memcpy(this->interrupt_table, parent->interrupt_table, sizeof(interrupt_entry) * (RXSIGNAL_MAX));
        this->is_interrupt = parent->is_interrupt;

        /* VM Plugins */
        this->unicode = parent->unicode;
        this->decimal = parent->decimal;
    }
    else {
        /* Set up the default interrupt mask */
        this->interrupt_table[RXSIGNAL_KILL-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_ERROR-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_OVERFLOW_UNDERFLOW-1].response = RXSIGNAL_RESPONSE_IGNORE;
        this->interrupt_table[RXSIGNAL_CONVERSION_ERROR-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_UNKNOWN_INSTRUCTION-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_NOT_IMPLEMENTED-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_FUNCTION_NOT_FOUND-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_OUT_OF_RANGE-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_FAILURE-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_QUIT-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_TERM-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_NOTREADY-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_INVALID_ARGUMENTS-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_DIVISION_BY_ZERO-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_UNICODE_ERROR-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_POSIX_HUP-1].response = RXSIGNAL_RESPONSE_IGNORE;
        this->interrupt_table[RXSIGNAL_POSIX_INT-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_POSIX_USR1-1].response = RXSIGNAL_RESPONSE_IGNORE;
        this->interrupt_table[RXSIGNAL_POSIX_USR2-1].response = RXSIGNAL_RESPONSE_IGNORE;
        this->interrupt_table[RXSIGNAL_POSIX_CHLD-1].response = RXSIGNAL_RESPONSE_IGNORE;
        this->interrupt_table[RXSIGNAL_BREAKPOINT-1].response = RXSIGNAL_RESPONSE_IGNORE;
        this->interrupt_table[RXSIGNAL_OTHER-1].response = RXSIGNAL_RESPONSE_HALT;
        this->is_interrupt = 0; // No signals pending

        /* VM Plugins */
        this->unicode = 0;
        this->decimal = 0;
    }
    this->decimal_loaded_here = 0;
    this->unicode_loaded_here = 0;
    this->return_pc = return_pc;
    this->number_locals = num_locals;
    this->number_args = no_args;
    this->return_reg = return_reg;
    this->procedure = procedure;

    return this;
}

/* Free Stack Frame */
RX_INLINE void free_frame(stack_frame *frame) {
    /* Add to free list */
    frame->prev_free = *(frame->procedure->frame_free_list);
    *(frame->procedure->frame_free_list) = frame;
}

/* Clear Stack Frame - deallocating register contents and plugins */
RX_INLINE void clear_frame(stack_frame *frame) {
    int i;
    /* Reset Local Registers */
    for (i = 0; i < frame->procedure->locals; i++) {
        clear_value(frame->baselocals[i]);
    }
    if (frame->decimal_loaded_here) {
        frame->decimal->base.free((rxvm_plugin*)frame->decimal);
        frame->decimal_loaded_here = 0;
    }
    if (frame->unicode_loaded_here) {
        frame->unicode->base.free((rxvm_plugin*)frame->unicode);
        frame->unicode_loaded_here = 0;
    }
}

// Bit field of raised VM interrupts (checked by the interpreter)
static volatile sig_atomic_t interrupts = 0;

// Function to set an interrupt
void raise_signal(unsigned char signal) {
    interrupts |= 1 << (signal - 1);
}

// Function to clear an interrupt
void clear_signal(unsigned char signal) {
    interrupts &= ~(1 << (signal - 1));
}

// Macro to detect and throw a signal if a RXVM plugin-raised error is present
#define RXSIGNAL_IF_RXVM_PLUGIN_ERROR(signal) \
if ((signal)->base.signal_number  && (signal)->base.signal_number < RXSIGNAL_MAX) { \
interrupts |= 1 << ((signal)->base.signal_number - 1); \
value_zero(interrupt_object[(signal)->base.signal_number]); \
set_null_string(interrupt_object[(signal)->base.signal_number], (signal)->base.signal_string); \
}

// Macro to throw a signal
#define SET_SIGNAL(signal) \
{interrupts |= 1 << ((signal) - 1); \
value_zero(interrupt_object[(signal)]);}

// Macro to throw a signal with a message
#define SET_SIGNAL_MSG(signal, message) \
{interrupts |= 1 << ((signal) - 1); \
value_zero(interrupt_object[(signal)]); \
set_null_string(interrupt_object[(signal)], (message));}

// Macro to throw a signal with a payload
#define SET_SIGNAL_PAYLOAD(signal, payload) \
{interrupts |= 1 << ((signal) - 1); \
copy_value(interrupt_object[(signal)], (payload));}

// Macro and function to detect and throw a signal if a RXPA plugin-raised error is present
#define INTERRUPT_FROM_RXPA_SIGNAL(signal) if ((signal)->int_value || (signal)->string_length) interrupt_from_rxpa_signal(signal,interrupt_object);

void interrupt_from_rxpa_signal(value *signal, value* interrupt_object[RXSIGNAL_MAX]) {
    size_t int_num;

    if (signal->int_value < 1 || signal->int_value >= RXSIGNAL_MAX) {
        null_terminate_string_buffer(signal);
        int_num = string_to_interrupt(signal->string_value);
        if (int_num == RXSIGNAL_MAX) {
            int_num = RXSIGNAL_OTHER;
            value_zero(interrupt_object[int_num]);
            set_null_string(interrupt_object[int_num], signal->string_value);
        }
        else {
            value_zero(interrupt_object[int_num]);
        }
    } else {
        int_num = signal->int_value;
        value_zero(interrupt_object[int_num]);
    }

    // Set the interrupt
    interrupts |= 1 << (int_num - 1);
}

/* Interpreter */
RX_FLATTEN int run(rxvm_context *context, int argc, char *argv[]) {
    proc_constant *procedure;
    proc_constant *step_handler = 0;
    int rc = 0;
    unsigned int initSeed = 0;   // keep last seed for Random function within REXX run
    char hasSeed = 0; // no seed set
    bin_code *pc, *next_pc;
    int mod_index;
    value *interrupt_arg;
    value *signal_value = value_f();
    unsigned char signal_code = 0;
    value *arguments_array;                /* note that the needs mallocing / freeing */
    unsigned char last_interrupt = 0; /* Interrupt being handled */
    /* Array of objects attached to raised interrupts */
    value *interrupt_object[RXSIGNAL_MAX];
    /* Array of addresses that were last interrupted by interrupt number */
    rxinteger last_interrupted_address[RXSIGNAL_MAX] = {0};
    /* Array of modules that were last interrupted by interrupt number */
    rxinteger last_interrupted_module[RXSIGNAL_MAX] = {0};
    stack_frame *current_frame = 0, *temp_frame;

    /* Set up the interrupt object array */
    {
        size_t i;
        for (i = 0; i < RXSIGNAL_MAX; i++) {
            interrupt_object[i] = value_f();
        }
    }
    // Initialize the native signal handler system
    initialize_vm_signals();

#ifdef NTHREADED
    void *next_inst = 0;
#else
    void *next_inst = &&IUNKNOWN;
#endif

    /*
     * Instruction database - loaded from a generated header file
     */
#include "instrset.h"  /* Set up void *address_map[], etc. */

    /* Allocate Interrupt Arg */
    interrupt_arg = value_f();

    /* Thread code - we need to do it here because address_map is only valid
     * in this run() function */
#ifndef NTHREADED
    DEBUG("Threading\n");
    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        size_t i = 0, j;

        while (i < context->modules[mod_index]->segment.inst_size) {
            j = i;
            i += context->modules[mod_index]->segment.binary[i].instruction.no_ops + 1;
            context->modules[mod_index]->segment.binary[j].impl_address =
                    (void *)address_map[context->modules[mod_index]->segment
                            .binary[j].instruction.opcode];
        }
    }
#endif

    /* Find the program's entry point */
    DEBUG("Find program entry point\n");
    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        int i = context->modules[mod_index]->proc_head;
        while (i != -1) {
            procedure =
                    (proc_constant *) (context->modules[mod_index]->segment.const_pool +
                                       i);
            if (procedure->base.type == PROC_CONST &&
                strcmp(procedure->name, "main") == 0)
                break;
            i = procedure->next;
            procedure = 0;
        }
        if (procedure) break;
    }

    if (!procedure) {
        DEBUG("main() not found\n");
        goto interprt_finished;
    }

    DEBUG("Create first Stack Frame\n");
    current_frame = frame_f(procedure, 1, 0, 0, 0);
    /* Arguments (passed in an array) */
    /* a0 is already set by frame_f() */
    /* a1 is the array  */
    {
        int i;
        int a1 = procedure->binarySpace->globals + procedure->locals + 1;
        arguments_array = value_f();
        current_frame->baselocals[a1] = arguments_array;
        current_frame->locals[a1] = current_frame->baselocals[a1];
        set_num_attributes(current_frame->baselocals[a1], argc);

        for (i = 0; i < argc; i++) {
            set_null_string(current_frame->baselocals[a1]->attributes[i], argv[i]);
        }
    }

    /* Load VM Plugins */
    DEBUG("Load VM Plugins\n");
    current_frame->decimal = (decplugin*)get_rxvmplugin(RXVM_PLUGIN_DECIMAL);
    if (!current_frame->decimal) {
        printf("PANIC - No default decimal plugin\n");
        exit(999); // Documented 999 is for missing decimal plugin
    }
    current_frame->decimal_loaded_here = 1;

    // Set the number of digits in the rxvmplugin context
    current_frame->decimal->setDigits(current_frame->decimal, 18); // 18 is the max significant digits for the default plugin

    /* Start */
    DEBUG("Starting inst# %s-0x%x\n", procedure->binarySpace->module->name, (int) procedure->start);
    next_pc = &(current_frame->procedure->binarySpace->binary[procedure->start]);

    CALC_DISPATCH_MANUAL
    DISPATCH

    /* Instruction implementations */
    /* ----------------------------------------------------------------------------
     * The following shortcut macros are used in the instruction implementation
     *      op1R   address the first register operand
     *      op2R   address the second register operand
     *      op3R   address the third  register operand
     *
     *      op1RI  integer of first register operand
     *      op2RI  integer of second register operand
     *      op3RI  integer of third register operand
     *
     *      op1RF  float of first register operand
     *      op2RF  float of second register operand
     *      op3RF  float of third register operand
     *
     *      op1I   integer value of first operand (non-register value)
     *      op2I   integer value of second operand (non-register value)
     *      op3I   integer value of third  operand (non-register value)
     *
     *      op1F   float value of first operand (non-register value)
     *      op2F   float value of second operand (non-register value)
     *      op3F   float value of third  operand (non-register value)
     *
     *      CONV2INT(integer-result-variable,value-to-be-converted)
     *      CONV2FLOAT(float-result-variable,value-to-be-converted)
     * ----------------------------------------------------------------------------
     */

    /* Signal Interrupt Support - this is only used/called when interrupts are pending */
    START_INTERRUPT;
    DEBUG("TRACE - SIGNAL FIRED - CHECK HANDLER\n");

    /* Also clear any pending signals that are ignored and also find the first signal which */
    /* is masked and pending - the first one is the highest priority */
    last_interrupt = 0;
    for (signal_code = 0; signal_code < RXSIGNAL_MAX; signal_code++) {
        if (interrupts & (1 << signal_code)) {
            last_interrupted_module[signal_code + 1] = (rxinteger) current_frame->procedure->binarySpace->module->module_number;
            last_interrupted_address[signal_code + 1] = (rxinteger) (pc - current_frame->procedure->binarySpace->binary);
            if (current_frame->interrupt_table[signal_code].response == RXSIGNAL_RESPONSE_IGNORE) {
                DEBUG("TRACE - INTR IGNORE %s\n", interrupt_to_string(signal_code + 1));
                interrupts &= ~(1 << signal_code);
            } else {
                last_interrupt = signal_code + 1;
                break;
            }
        }
    }

    if (!last_interrupt || last_interrupt >= RXSIGNAL_MAX) {
        /* No un-ignored interrupts pending */
        END_INTERRUPT
    }

    // Clear the interrupt
    if (last_interrupt != RXSIGNAL_BREAKPOINT) {
        // Breakpoints are not cleared
        interrupts &= ~(1 << (last_interrupt - 1));
    }

    // Handle the interrupt
    switch (current_frame->interrupt_table[last_interrupt - 1].response) {

        case RXSIGNAL_RESPONSE_HALT:
            /* Halt */
            DEBUG("TRACE - INTR HANDLER -> HALT %s\n", interrupt_to_string(last_interrupt));
            /* Print error message to stderr */
            if (interrupt_object[last_interrupt]->string_length) {
                fprintf(stderr, "PANIC: %.*s (SIGNAL %s)\n", (int)(interrupt_object[last_interrupt]->string_length), interrupt_object[last_interrupt]->string_value, interrupt_to_string(last_interrupt));
            } else {
                fprintf(stderr, "PANIC: (SIGNAL %s)\n", interrupt_to_string(last_interrupt));
            }
            rc = (int)last_interrupt;
            goto interprt_finished;

        case RXSIGNAL_RESPONSE_SILENT_HALT:
            /* Silent Halt */
            DEBUG("TRACE - INTR HANDLER -> SILENT HALT %s\n", interrupt_to_string(last_interrupt));
            rc = 0;
            goto interprt_finished;

        case RXSIGNAL_RESPONSE_CALL_BRANCH:
            DEBUG("TRACE - INTR HANDLER -> SET BRANCH FOR CALL RETURN ");
            next_pc = current_frame->procedure->binarySpace->binary + current_frame->interrupt_table[last_interrupt -1 ].jump;
            pc = next_pc;
            // Fall through to CALL

        case RXSIGNAL_RESPONSE_CALL: {
            /* Call */
            proc_constant *intr_function = current_frame->interrupt_table[last_interrupt-1].function;
            DEBUG("TRACE - INTR HANDLER -> CALL %s->%s()\n", interrupt_to_string(last_interrupt), intr_function->name);

            if (intr_function->start == SIZE_MAX) {
                SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, "Exception handler not exposed/linked")
                DISPATCH
            }

            /* Populate the interrupt argument object */
            value_zero(interrupt_arg);
            set_num_attributes(interrupt_arg, 5);
            interrupt_arg->attributes[0]->int_value = (rxinteger)last_interrupt;
            interrupt_arg->attributes[1]->int_value = last_interrupted_module[last_interrupt];
            interrupt_arg->attributes[2]->int_value = last_interrupted_address[last_interrupt];
            // Copy the signal name to the 4th attribute
            set_null_string(interrupt_arg->attributes[3], interrupt_to_string(last_interrupt));
            // Move the object to the 5th attribute
            move_value(interrupt_arg->attributes[4], interrupt_object[last_interrupt]);

            if (intr_function->binarySpace == 0) {
                /* This is a native plugin function */
                rxpa_callfunc((void *) (intr_function->start), 1, &interrupt_arg, 0, signal_value);
                if (signal_value->int_value && signal_value->int_value < RXSIGNAL_MAX) {
                    if (signal_value->string_length) {
                        SET_SIGNAL_MSG(signal_value->int_value, signal_value->string_value)
                    } else {
                        SET_SIGNAL(signal_value->int_value)
                    }
                }
                DISPATCH
            } else {
                /* A CREXX Procedure */
                current_frame = frame_f(intr_function, 1, current_frame, pc, 0);

                /* Prepare dispatch to procedure as early as possible */
                next_pc = &(current_frame->procedure->binarySpace->binary[intr_function->start]);
                CALC_DISPATCH_MANUAL

                /* Interrupt being handled */
                current_frame->is_interrupt = last_interrupt;

                /* Argument */
                size_t arg_index = intr_function->binarySpace->globals + intr_function->locals + 1;
                current_frame->baselocals[arg_index] = current_frame->locals[arg_index] = interrupt_arg;

                /* DISPATCH goes the interrupt handler */
                DISPATCH
            }
        }

        case RXSIGNAL_RESPONSE_BRANCH:
            DEBUG("TRACE - INTR HANDLER -> BRANCH %s\n", interrupt_to_string(last_interrupt));
            next_pc = current_frame->procedure->binarySpace->binary + current_frame->interrupt_table[last_interrupt -1].jump;
            CALC_DISPATCH_MANUAL
            DISPATCH

        case RXSIGNAL_RESPONSE_RETURN:
            DEBUG("TRACE - INTR HANDLER -> RET %s\n", interrupt_to_string(last_interrupt));
            {
                /* Where we return to */
                next_pc = current_frame->return_pc;
                CALC_DISPATCH_MANUAL
                // Note that current_frame->is_interrupt cannot be set as a signal triggers us
                /* back to the parent's stack frame */
                temp_frame = current_frame;
                current_frame = current_frame->parent;
                if (!current_frame) {
                    DEBUG("TRACE - INTR RETURNING FROM MAIN()\n");
                    /* Free Argument Values a1... */
                    int i, j;
                    /* a0 is the number of args */
                    int num_args = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                               temp_frame->procedure->locals]->int_value;
                    for (i = 0, j = temp_frame->procedure->binarySpace->globals + temp_frame->procedure->locals + 1;
                         i < num_args;
                         i++, j++) {
                        clear_value(temp_frame->baselocals[j]);
                        free(temp_frame->baselocals[j]);
                         }
                    rc = 0;
                    free_frame(temp_frame);
                    arguments_array = 0; /* We have freed it in the loop above */
                    goto interprt_finished;
                }
                free_frame(temp_frame);
                DISPATCH
            }

        case RXSIGNAL_RESPONSE_IGNORE:
            /* Ignore - Should never get here */
            DEBUG("*ERROR* TRACE INTR HANDLER -> IGNORE (SHOULD NOT GET HERE) %s\n", interrupt_to_string(last_interrupt));
            END_INTERRUPT
    }

    /* Should never get here */
    END_INTERRUPT

START_OF_INSTRUCTIONS

        /* Signal / Interrupt Instructions */

        /* Enable Breakpoints */
        START_INSTRUCTION(BPON) CALC_DISPATCH(0)
            DEBUG("TRACE - BPON\n");
            interrupts |= 1 << (RXSIGNAL_BREAKPOINT - 1);
            DISPATCH

        /* Enable Breakpoints with op1 handler */
        START_INSTRUCTION(BPON_FUNC) CALC_DISPATCH(1)
            {
                proc_constant *signal_function = PROC_OP(1);
                DEBUG("TRACE - BPON %s()\n", signal_function->name);
                current_frame->interrupt_table[RXSIGNAL_BREAKPOINT-1].response = RXSIGNAL_RESPONSE_CALL;
                current_frame->interrupt_table[RXSIGNAL_BREAKPOINT-1].function = signal_function;
                interrupts |= 1 << (RXSIGNAL_BREAKPOINT - 1);
            }
            DISPATCH

        /* Disable Breakpoints */
        START_INSTRUCTION(BPOFF) CALC_DISPATCH(0)
            DEBUG("TRACE - BPOFF\n");
            interrupts &= ~(1 << (RXSIGNAL_BREAKPOINT - 1));
            DISPATCH

        /* Set Signal op1 Handle to Ignore */
        START_INSTRUCTION(SIGIGNORE_STRING) CALC_DISPATCH(1)
            DEBUG("TRACE - SIGIGNORE \"%.*s\"\n", (int) op1S->string_len, op1S->string);
            {
                size_t sig = string_to_interrupt(op1S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_IGNORE;
                    ignore_interrupt((int)sig); // Set the corresponding native interrupt to ignore
                }
            }
            DISPATCH

        /* Set Signal op1 Handle to Halt */
        START_INSTRUCTION(SIGHALT_STRING) CALC_DISPATCH(1)
            DEBUG("TRACE - SIGHALT \"%.*s\"\n", (int) op1S->string_len, op1S->string);
            {
                size_t sig = string_to_interrupt(op1S->string);
                if (sig == RXSIGNAL_MAX) { // Kill can be set to halt
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_HALT;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH

        /* Set Signal op1 Handle to Silent Halt */
        START_INSTRUCTION(SIGSHALT_STRING) CALC_DISPATCH(1)
            DEBUG("TRACE - SIGSHALT \"%.*s\"\n", (int) op1S->string_len, op1S->string);
            {
                size_t sig = string_to_interrupt(op1S->string);
                if (sig == RXSIGNAL_MAX) { // KILL can be set to silent halt
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_SILENT_HALT;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH

        /* Set Signal op2 Handle to Branch to op1 */
        START_INSTRUCTION(SIGBR_ID_STRING) CALC_DISPATCH(2)
            DEBUG("TRACE - SIGBR 0x%x,\"%.*s\"\n", (unsigned int)REG_IDX(1), (int)op2S->string_len, op2S->string);
            {
                size_t sig = string_to_interrupt(op2S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_BRANCH;
                    current_frame->interrupt_table[sig-1].jump = REG_IDX(1);
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH

        /* Set Signal op2 Handle to Call op1 */
        START_INSTRUCTION(SIGCALL_FUNC_STRING) CALC_DISPATCH(2)
            {
                proc_constant *signal_function = PROC_OP(1);
                DEBUG("TRACE - SIGCALL %s(),\"%.*s\"\n", signal_function->name, (int)op2S->string_len, op2S->string);

                size_t sig = string_to_interrupt(op2S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_CALL;
                    current_frame->interrupt_table[sig-1].function = signal_function;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH

        /* Set Signal op3 Handle to Call op2 returning to op1 */
        START_INSTRUCTION(SIGCALLBR_ID_FUNC_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - SIGCALLBR 0x%x,%s(),\"%.*s\"\n", (unsigned int)REG_IDX(1), PROC_OP(2)->name, (int)op3S->string_len, op3S->string);
            {
                proc_constant *signal_function = PROC_OP(2);
                size_t sig = string_to_interrupt(op3S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_CALL_BRANCH;
                    current_frame->interrupt_table[sig-1].function = signal_function;
                    current_frame->interrupt_table[sig-1].jump = REG_IDX(1);
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH

        /* Set Signal op1 Handle to Return */
        START_INSTRUCTION(SIGRET_STRING) CALC_DISPATCH(1)
            DEBUG("TRACE - SIGRET \"%.*s\"\n", (int)op1S->string_len, op1S->string);
            {
                size_t sig = string_to_interrupt(op1S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_RETURN;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH

        /* RXSIGNAL_STRING Signal type op1 */
        START_INSTRUCTION(SIGNAL_STRING) CALC_DISPATCH(1)
            DEBUG("TRACE - SIGNAL \"%.*s\"\n", (int)op1S->string_len, op1S->string);
            SET_SIGNAL(string_to_interrupt(op1S->string));
            DISPATCH

        /* SIGNALT_STRING_REG Signal type op1 if op2 true */
        START_INSTRUCTION(SIGNALT_STRING_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SIGNALT \"%.*s\",R%d\n", (int)op1S->string_len, op1S->string, (int)REG_IDX(2));
            if (op2RI) {
                SET_SIGNAL(string_to_interrupt(op1S->string));
            }
            DISPATCH

        /* SIGNALF_STRING_REG Signal type op1 if op2 true */
        START_INSTRUCTION(SIGNALF_STRING_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SIGNALF \"%.*s\",R%d\n", (int)op1S->string_len, op1S->string, (int)REG_IDX(2));
            if (!op2RI) {
                SET_SIGNAL(string_to_interrupt(op1S->string));
            }
            DISPATCH

        /* SIGNAL_STRING_STRING Signal type op1 (message op2) */
        START_INSTRUCTION(SIGNAL_STRING_STRING) CALC_DISPATCH(2)
            DEBUG("TRACE - SIGNAL \"%.*s\",\"%.*s\"\n", (int)op1S->string_len, op1S->string, (int)op2S->string_len, op2S->string);
            SET_SIGNAL_MSG(string_to_interrupt(op1S->string), op2S->string);
            DISPATCH

        /* SIGNAL_STRING_REG Signal type op1 (payload op2) */
        START_INSTRUCTION(SIGNAL_STRING_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SIGNAL \"%.*s\",R%d\n", (int)op1S->string_len, op1S->string, (int)REG_IDX(2));
            SET_SIGNAL_PAYLOAD(string_to_interrupt(op1S->string), op2R);
            DISPATCH

        /* SIGNALT_STRING_STRING_REG Signal type op1 (message op2) if op3 true */
        START_INSTRUCTION(SIGNALT_STRING_STRING_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SIGNALT \"%.*s\",\"%.*s\",R%d\n", (int)op1S->string_len, op1S->string, (int)op2S->string_len, op2S->string, (int)REG_IDX(2));
            if (op3RI) {
                SET_SIGNAL_MSG(string_to_interrupt(op1S->string), op2S->string);
            }
            DISPATCH

        /* SIGNALF_STRING_STRING_REG Signal type op1 (message op2) if op3 true */
        START_INSTRUCTION(SIGNALF_STRING_STRING_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SIGNALF \"%.*s\",\"%.*s\"\",R%d\n", (int)op1S->string_len, op1S->string, (int)op2S->string_len, op2S->string, (int)REG_IDX(2));
            if (!op3RI) {
                SET_SIGNAL_MSG(string_to_interrupt(op1S->string), op2S->string);
            }
            DISPATCH

        /* Meta Instructions */

        /* Load Module (op1 = module num of loaded op2) */
        START_INSTRUCTION(METALOADMODULE_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - METALOADMODULE R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2));
            {
                null_terminate_string_buffer(op2R);
                /* Load the module */
                int num_modules_before = (int) context->num_modules;
                op1R->int_value = rxldmod(context, op2R->string_value);
                if (op1R->int_value > 0) {
                    /* If successfully loaded, thread the binary - must be done in run() */
#ifndef NTHREADED
                    DEBUG("Threading\n");
                    int mod;
                    for (mod = num_modules_before; mod < op1R->int_value; mod++) {
                        size_t i = 0, j;
                        while (i < context->modules[mod]->segment.inst_size) {
                            j = i;
                            i += context->modules[mod]->segment.binary[i].instruction.no_ops + 1;
                            context->modules[mod]->segment.binary[j].impl_address =
                                (void *) address_map[context->modules[mod]->segment.binary[j].instruction.opcode];
                        }
                    }
#endif
                }
            }
            DISPATCH

            /* Load Instruction Code (op1 = (inst)op2[op3]) */
        START_INSTRUCTION(METALOADINST_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - METALOADINST R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            {
                bin_code inst = context->modules[op2R->int_value - 1]->segment.binary[op3R->int_value];
#ifdef NTHREADED
                /* Bytecode Version */
                op1R->int_value = inst.instruction.opcode;
#else
                /* Threaded Version - basically we are unthreading, finding the
                 * instruction with the corresponding implementation address
                 * (quite slow ... but not an instruction used in normal code */
                op1R->int_value = 0;
                void *impl = inst.impl_address;
                size_t a;
                size_t num_instructions = sizeof(address_map) / sizeof(address_map[0]);
                for (a = 0; a < num_instructions; a++) {
                    if (address_map[a] == impl) {
                        op1R->int_value = (rxinteger)a;
                        break;
                    }
                }
#endif
            }
            DISPATCH

            /* Loaded Modules (op1=array loaded modules) */
        START_INSTRUCTION(METALOADEDMODULES_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - METALOADEDMODULES R%d\n", (int) REG_IDX(1));
            /* op1R will become an array of module names */
            value_zero(op1R);
            set_num_attributes(op1R, context->num_modules);
            op1R->int_value = (rxinteger) context->num_modules; /* The cREXX convention for arrays */
            for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
                set_null_string(op1R->attributes[mod_index], context->modules[mod_index]->name);
            }
            DISPATCH

        /* Loaded Exposed Procedures (op1 = array procedures in module op2) */
        START_INSTRUCTION(METALOADEDEPROCS_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - METALOADEDEPROCS R%d\n", (int) REG_IDX(1));
            {
                chameleon_constant *c_entry;
                proc_constant *p_entry;
                expose_proc_constant *e_entry;
                int i;
                size_t entries;
                value *entry;

                /* Module to be listed */
                size_t mod = op2R->int_value - 1;

                /* op1R will become an array of procedure names and pointers */
                value_zero(op1R);

                /* How many entries are needed? */
                i = context->modules[mod]->expose_head;
                entries = 0;
                while (i != -1) {
                    c_entry = (chameleon_constant *) (context->modules[mod]->segment.const_pool + i);
                    if (c_entry->type == EXPOSE_PROC_CONST) {
                        if (!((expose_proc_constant *) c_entry)->imported) entries++;
                    }
                    i = ((expose_proc_constant *) c_entry)->next;
                }

                /* Set up array */
                set_num_attributes(op1R, entries);
                op1R->int_value = (rxinteger) entries; /* The cREXX convention for arrays */

                /* Populate array */
                i = context->modules[mod]->expose_head;
                entries = 0;
                while (i != -1) {
                    c_entry = (chameleon_constant *) (context->modules[mod]->segment.const_pool + i);
                    if (c_entry->type == EXPOSE_PROC_CONST) {
                        /* Exposed Procedure */
                        e_entry = (expose_proc_constant *) c_entry;
                        p_entry = (proc_constant *) (
                                context->modules[mod]->segment.const_pool
                                + e_entry->procedure);
                        if (!e_entry->imported) {
                            /* Exported - Procedure - populate object and add to array  */
                            entry = op1R->attributes[entries];
                            set_num_attributes(entry, 2);
                            set_null_string(entry->attributes[0], e_entry->index);
                            entry->attributes[1]->int_value = (rxinteger) p_entry;

                            entries++;
                        }
                    }
                    i = e_entry->next;
                }
            }
            DISPATCH

        /* Loaded Procedures (op1 = array procedures in module op2) */
        START_INSTRUCTION(METALOADEDPROCS_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - METALOADEDPROCS R%d\n", (int) REG_IDX(1));
            {
                chameleon_constant *c_entry;
                proc_constant *p_entry;
                int i;
                size_t entries;
                value *entry;

                /* Module to be listed */
                size_t mod = op2R->int_value - 1;

                /* op1R will become an array of procedure names and pointers */
                value_zero(op1R);

                /* How many entries are needed? */
                i = context->modules[mod]->proc_head;
                entries = 0;
                while (i != -1) {
                    c_entry = (chameleon_constant *) (context->modules[mod]->segment.const_pool + i);
                    if (c_entry->type == PROC_CONST) {
                        entries++;
                    }
                    i = ((proc_constant *) c_entry)->next;
                }

                /* Set up array */
                set_num_attributes(op1R, entries);
                op1R->int_value = (rxinteger) entries; /* The cREXX convention for arrays */

                /* Populate array */
                i = context->modules[mod]->proc_head;
                entries = 0;
                while (i != -1) {
                    c_entry = (chameleon_constant *) (context->modules[mod]->segment.const_pool + i);
                    if (c_entry->type == PROC_CONST) {
                        /* Exposed Procedure */
                        p_entry = (proc_constant *) c_entry;
                        entry = op1R->attributes[entries];
                        set_num_attributes(entry, 2);
                        set_null_string(entry->attributes[0], p_entry->name);
                        entry->attributes[1]->int_value = (rxinteger) p_entry;
                        entries++;
                    }
                    i = p_entry->next;
                }
            }
            DISPATCH

            /* Decode opcode (op1 decoded op2) */
        START_INSTRUCTION(METADECODEINST_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - METADECODEINST R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2));

            /* The target register is turned into an object with 7 attributes */
            value_zero(op1R);
            set_num_attributes(op1R, 7);

            /* Populate the object */
            op1R->attributes[0]->int_value = meta_map[op2R->int_value].opcode;
            set_null_string(op1R->attributes[1], meta_map[op2R->int_value].instruction);
            set_null_string(op1R->attributes[2], meta_map[op2R->int_value].desc);
            op1R->attributes[3]->int_value = meta_map[op2R->int_value].operands;
            op1R->attributes[4]->int_value = meta_map[op2R->int_value].op1_type;
            op1R->attributes[5]->int_value = meta_map[op2R->int_value].op2_type;
            op1R->attributes[6]->int_value = meta_map[op2R->int_value].op3_type;
            DISPATCH

            /* Load Integer/Index Operand (op1 = (int)op2[op3]) */
        START_INSTRUCTION(METALOADIOPERAND_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - METALOADIOPERAND R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            op1R->int_value = context->modules[op2R->int_value - 1]->segment.binary[op3R->int_value].iconst;
            DISPATCH

            /* Load Float Operand (op1 = (float)op2[op3]) */
        START_INSTRUCTION(METALOADFOPERAND_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - METALOADFOPERAND R%d,R%dR%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            op1R->float_value = context->modules[op2R->int_value - 1]->segment.binary[op3R->int_value].fconst;
            DISPATCH

            /* Load String Operand (op1 = (string)op2[op3]) */
        START_INSTRUCTION(METALOADSOPERAND_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - METALOADSOPERAND R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            set_const_string(op1R,
                             (string_constant *) (
                                     context->modules[op2R->int_value - 1]->segment.const_pool +
                                     context->modules[op2R->int_value - 1]->segment.binary[op3R->int_value].index
                             ));

            DISPATCH

            /* Load Procedure Operand (op1 = (proc)op2[op3]) */
            /* TODO needs to do more that get the function name - a function object is needed */
        START_INSTRUCTION(METALOADPOPERAND_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - METALOADPOPERAND R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            {
                proc_constant
                        *proc =
                        (proc_constant *) (
                                context->modules[op2R->int_value - 1]->segment.const_pool +
                                context->modules[op2R->int_value - 1]->segment.binary[op3R->int_value].index
                        );
                set_null_string(op1R, proc->name);
            }
            DISPATCH

            /* Load Metadata (op1 = (metadata)op2[op3]) */
        START_INSTRUCTION(METALOADDATA_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - METALOADDATA R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            {
                unsigned char *pool = context->modules[op2R->int_value - 1]->segment.const_pool;
                int i = context->modules[op2R->int_value - 1]->meta_head;
                int j;
                size_t x;
                meta_entry *meta = 0;
                int size = 0;

                /* Clear return object */
                value_zero(op1R);

                /* Find the start of the metadata @ address */
                while (i != -1) {
                    meta = (meta_entry *) (pool + i);
                    if (meta->address < op3R->int_value) i = meta->next;
                    else break;
                }
                if (i == -1 || meta->address > op3R->int_value) {
                    /* No metadata for the address */
                    DISPATCH
                }
                int start = i;

                /* How many entries */
                size = 0;
                while (i != -1) {
                    meta = (meta_entry *) (pool + i);
                    if (meta->address == op3R->int_value) {
                        i = meta->next;
                        size++;
                    } else break;
                }
                set_num_attributes(op1R, size);
                op1R->int_value = size; /* TODO Using int to store the size of an array ... approach tbc! */

                /* Populate output with the metadata */
                for (j = 0, i = start; j < size; j++, i = ((meta_entry *) (pool + i))->next) {
                    value_zero(op1R->attributes[j]);
                    switch (((meta_entry *) (pool + i))->base.type) {
                        case META_SRC:
                            /* TODO we are using the string to hold the object type - final approach tbc */
                            set_null_string(op1R->attributes[j], ".meta_src");
                            set_num_attributes(op1R->attributes[j], 3);
                            op1R->attributes[j]->attributes[0]->int_value = (rxinteger) ((meta_src_constant *) (pool +
                                                                                                                i))->line;
                            op1R->attributes[j]->attributes[1]->int_value = (rxinteger) ((meta_src_constant *) (pool +
                                                                                                                i))->column;
                            x = (rxinteger) ((meta_src_constant *) (pool + i))->source;
                            set_const_string(op1R->attributes[j]->attributes[2], (string_constant *) (pool + x));
                            break;
                        case META_FILE:
                            set_null_string(op1R->attributes[j], ".meta_file");
                            set_num_attributes(op1R->attributes[j], 1);
                            x = (rxinteger) ((meta_file_constant *) (pool + i))->file;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            break;
                        case META_FUNC:
                            set_null_string(op1R->attributes[j], ".meta_func");
                            set_num_attributes(op1R->attributes[j], 6);
                            x = (rxinteger) ((meta_func_constant *) (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_func_constant *) (pool + i))->option;
                            set_const_string(op1R->attributes[j]->attributes[1], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_func_constant *) (pool + i))->type;
                            set_const_string(op1R->attributes[j]->attributes[2], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_func_constant *) (pool + i))->args;
                            set_const_string(op1R->attributes[j]->attributes[3], (string_constant *) (pool + x));
                            op1R->attributes[j]->attributes[4]->int_value = (rxinteger) ((meta_func_constant *) (pool +
                                                                                                                 i))->func;
                            x = (rxinteger) ((meta_func_constant *) (pool + i))->inliner;
                            set_const_string(op1R->attributes[j]->attributes[5], (string_constant *) (pool + x));
                            break;
                        case META_REG:
                            set_null_string(op1R->attributes[j], ".meta_reg");
                            set_num_attributes(op1R->attributes[j], 4);
                            x = (rxinteger) ((meta_reg_constant *) (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_reg_constant *) (pool + i))->option;
                            set_const_string(op1R->attributes[j]->attributes[1], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_reg_constant *) (pool + i))->type;
                            set_const_string(op1R->attributes[j]->attributes[2], (string_constant *) (pool + x));
                            op1R->attributes[j]->attributes[3]->int_value = (rxinteger) ((meta_reg_constant *) (pool +
                                                                                                                i))->reg;
                            break;
                        case META_CONST:
                            set_null_string(op1R->attributes[j], ".meta_const");
                            set_num_attributes(op1R->attributes[j], 4);
                            x = (rxinteger) ((meta_const_constant *) (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_const_constant *) (pool + i))->option;
                            set_const_string(op1R->attributes[j]->attributes[1], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_const_constant *) (pool + i))->type;
                            set_const_string(op1R->attributes[j]->attributes[2], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_const_constant *) (pool + i))->constant;
                            set_const_string(op1R->attributes[j]->attributes[3], (string_constant *) (pool + x));
                            break;
                        case META_CLEAR:
                            set_null_string(op1R->attributes[j], ".meta_clear");
                            set_num_attributes(op1R->attributes[j], 1);
                            x = (rxinteger) ((meta_clear_constant *) (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            break;
                        case STRING_CONST:
                        case PROC_CONST:
                        case BINARY_CONST:
                        case DECIMAL_CONST:
                        case EXPOSE_REG_CONST:
                        case EXPOSE_PROC_CONST:
                            break;

                    }
                }
            }
            DISPATCH

            /* METALOADCALLERADDR - Load caller address object to op1 */
        START_INSTRUCTION(METALOADCALLERADDR_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - METALOADCALLERADDR R%d\n", (int) REG_IDX(1));
            {
                rxinteger mod_no = -1;
                rxinteger addr = -1;

                if (current_frame->parent != 0) {
                    mod_no = (rxinteger) current_frame->parent->procedure->binarySpace->module->module_number;
                    addr = (rxinteger) (current_frame->return_pc -
                                        current_frame->parent->procedure->binarySpace->binary);
                }

                /* Populate the result object */
                value_zero(op1R);
                set_num_attributes(op1R, 2);
                op1R->attributes[0]->int_value = mod_no;
                op1R->attributes[1]->int_value = addr;
            }
            DISPATCH

            /* Regular Instructions */
            /* LOAD */
        START_INSTRUCTION(LOAD_REG_INT) CALC_DISPATCH(2)
            DEBUG("TRACE - LOAD R%d,%d\n", (int) REG_IDX(1), (int) op2I);
            set_int(op1R, op2I);
            DISPATCH

        START_INSTRUCTION(LOAD_REG_STRING) CALC_DISPATCH(2)
            DEBUG("TRACE - LOAD R%lu,\"%.*s\"\n",
                  REG_IDX(1), (int) (CONSTSTRING_OP(2))->string_len,
                  (CONSTSTRING_OP(2))->string);
            set_const_string(op1R, CONSTSTRING_OP(2));
            DISPATCH
        START_INSTRUCTION(LOAD_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - LOAD R%lu,R%lu\n",
                  REG_IDX(1), REG_IDX(2));
            op1R=op2R;
            DISPATCH

        START_INSTRUCTION(LOAD_INT_INT) CALC_DISPATCH(2) // TODO - review instruction
            DEBUG("TRACE - LOAD R%lu,R%lu\n",
                  (long) op1I, (long) op2I);
            REG_OP(op1I)=REG_OP(op2I);
        DISPATCH

    START_INSTRUCTION(LOAD_INT_REG) CALC_DISPATCH(2) // TODO - review instruction
    DEBUG("TRACE - LOAD R%lu, R%d\n",
          REG_IDX(1), (int) op2I);
          REG_OP(op1I)=op2R;
    DISPATCH

            /* Readline - Read a line from stdin to a register */
        START_INSTRUCTION(READLINE_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - READLINE R%lu\n", REG_IDX(1));
            {
                size_t pos = 0;
                int ch;
                op1R->string_length = 0;
                while ((ch = getchar()) != EOF) {
                    if (ch == '\n') break;
                    extend_string_buffer(op1R, pos + 1);
                    op1R->string_value[pos] = (char) ch;
                    pos++;
                }
                op1R->string_pos = 0;
#ifndef NUTF8
                op1R->string_char_pos = 0;
                op1R->string_chars = utf8nlen(op1R->string_value, op1R->string_length);
#endif
            }
            DISPATCH
	      
        START_INSTRUCTION(SAY_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - SAY R%lu\n", REG_IDX(1));
            mprintf("%.*s\n", (int) op1R->string_length, op1R->string_value);
            DISPATCH

        START_INSTRUCTION(SAY_STRING) CALC_DISPATCH(1)
            DEBUG("TRACE - SAY \"%.*s\"\n",
                  (int) op1S->string_len, op1S->string);
            mprintf("%.*s\n", (int) op1S->string_len, op1S->string);
            DISPATCH
	      
        START_INSTRUCTION(SAYX_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - SAYX R%lu\n", REG_IDX(1));
            mprintf("%.*s", (int) op1R->string_length, op1R->string_value);
            DISPATCH
	      
	START_INSTRUCTION(SAYX_STRING) CALC_DISPATCH(1)
            DEBUG("TRACE - SAYX \"%.*s\"\n",
                  (int) op1S->string_len, op1S->string);
            mprintf("%.*s", (int) op1S->string_len, op1S->string);
            DISPATCH

        START_INSTRUCTION(SCONCAT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SCONCAT R%lu,R%lu,R%lu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            string_sconcat(op1R, op2R, op3R);
            DISPATCH

        START_INSTRUCTION(CONCAT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - CONCAT R%lu,R%lu,R%lu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            string_concat(op1R, op2R, op3R);
            DISPATCH

        START_INSTRUCTION(SCONCAT_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - SCONCAT R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                  REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                  (CONSTSTRING_OP(3))->string);
            string_sconcat_var_const(op1R, op2R, op3S);
            DISPATCH

        START_INSTRUCTION(CONCAT_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - CONCAT R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                  REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                  (CONSTSTRING_OP(3))->string);
            string_concat_var_const(op1R, op2R, op3S);
            DISPATCH

        START_INSTRUCTION(SCONCAT_REG_STRING_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SCONCAT R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                  (int) (CONSTSTRING_OP(2))->string_len,
                  (CONSTSTRING_OP(2))->string, REG_IDX(3));
            string_sconcat_const_var(op1R, op2S, op3R);
            DISPATCH

        START_INSTRUCTION(CONCAT_REG_STRING_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - CONCAT R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                  (int) (CONSTSTRING_OP(2))->string_len,
                  (CONSTSTRING_OP(2))->string, REG_IDX(3));
            string_concat_const_var(op1R, op2S, op3R);
            DISPATCH

        START_INSTRUCTION(IMULT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IMULT R%lu,R%lu,R%lu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            REG_RETURN_INT(op2RI * op3RI)
            DISPATCH

        START_INSTRUCTION(IMULT_REG_REG_INT) {
            CALC_DISPATCH(3)
            DEBUG("TRACE - IMULT R%lu,R%lu,%lu\n", (long)REG_IDX(1),
                  (long)REG_IDX(2), (long)op3I);
            REG_RETURN_INT(op2RI * op3I)
            DISPATCH
        }

        START_INSTRUCTION(IADD_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IADD R%lu,R%lu,R%lu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            REG_RETURN_INT(op2RI + op3RI)
            DISPATCH

        START_INSTRUCTION(ISUB_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - ISUB R%lu,R%lu,R%lu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            REG_RETURN_INT(op2RI - op3RI)
            DISPATCH

        START_INSTRUCTION(IADD_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IADD R%lu,R%lu,%d\n", REG_IDX(1),
                  REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI + op3I)
            DISPATCH

        START_INSTRUCTION(ERASE_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - ERASE R%lu\n", REG_IDX(1));
            value_zero(op1R);
            DISPATCH

/* ====================================================================================
 * Decimal Plugin instructions
 * ====================================================================================
 */

/* ------------------------------------------------------------------------------------
 * DECPLNM_REG_REG_REG Get Decimal Plugin Name op1=name op2=description op3=version
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DECPLNM_REG_REG_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - DECPLNM R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_null_string(op1R, current_frame->decimal->base.name);
    set_null_string(op2R, current_frame->decimal->base.description);
    set_null_string(op3R, current_frame->decimal->base.version);
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  SETDGTS_REG Set Decimal Digits digits=op1
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(SETDGTS_REG) CALC_DISPATCH(1)
    DEBUG("TRACE - SETDGTS R%lu\n", REG_IDX(1));
    current_frame->decimal->setDigits(current_frame->decimal, op1R->int_value);
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  SETDGTS_INT Set Decimal Digits digits=op1"
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(SETDGTS_INT) CALC_DISPATCH(1)
    DEBUG("TRACE - SETDGTS %d\n", (int)op1I);
    current_frame->decimal->setDigits(current_frame->decimal, op1I);
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  GETDGTS_REG Get Decimal Digits op1=digits
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(GETDGTS_REG) CALC_DISPATCH(1)
    DEBUG("TRACE - GETDGTS R%lu\n", REG_IDX(1));
    op1R->int_value = (rxinteger)current_frame->decimal->getDigits(current_frame->decimal);
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  LOAD_REG_DECIMAL Load op1 with op2
 *  -----------------------------------------------------------------------------------
*/
    START_INSTRUCTION(LOAD_REG_DECIMAL) CALC_DISPATCH(2)
    DEBUG("TRACE - LOAD R%d,%s\n",(int)REG_IDX(1),op2S->string);
    current_frame->decimal->decimalFromString(current_frame->decimal, op1R, op2S->string);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * Convert decimal string to Decimal                              added August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(STOD_REG)
    CALC_DISPATCH(1)
    DEBUG("TRACE - STOD R%lu\n", REG_IDX(1));
    // Ensure the string is null terminated
    null_terminate_string_buffer(op1R);
    // Convert
    current_frame->decimal->decimalFromString(current_frame->decimal, op1R, op1R->string_value);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * Convert Integer to Decimal                                     added August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(ITOD_REG)
    CALC_DISPATCH(1)
    DEBUG("TRACE - ITOD R%lu\n", REG_IDX(1));
    // Convert
    current_frame->decimal->decimalFromInt(current_frame->decimal, op1R, op1R->int_value);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * Convert Float to Decimal                                       added August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FTOD_REG)
    CALC_DISPATCH(1)
    DEBUG("TRACE - FTOD R%lu\n",REG_IDX(1));
    current_frame->decimal->decimalFromDouble(current_frame->decimal, op1R, op1R->float_value);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * Convert Decimal to string                                        17. August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DTOS_REG)
    CALC_DISPATCH(1)
    DEBUG("TRACE - DTOS R%lu\n", REG_IDX(1));
    /* Determine how long the string needs to be */
    size_t string_size = current_frame->decimal->getRequiredStringSize(current_frame->decimal);
    /* Ensure the string buffer is big enough */
    extend_string_buffer(op1R, string_size);
    /* Convert */
    current_frame->decimal->decimalToString(current_frame->decimal, op1R, op1R->string_value);
    op1R->string_length = strlen(op1R->string_value);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * Convert Decimal to integer                                       17. August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DTOI_REG)
    CALC_DISPATCH(1)
    DEBUG("TRACE - DTOI R%lu\n", REG_IDX(1));
    current_frame->decimal->decimalToInt(current_frame->decimal, op1R, &op1R->int_value);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DTOF_REG  Convert Decimal Number to Float op1=f2dec(op2)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
*/
    START_INSTRUCTION(DTOF_REG) // label not yet defined
    CALC_DISPATCH(1);
    DEBUG("TRACE - DTOF_REG\n");
    current_frame->decimal->decimalToDouble(current_frame->decimal, op1R, &op1R->float_value);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * Decimal addition                                               added August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DADD_REG_REG_REG)
    CALC_DISPATCH(3)
    DEBUG("TRACE - DADD R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    current_frame->decimal->decimalAdd(current_frame->decimal, op1R, op2R, op3R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DADD_REG_REG_DECIMAL  Decimal Add (op1=op2+op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DADD_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DADD R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    // Convert the string to a decimal
    current_frame->decimal->decimalFromString(current_frame->decimal, op1R, op3S->string);
    current_frame->decimal->decimalAdd(current_frame->decimal, op1R, op2R, op1R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * Decimal subtraction                                            added August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DSUB_REG_REG_REG)
    CALC_DISPATCH(3)
    DEBUG("TRACE - DSUB R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    current_frame->decimal->decimalSub(current_frame->decimal, op1R, op2R, op3R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DSUB_REG_REG_DECIMAL  Decimal Subtract (op1=op2-op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DSUB_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DSUB R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    current_frame->decimal->decimalFromString(current_frame->decimal, op1R, op3S->string);
    current_frame->decimal->decimalSub(current_frame->decimal, op1R, op2R, op1R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DSUB_REG_REG_DECIMAL  Decimal Subtract (op1=op2-op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DSUB_REG_DECIMAL_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DSUB R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    current_frame->decimal->decimalFromString(current_frame->decimal, op1R, op2S->string);
    current_frame->decimal->decimalSub(current_frame->decimal, op1R, op1R, op3R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH

/* ------------------------------------------------------------------------------------
 * Decimal Multiply                                               added August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DMULT_REG_REG_REG)
    CALC_DISPATCH(3)
    DEBUG("TRACE - DMULT R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    current_frame->decimal->decimalMul(current_frame->decimal, op1R, op2R, op3R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DMULT_REG_REG_DECIMAL Decimal Multiply (op1=op2*op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DMULT_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DMULT R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    current_frame->decimal->decimalFromString(current_frame->decimal, op1R, op3S->string);
    current_frame->decimal->decimalMul(current_frame->decimal, op1R, op2R, op1R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * Decimal division                                               added August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DDIV_REG_REG_REG)
    CALC_DISPATCH(3)
    DEBUG("TRACE - DDIV R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    current_frame->decimal->decimalDiv(current_frame->decimal, op1R, op2R, op3R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DDIV_REG_DECIMAL_REG  Decimal Divide  (op1=op2/op3)                   pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DDIV_REG_DECIMAL_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DDIV R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    current_frame->decimal->decimalFromString(current_frame->decimal, op1R, op2S->string);
    current_frame->decimal->decimalDiv(current_frame->decimal, op1R, op1R, op3R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DDIV_REG_REG_DECIMAL  Decimal Divide (op1=op2/op3)                   pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DDIV_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DDIV R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    current_frame->decimal->decimalFromString(current_frame->decimal, op1R, op3S->string);
    current_frame->decimal->decimalDiv(current_frame->decimal, op1R, op2R, op1R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DEQ_REG_REG_REG  Decimal Equals op1=(op2==op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DEQ_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DEQ R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) == 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DNE_REG_REG_REG  Decimal Not equals op1=(op2!=op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DNE_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DNE R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) != 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DGT_REG_REG_REG  Decimal Greater than op1=(op2>op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DGT_REG_REG_REG) // label not yet defined
    CALC_DISPATCH(3);
    DEBUG("TRACE - DGT R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) > 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DGTE_REG_REG_REG  Decimal Greater than equals op1=(op2>=op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DGTE_REG_REG_REG) // label not yet defined
    CALC_DISPATCH(3);
    DEBUG("TRACE - DGTE R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) >= 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DLT_REG_REG_REG  Decimal Less than op1=(op2<op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DLT_REG_REG_REG) // label not yet defined
    CALC_DISPATCH(3);
    DEBUG("TRACE - DLT R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) < 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DLTE_REG_REG_REG  Decimal Less than equals op1=(op2<=op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DLTE_REG_REG_REG) // label not yet defined
    CALC_DISPATCH(3);
    DEBUG("TRACE - DLTE R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) <= 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DEQ_REG_REG_DECIMAL  Decimal Equals op1=(op2==op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DEQ_REG_REG_DECIMAL) // label not yet defined
    CALC_DISPATCH(3);
    DEBUG("TRACE - DEQ R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op2R, op3S->string) == 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DLTBR_ID_REG_REG  Decimal Less than if (op2<op3) goto op1              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DLTBR_ID_REG_REG)
    CALC_DISPATCH(3); // This branch prediction for the condition not being met
    DEBUG("TRACE - DLTBR 0x%x,0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    if (current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) < 0) {
        next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
        CALC_DISPATCH_MANUAL
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DGTBR_ID_REG_REG  Decimal Greater than if (op2>op3) goto op1              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DGTBR_ID_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DLTBR 0x%x,0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    if (current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) > 0) {
        next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
        CALC_DISPATCH_MANUAL
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DEQBR_ID_REG_REG  Decimal Equal if (op2=op3) goto op1              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DEQBR_ID_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DEQBR 0x%x,0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    if (current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) == 0) {
        next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
        CALC_DISPATCH_MANUAL
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DNE_REG_REG_DECIMAL  Decimal Not equals op1=(op2!=op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DNE_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DNE R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op2R, op3S->string) != 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DGT_REG_REG_DECIMAL  Decimal Greater than op1=(op2>op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DGT_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DGT R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op2R, op3S->string) > 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DGT_REG_DECIMAL_REG  Decimal Greater than op1=(op2>op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DGT_REG_DECIMAL_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DGT R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op3R, op2S->string) < 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DGTE_REG_REG_DECIMAL  Decimal Greater than equals op1=(op2>=op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DGTE_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DGTE R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op2R, op3S->string) >= 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DGTE_REG_DECIMAL_REG  Decimal Greater than equals op1=(op2>=op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DGTE_REG_DECIMAL_REG) // label not yet defined
    CALC_DISPATCH(3);
    DEBUG("TRACE - DGTE R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op3R, op2S->string) <= 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DLT_REG_REG_DECIMAL  Decimal Less than op1=(op2<op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DLT_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DLT R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op2R, op3S->string) < 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DLT_REG_DECIMAL_REG  Decimal Less than op1=(op2<op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DLT_REG_DECIMAL_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DLT R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op3R, op2S->string) > 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DLTE_REG_REG_DECIMAL  Decimal Less than equals op1=(op2<=op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DLTE_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DLTE R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op2R, op3S->string) <= 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DLTE_REG_DECIMAL_REG  Decimal Less than equals op1=(op2<=op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DLTE_REG_DECIMAL_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DLTE R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op3R, op2S->string) >= 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DCOPY_REG_REG  Copy Decimal op2 to op1              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DCOPY_REG_REG) // label not yet defined
    CALC_DISPATCH(2);
    DEBUG("TRACE - DCOPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
    if (op2R->decimal_value == NULL) {
        // Signal error
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "No Source Decimal Value")
        DISPATCH
    }
    if (op1R == op2R) {
        // NOP
        DISPATCH
    }
    if (op1R->decimal_value == NULL) {
        // Allocate storage for the decimal
        op1R->decimal_value = malloc(op2R->decimal_value_length);
        op1R->decimal_buffer_length = op2R->decimal_value_length;
    }
    else if (op1R->decimal_buffer_length < op2R->decimal_value_length) {
        // Reallocate storage for the decimal
        op1R->decimal_value = realloc(op1R->decimal_value, op2R->decimal_value_length);
        op1R->decimal_buffer_length = op2R->decimal_value_length;
    }
    memcpy(op1R->decimal_value, op2R->decimal_value, op2R->decimal_value_length);
    op1R->decimal_value_length = op2R->decimal_value_length;
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DSEX_REG  Decimal op1 = -op1 (sign change)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DSEX_REG)
    CALC_DISPATCH(1);
    DEBUG("TRACE - DSEX R%lu\n", REG_IDX(1));
    if (op1R->decimal_value == NULL) {
        // Signal error
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "No Source Decimal Value")
        DISPATCH
    }
    current_frame->decimal->decimalNeg(current_frame->decimal, op1R, op1R);
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DPOW_REG_REG_REG  op1=op2**op3              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DPOW_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DPOW R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    current_frame->decimal->decimalPow(current_frame->decimal, op1R, op2R, op3R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DPOW_REG_REG_DECIMAL  op1=op2**op3              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DPOW_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DPOW R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    current_frame->decimal->decimalFromString(current_frame->decimal, op1R, op3S->string);
    current_frame->decimal->decimalPow(current_frame->decimal, op1R, op2R, op1R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DPOW_REG_DECIMAL_REG  op1=op2**op3              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DPOW_REG_DECIMAL_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DPOW R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    current_frame->decimal->decimalFromString(current_frame->decimal, op1R, op2S->string);
    current_frame->decimal->decimalPow(current_frame->decimal, op1R, op1R, op3R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * DEXTR_REG_REG_REG Extract decimal to string coefficient and decimal exponent integer
 * R3 contains the decimal value
 * R1 will store the coefficient as a string (or nan, inf, -inf)
 * R2 will store the decimal exponent as an integer
 * Output normalised, rounded to the digits setting decimal places, and
 * trimmed of trailing zeros
 * This instruction is designed to allow the user to format the float as they wish
 */
    START_INSTRUCTION(DEXTR_REG_REG_REG)
    CALC_DISPATCH(3)
    DEBUG("TRACE - DEXTR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    prep_string_buffer(op1R, current_frame->decimal->getRequiredStringSize(current_frame->decimal));
    current_frame->decimal->decimalExtract(current_frame->decimal, op1R->string_value, &(op2R->int_value), op3R);
    op1R->string_length = strlen(op1R->string_value);
    DISPATCH
/* ====================================================================================
 * End of Decimal Plugin instructions
 * ====================================================================================
 */
       START_INSTRUCTION(CALL_FUNC) CALC_DISPATCH(1)
            /* New stackframe - grabbing procedure object from the caller frame */
            {
                proc_constant *called_function = PROC_OP(1);
                DEBUG("TRACE - CALL %s()\n", called_function->name);
                if (called_function->start == SIZE_MAX) {
                    SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, called_function->name)
                    DISPATCH
                }
                if (called_function->binarySpace == 0) {
                    /* This is a native plugin function */
                    rxpa_callfunc((void *) (called_function->start), 0, NULL, NULL, signal_value);
                    INTERRUPT_FROM_RXPA_SIGNAL(signal_value);
                } else {
                    /* This is a CREXX Procedure */
                    current_frame = frame_f(called_function, 0, current_frame, next_pc, 0);
                    /* Prepare dispatch to procedure as early as possible */
                    next_pc = &(current_frame->procedure->binarySpace->binary[called_function->start]);
                    CALC_DISPATCH_MANUAL
                    /* No Arguments - so nothing to do */
                }
            }
            DISPATCH

        START_INSTRUCTION(CALL_REG_FUNC) CALC_DISPATCH(2)
            {
                /* Clear target return value register */
                value_zero(op1R);

                proc_constant *called_function = PROC_OP(2);
                DEBUG("TRACE - CALL R%lu,%s()\n", REG_IDX(1), called_function->name);

                if (called_function->start == SIZE_MAX) {
                    SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, called_function->name);
                    DISPATCH
                }

                if (called_function->binarySpace == 0) {
                    /* This is a native plugin function */
                    rxpa_callfunc((void *) (called_function->start), 0, NULL, op1R, signal_value);
                    INTERRUPT_FROM_RXPA_SIGNAL(signal_value);
                } else {
                    /* This is a CREXX Procedure */
                    /* New stackframe - grabbing a procedure object from the caller frame */
                    current_frame = frame_f(called_function, 0, current_frame, next_pc, op1R);

                    /* Prepare dispatch to procedure as early as possible */
                    next_pc = &(current_frame->procedure->binarySpace->binary[called_function->start]);
                    CALC_DISPATCH_MANUAL
                    /* No Arguments - so nothing to do */
                }
            }
            DISPATCH

        START_INSTRUCTION(CALL_REG_FUNC_REG) CALC_DISPATCH(3)
            {
                proc_constant *called_function = PROC_OP(2);
                DEBUG("TRACE - CALL R%lu,%s,R%lu\n", REG_IDX(1), called_function->name, REG_IDX(3));
                if (called_function->start == SIZE_MAX) {
                    SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, called_function->name);
                    DISPATCH
                }

                if (called_function->binarySpace == 0) {
                    /* This is a native plugin function */
                    rxpa_callfunc((void *) (called_function->start), op3R->int_value, (&(op3R)) + 1, op1R,
                                  signal_value);
                    INTERRUPT_FROM_RXPA_SIGNAL(signal_value);
                } else {
                    /* This is a CREXX Procedure */
                    /* New stackframe - grabbing a procedure object from the caller frame */
                    current_frame = frame_f(called_function, (int) op3R->int_value, current_frame, next_pc, op1R);
                    /* Prepare dispatch to procedure as early as possible */
                    next_pc = &(current_frame->procedure->binarySpace->binary[called_function->start]);
                    CALC_DISPATCH_MANUAL

                    /* Arguments - complex lets never have to change this code! */
                    size_t j =
                            current_frame->procedure->binarySpace->globals +
                            current_frame->procedure->locals + 1; /* Callee register index */
                    size_t k = (pc + 3)->index + 1; /* Caller register index */
                    size_t i;
                    for (i = 0;
                         i < (current_frame->parent->locals[(pc + 3)->index])->int_value;
                         i++, j++, k++) {
                        current_frame->locals[j] = current_frame->parent->locals[k];
                    }
                }
            }
            DISPATCH

        START_INSTRUCTION(DCALL_REG_REG_REG) CALC_DISPATCH(3)
            {
                /* Function pointer is in register 2 */
                proc_constant *called_function = (proc_constant *) op2R->int_value;
                DEBUG("TRACE - DCALL R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
                if (called_function->start == SIZE_MAX) {
                    SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, called_function->name);
                    DISPATCH
                }

                if (called_function->binarySpace == 0) {
                    /* This is a native plugin function */
                    rxpa_callfunc((void *) (called_function->start), op3R->int_value, (&(op3R)) + 1, op1R,
                                  signal_value);
                    INTERRUPT_FROM_RXPA_SIGNAL(signal_value);
                } else {
                    /* This is a CREXX Procedure */
                    current_frame = frame_f(called_function, (int) op3R->int_value, current_frame, next_pc, op1R);

                    /* Prepare dispatch to procedure as early as possible */
                    next_pc = &(current_frame->procedure->binarySpace->binary[called_function->start]);
                    CALC_DISPATCH_MANUAL

                    /* Arguments - complex lets never have to change this code! */
                    size_t j =
                            current_frame->procedure->binarySpace->globals +
                            current_frame->procedure->locals + 1; /* Callee register index */
                    size_t k = (pc + 3)->index + 1; /* Caller register index */
                    size_t i;
                    for (i = 0;
                         i < (current_frame->parent->locals[(pc + 3)->index])->int_value;
                         i++, j++, k++) {
                        current_frame->locals[j] = current_frame->parent->locals[k];
                    }
                }
            }
            DISPATCH

        START_INSTRUCTION(RET)
            DEBUG("TRACE - RET\n");
            {
                /* Where we return to */
                next_pc = current_frame->return_pc;
                unsigned char is_interrupt = current_frame->is_interrupt;
                /* back to the parent's stack frame */
                temp_frame = current_frame;
                current_frame = current_frame->parent;
                if (!current_frame) {
                    DEBUG("TRACE - RETURNING FROM MAIN()\n");
                    /* Free Argument Values a1... */
                    int i, j;
                    /* a0 is the number of args */
                    int num_args = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                               temp_frame->procedure->locals]->int_value;
                    for (i = 0, j = temp_frame->procedure->binarySpace->globals + temp_frame->procedure->locals + 1;
                         i < num_args;
                         i++, j++) {
                        clear_value(temp_frame->baselocals[j]);
                        free(temp_frame->baselocals[j]);
                    }
                    rc = 0;
                    free_frame(temp_frame);
                    arguments_array = 0; /* We have freed it in the loop above */
                    goto interprt_finished;
                }
                free_frame(temp_frame);
                CALC_DISPATCH_MANUAL
                if (is_interrupt == RXSIGNAL_BREAKPOINT) {
                    pc = next_pc;
                    END_INTERRUPT // Breakpoints are not cleared, so we bypass the interrupt check
                }
                DISPATCH
            }

        START_INSTRUCTION(RET_REG)
            DEBUG("TRACE - RET R%lu\n", REG_IDX(1));
            {
                /* Where we return to */
                next_pc = current_frame->return_pc;
                unsigned char is_interrupt = current_frame->is_interrupt;
                /* Set the result register */
                if (current_frame->return_reg) {
                    if (REG_IDX(1) >= current_frame->procedure->locals)
                        copy_value(current_frame->return_reg,
                                   op1R); /* Must do a copy from an argument or global because ... */
                    else
                        move_value(current_frame->return_reg,
                                   op1R); /* ... the faster move deletes the source which is ok for locals */
                }
                /* back to the parents stack frame */
                temp_frame = current_frame;
                current_frame = current_frame->parent;
                if (!current_frame) rc =
                                            (int) (temp_frame->locals[(pc + 1)
                                                    ->index])
                                                    ->int_value; /* Exiting - grab the int rc */
                if (!current_frame) {
                    DEBUG("TRACE - RETURNING FROM MAIN()\n");
                    /* Free Argument Values a1... */
                    int i, j;
                    /* a0 is the number of args */
                    int num_args = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                               temp_frame->procedure->locals]->int_value;
                    for (i = 0, j =
                                        temp_frame->procedure->binarySpace
                                                ->globals +
                                        temp_frame->procedure->locals + 1;
                            i < num_args;
                            i++, j++) {
                        clear_value(temp_frame->baselocals[j]);
                        free(temp_frame->baselocals[j]);
                    }
                    free_frame(temp_frame);
                    arguments_array = 0; /* We have freed it in the loop above */
                    goto interprt_finished;
                }
                free_frame(temp_frame);
                CALC_DISPATCH_MANUAL
                if (is_interrupt == RXSIGNAL_BREAKPOINT) {
                    pc = next_pc;
                    END_INTERRUPT // Breakpoints are not cleared, so we bypass the interrupt check
                }
                DISPATCH
            }

        START_INSTRUCTION(RET_INT)
            DEBUG("TRACE - RET %d\n", (int)op1I);
            {
                /* Where we return to */
                next_pc = current_frame->return_pc;
                unsigned char is_interrupt = current_frame->is_interrupt;
                /* Set the result register */
                if (current_frame->return_reg)
                    current_frame->return_reg->int_value = op1I;
                /* back to the parents stack frame */
                temp_frame = current_frame;
                current_frame = current_frame->parent;
                if (!current_frame) {
                    DEBUG("TRACE - RETURNING FROM MAIN()\n");
                    /* Free Argument Values a1... */
                    int i, j;
                    /* a0 is the number of args */
                    int num_args = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                               temp_frame->procedure->locals]->int_value;
                    for (i = 0, j =
                                        temp_frame->procedure->binarySpace
                                                ->globals +
                                        temp_frame->procedure->locals + 1;
                            i < num_args;
                            i++, j++) {
                        clear_value(temp_frame->baselocals[j]);
                        free(temp_frame->baselocals[j]);
                    }
                    rc = (int) op1I;
                    free_frame(temp_frame);
                    arguments_array = 0; /* We have freed it in the loop above */
                    goto interprt_finished;
                }
                free_frame(temp_frame);
                CALC_DISPATCH_MANUAL
                if (is_interrupt == RXSIGNAL_BREAKPOINT) {
                    pc = next_pc;
                    END_INTERRUPT // Breakpoints are not cleared, so we bypass the interrupt check
                }
                DISPATCH
            }
/* ------------------------------------------------------------------------------------
 *  RET_FLOAT                                                        pej 12. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(RET_FLOAT)
            DEBUG("TRACE - RET %.15g\n", op1F);
            {
                /* Where we return to */
                next_pc = current_frame->return_pc;
                unsigned char is_interrupt = current_frame->is_interrupt;
                /* Set the result register */
                if (current_frame->return_reg)
                    current_frame->return_reg->float_value = op1F;
                /* back to the parents stack frame */
                temp_frame = current_frame;
                current_frame = current_frame->parent;
                if (!current_frame) {
                    DEBUG("TRACE - RETURNING FROM MAIN()\n");
                    /* Free Argument Values a1... */
                    int i, j;
                    /* a0 is the number of args */
                    int num_args = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                               temp_frame->procedure->locals]->int_value;
                    for (i = 0, j = temp_frame->procedure->binarySpace->globals +
                                    temp_frame->procedure->locals + 1;
                            i < num_args;
                            i++, j++) {
                        clear_value(temp_frame->baselocals[j]);
                        free(temp_frame->baselocals[j]);
                    }
                    rc = 0;
                    free_frame(temp_frame);
                    arguments_array = 0; /* We have freed it in the loop above */
                    goto interprt_finished;
                }
                free_frame(temp_frame);
                CALC_DISPATCH_MANUAL
                if (is_interrupt == RXSIGNAL_BREAKPOINT) {
                    pc = next_pc;
                    END_INTERRUPT // Breakpoints are not cleared, so we bypass the interrupt check
                }
                DISPATCH
            }

            /* ------------------------------------------------------------------------------------
            *  RET_STRING                                                        pej 12. April 2021
            *  -----------------------------------------------------------------------------------
            */
        START_INSTRUCTION(RET_STRING)
            DEBUG("TRACE - RET \"%.*s\"\n", (int)op1S->string_len, op1S->string);
            {
                /* Where we return to */
                next_pc = current_frame->return_pc;
                unsigned char is_interrupt = current_frame->is_interrupt;
                /* Set the result register */
                if (current_frame->return_reg)
                    set_const_string(current_frame->return_reg, CONSTSTRING_OP(1));
                /* back to the parents stack frame */
                temp_frame = current_frame;
                current_frame = current_frame->parent;
                if (!current_frame) {
                    DEBUG("TRACE - RETURNING FROM MAIN()\n");
                    /* Free Argument Values a1... */
                    int i, j;
                    /* a0 is the number of args */
                    int num_args = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                               temp_frame->procedure->locals]->int_value;
                    for (i = 0, j = temp_frame->procedure->binarySpace->globals +
                            temp_frame->procedure->locals + 1;
                            i < num_args;
                            i++, j++) {
                        clear_value(temp_frame->baselocals[j]);
                        free(temp_frame->baselocals[j]);
                    }
                    rc = 0;
                    free_frame(temp_frame);
                    goto interprt_finished;
                }
                free_frame(temp_frame);
                CALC_DISPATCH_MANUAL
                if (is_interrupt == RXSIGNAL_BREAKPOINT) {
                    pc = next_pc;
                    END_INTERRUPT // Breakpoints are not cleared, so we bypass the interrupt check
                }
                DISPATCH
            }

        START_INSTRUCTION(MOVE_REG_REG) CALC_DISPATCH(2) /* Deprecated */
            DEBUG("TRACE - MOVE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            move_value(op1R, op2R);
            DISPATCH

        START_INSTRUCTION(SWAP_REG_REG) CALC_DISPATCH(2) /* Deprecated */
            DEBUG("TRACE - SWAP R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                value *v_temp;
                v_temp = op1R;
                op1R = op2R;
                op2R = v_temp;
            }
            DISPATCH

        /* Link attribute op3 of op2 to op1 */
        START_INSTRUCTION(LINKATTR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - LINKATTR R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            if (op3R->int_value < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if (op3R->int_value >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op1R = op2R->attributes[op3R->int_value];
            DISPATCH

        /* Link attribute op3 of op2 to op1 */
        START_INSTRUCTION(LINKATTR_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - LINKATTR R%lu,R%lu,%d\n", REG_IDX(1), REG_IDX(2), (int)op3I);
            if ((int)op3I < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if ((int)op3I >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op1R = op2R->attributes[(int)op3I];
            DISPATCH

        /* Link attribute op3 (1 base) of op2 to op1 */
        START_INSTRUCTION(LINKATTR1_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - LINKATTR1 R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            if (op3R->int_value - 1 < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if (op3R->int_value - 1 >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op1R = op2R->attributes[op3R->int_value - 1];
            DISPATCH

        /* Link attribute op3 (1 base) of op2 to op1 */
        START_INSTRUCTION(LINKATTR1_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - LINKATTR1 R%lu,R%lu,%d\n", REG_IDX(1), REG_IDX(2), (int)op3I);
            if ((int)op3I - 1 < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if ((int)op3I - 1 >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op1R = op2R->attributes[(int)op3I - 1];
            DISPATCH

        /* Link op3 to attribute op1 of op2 */
        START_INSTRUCTION(LINKTOATTR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - LINKTOATTR R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            if (op1R->int_value < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if (op1R->int_value >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op2R->attributes[op1R->int_value] = op3R;
            DISPATCH

        /* Link op3 to attribute op1 of op2 */
        START_INSTRUCTION(LINKTOATTR_INT_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - LINKTOATTR %d,R%lu,R%lu\n", (int)op1I, REG_IDX(2), REG_IDX(3));
            if ((int)op1I < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if ((int)op1I >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op2R->attributes[(int)op1I] = op3R;
            DISPATCH

        /* Link op3 to attribute op1 (1 base) of op2 */
        START_INSTRUCTION(LINKTOATTR1_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - LINKTOATTR1 R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            if (op1R->int_value - 1 < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if (op1R->int_value - 1 >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op2R->attributes[op1R->int_value - 1] = op3R;
            DISPATCH

        /* Link op3 to attribute op1 (1 base) of op2 */
        START_INSTRUCTION(LINKTOATTR1_INT_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - LINKTOATTR1 %d,R%lu,R%lu\n", (int)op1I, REG_IDX(2), REG_IDX(3));
            if ((int)op1I - 1 < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if ((int)op1I - 1 >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op2R->attributes[(int)op1I -1] = op3R;
            DISPATCH

        /* Unlink attribute op1 of op2 */
        START_INSTRUCTION(UNLINKATTR_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - UNLINKATTR R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            if (op1R->int_value < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if (op1R->int_value >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op2R->attributes[op1R->int_value] = op2R->unlinked_attributes[op1R->int_value];
            DISPATCH

        /* Unlink attribute op1 of op2 */
        START_INSTRUCTION(UNLINKATTR_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - UNLINKATTR %d,R%lu\n", (int)op1I, REG_IDX(2));
            if ((int)op1I < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if ((int)op1I >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op2R->attributes[(int)op1I] = op2R->unlinked_attributes[(int)op1I];
            DISPATCH

        /* Unlink attribute op1 (1 base) of op2 */
        START_INSTRUCTION(UNLINKATTR1_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - UNLINKATTR1 R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            if (op1R->int_value - 1 < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if (op1R->int_value - 1 >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op2R->attributes[op1R->int_value - 1] = op2R->unlinked_attributes[op1R->int_value - 1];
            DISPATCH

        /* Unlink attribute op1 (1 base) of op2 */
        START_INSTRUCTION(UNLINKATTR1_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - UNLINKATTR1 %d,R%lu\n", (int)op1I, REG_IDX(2));
            if ((int)op1I - 1 < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if ((int)op1I - 1 >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op2R->attributes[(int)op1I - 1] = op2R->unlinked_attributes[(int)op1I - 1];
            DISPATCH

            /* Link op2 to op1 */
        START_INSTRUCTION(LINK_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - LINK R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            op1R = op2R;
            DISPATCH

        /* Link parent-frame-register[op2] to op1 */
        START_INSTRUCTION(METALINKPREG_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - METALINKPREG R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            op1R = current_frame->parent->locals[op2R->int_value];
            DISPATCH

        /* Unlink op1 */
        START_INSTRUCTION(UNLINK_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - UNLINK R%lu\n", REG_IDX(1));
            op1R = (current_frame->baselocals[(pc + 1)->index]);
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  GETATTRS_REG_REG Get Number Attributes op1 = op2.num_attributes
         *  ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(GETATTRS_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - GETATTRS R%lu,R%lu\n", REG_IDX(1),REG_IDX(2));
            REG_RETURN_INT(op2R->num_attributes)
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  GETATTRS_REG_REG_INT Get Number Attributes op1 = op2.num_attributes + op3
         *  ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(GETATTRS_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - GETATTRS R%lu,R%lu,%d\n", REG_IDX(1),REG_IDX(2),(int)op3I);
            REG_RETURN_INT(op2R->num_attributes + op3I)
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  SETATTRS_REG_REG Set Number Attributes op1.num_attributes = op2
         * ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(SETATTRS_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SETATTRS R%lu,R%lu\n", REG_IDX(1),REG_IDX(2));
            set_num_attributes(op1R, op2RI);
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  SETATTRS_REG_REG Set Number Attributes op1.num_attributes = op2
         * ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(SETATTRS_REG_INT) CALC_DISPATCH(2)
            DEBUG("TRACE - SETATTRS R%lu,%d\n", REG_IDX(1),(int)op2I);
            set_num_attributes(op1R, op2I);
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  SETATTRS_REG_REG_INT Set Number Attributes op1.num_attributes = op2 + op3
         * ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(SETATTRS_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - SETATTRS R%lu,R%lu,%d\n", REG_IDX(1),REG_IDX(2), (int)op3I);
            set_num_attributes(op1R, op2RI + op3I);
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  SETATTRS_REG_INT_INT Set Number Attributes op1.num_attributes = op2 + op3
         * ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(SETATTRS_REG_INT_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - SETATTRS R%lu,%d,%d\n", REG_IDX(1),(int)op2I, (int)op3I);
            set_num_attributes(op1R, op2I + op3I);
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  MINATTRS_REG_REG Ensure min number attributes op1.num_attributes >= op2
         * ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(MINATTRS_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - MINATTRS R%lu,R%lu\n", REG_IDX(1),REG_IDX(2));
            if (op2RI > op1R->num_attributes) {
                /* We need to add attributes */
                if (op2RI > op1R->max_num_attributes) {
                    /* We need to increase the size of the buffer */
                    /* Make the buffer double sized by setting the number of attributes */
                    set_num_attributes(op1R, op2RI * 2);
                }
                /* Set the number of attributes to the requested number */
                set_num_attributes(op1R, op2RI);
            }
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  MINATTRS_REG_REG Ensure min number attributes op1.num_attributes >= op2
         * ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(MINATTRS_REG_INT) CALC_DISPATCH(2)
            DEBUG("TRACE - MINATTRS R%lu,%d\n", REG_IDX(1),(int)op2I);
            if (op2I > op1R->num_attributes) {
                /* We need to add attributes */
                if (op2I > op1R->max_num_attributes) {
                    /* We need to increase the size of the buffer */
                    /* Make the buffer double sized by setting the number of attributes */
                    set_num_attributes(op1R, op2I * 2);
                }
                /* Set the number of attributes to the requested number */
                set_num_attributes(op1R, op2I);
            }
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  MINATTRS_REG_REG_INT Ensure min number attributes op1.num_attributes >= op2 + op3
         * ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(MINATTRS_REG_REG_INT) CALC_DISPATCH(3)
        DEBUG("TRACE - MINATTRS R%lu,R%lu,%d\n", REG_IDX(1),REG_IDX(2),(int)op3I);
        if (op2RI + op3I > op1R->num_attributes) {
            /* We need to add attributes */
            if (op2RI + op3I > op1R->max_num_attributes) {
                /* We need to increase the size of the buffer */
                /* Make the buffer double sized by setting the number of attributes */
                set_num_attributes(op1R, (op2RI + op3I) * 2);
            }
            /* Set the number of attributes to the requested number */
            set_num_attributes(op1R, op2RI + op3I);
        }
        DISPATCH

        /* ------------------------------------------------------------------------------------
         *  MINATTRS_REG_REG_INT Ensure min number attributes op1.num_attributes >= op2 + op3
         * ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(MINATTRS_REG_INT_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - MINATTRS R%lu,%d,%d\n", REG_IDX(1),(int)op2I,(int)op3I);
            if (op2I + op3I > op1R->num_attributes) {
                /* We need to add attributes */
                if (op2I + op3I > op1R->max_num_attributes) {
                    /* We need to increase the size of the buffer */
                    /* Make the buffer double sized by setting the number of attributes */
                    set_num_attributes(op1R, (op2I + op3I) * 2);
                }
                /* Set the number of attributes to the requested number */
                set_num_attributes(op1R, op2I + op3I);
            }
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  GETABUFS_REG_REG Get attribute buffer size op1 = op2.max_attributes
         *  ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(GETABUFS_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - GETABUFS R%lu,R%lu\n", REG_IDX(1),REG_IDX(1));
            REG_RETURN_INT(op2R->max_num_attributes)
            DISPATCH

        START_INSTRUCTION(DEC0) CALC_DISPATCH(0)
            /* TODO This is really idec0 - i.e. it does not prime the int */
            DEBUG("TRACE - DEC0\n");
            (current_frame->locals[0]->int_value)--;
            DISPATCH

            /* ------------------------------------------------------------------------------------
         *  DEC1   R1--                                                       pej 7. April 2021
         *  -----------------------------------------------------------------------------------
         */
        START_INSTRUCTION(DEC1) CALC_DISPATCH(0)
            /* TODO This is really idec1 - i.e. it does not prime the int */
            DEBUG("TRACE - DEC1\n");
            (current_frame->locals[1]->int_value)--;
            DISPATCH

            /* ------------------------------------------------------------------------------------
            *  DEC2   op2R--                                                       pej 7. April 2021
            *  -----------------------------------------------------------------------------------
            */
        START_INSTRUCTION(DEC2) CALC_DISPATCH(0)
            /* TODO This is really idec2 - i.e. it does not prime the int */
            DEBUG("TRACE - DEC2\n");
            (current_frame->locals[2]->int_value)--;
            DISPATCH

        START_INSTRUCTION(DEC_REG) CALC_DISPATCH(1)
            /* TODO This is really idec reg - i.e. it does not prime the int */
            DEBUG("TRACE - DEC R%lu\n", REG_IDX(1));
            (current_frame->locals[REG_IDX(1)]->int_value)--;
            DISPATCH

        START_INSTRUCTION(BR_ID)
            DEBUG("TRACE - BR 0x%x\n", (unsigned int)REG_IDX(1));
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
            DISPATCH

            /* For these we optimise for condition to NOT be met because in a loop
             * these ae used to jump out of the loop when the end condition it met
             * (and every little bit helps to improve performance!)
             */

        START_INSTRUCTION(BRT_ID_REG) CALC_DISPATCH(2) /* i.e. if the condition is not met - this helps the
                                the real CPUs branch prediction (in theory) */
            DEBUG("TRACE - BRT 0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2));
            if (op2RI) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
            DISPATCH

        START_INSTRUCTION(BRF_ID_REG) CALC_DISPATCH(2) /* i.e. if the condition is not met - this helps the
                                  the real CPUs branch prediction (in theory) */
            DEBUG("TRACE - BRF 0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2));
            if (!(op2RI)) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
            DISPATCH

        START_INSTRUCTION(BRTF_ID_ID_REG)
            DEBUG("TRACE - BRTF 0x%x,0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            if (op3RI) next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            else next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(2);
            CALC_DISPATCH_MANUAL
            DISPATCH

        START_INSTRUCTION(TIME_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - TIME R%d\n", (int)REG_IDX(1));
            {
                struct timeval tv;
                tzset();
                gettimeofday(&tv, NULL);
                REG_RETURN_INT(tv.tv_sec - timezone)
            }
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  XTIME return time properties                                  pej 02. December 2021
 * ------------------------------------------------------------------------------------
 */
        START_INSTRUCTION(XTIME_REG_STRING) CALC_DISPATCH(2)
        DEBUG("TRACE - XTIME R%d,\"%s\"\n", (int)REG_IDX(1),(CONSTSTRING_OP(2))->string);

            tzset();
            switch ((CONSTSTRING_OP(2))->string[0]) {
                case 'Z':
                    tzset();
                    op1R->int_value  = timezone;
                    break;
                case 'T':  op1R->int_value  = (rxinteger)clock(); break;
                case 'C':  op1R->int_value  = CLOCKS_PER_SEC; break;
                case 'N':  {
                     prep_string_buffer(op1R,2*SMALLEST_STRING_BUFFER_LENGTH); // Large enough for both time zone names
                     op1R->string_length = snprintf(op1R->string_value,2*SMALLEST_STRING_BUFFER_LENGTH,"%s;%s",tzname[0],tzname[1]);
                     op1R->string_pos = 0;
                     break;  // time zone names
                }
                case 'U':  {
                     time_t ctime;
                     rxinteger tm;
                     struct timeval tv;
                     struct tm *tmdata;
                     ctime = time(NULL);
                     tmdata = localtime(&ctime);
                     tzset();
                     tm=((tmdata->tm_hour * 3600) + (tmdata->tm_min  * 60) + (tmdata->tm_sec))+timezone;
                     gettimeofday(&tv, NULL);
                     op1R->int_value = tm*1000000+tv.tv_usec;
                     break;  // UTC Time
                }
            }
            DISPATCH

/* ---------------------------------------------------------------------------------
 *  MTIME get time of the day in microseconds                      pej 31. October 2021
 * ------------------------------------------------------------------------------------
 */
        START_INSTRUCTION(MTIME_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - MTIME R%d\n", (int)REG_IDX(1));
            {
                rxinteger tm;
                struct timeval tv;
                //struct timezone tz;
                time_t	ctime;
                struct tm *tmdata;

                ctime = time(NULL);
                tmdata = localtime(&ctime);
                tm =
                        ((tmdata->tm_hour * 3600) + (tmdata->tm_min * 60) +
                        (tmdata->tm_sec));
                gettimeofday(&tv, NULL);
                REG_RETURN_INT(tm * 1000000 + tv.tv_usec)
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  TRIMR  Trim right                                                 pej 7. April 2021
 * ------------------------------------------------------------------------------------
 */
        START_INSTRUCTION(TRIMR_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - TRIMR (DEPRECATED) R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            {
                int i = op1R->string_length - 1;
                while (i >= 0 && op1R->string_value[i] == ' ') {
                    op1R->string_value[i] = '\0';
                    i--;
                }
                op1R->string_length = i + 1;
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  TRIML  Trim left                                                  pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(TRIML_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - TRIML (DEPRECATED) R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            /* TODO - UTF etc */
            {
                int j = op1R->string_length - 1;
                int i = 0;
                while (i <= j && op1R->string_value[i] == ' ') i++;

                if (i >= j) {
                    op1R->string_length = 0;
                    op1R->string_value[0] = '\0';
                } else {
                    op1R->string_length = op1R->string_length - i;
                    memcpy(op1R->string_value, op1R->string_value + i,
                           op1R->string_length);
                    op1R->string_value[op1R->string_length] = '\0';
                }
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  INC0   R0++                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INC0) CALC_DISPATCH(0)
            DEBUG("TRACE - INC0\n");
            REG_VAL(0)->int_value++;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  INC1   R1++                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INC1) CALC_DISPATCH(0)
            DEBUG("TRACE - INC1\n");
            REG_VAL(1)->int_value++;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  INC2   op2R++                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INC2) CALC_DISPATCH(0)
            DEBUG("TRACE - INC2\n");
            REG_VAL(2)->int_value++;
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  ISEX   op1 = -op1  decimal                                    pej 2. September 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISEX_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - INC R%lu\n", REG_IDX(1));
            (current_frame->locals[REG_IDX(1)]->int_value)=0-(current_frame->locals[REG_IDX(1)]->int_value);
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  FSEX   op1 = -op1  decimal                                    pej 2. September 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FSEX_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - INC R%lu\n", REG_IDX(1));
            (current_frame->locals[REG_IDX(1)]->float_value)=0-(current_frame->locals[REG_IDX(1)]->float_value);
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  ISUB_REG_REG_INT: Integer Subtract (op1=op2-op3)               pej 8. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISUB_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - ISUB R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI - op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ISUB_REG_INT_REG: Integer Subtract (op1=op2-op3)               pej 8. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISUB_REG_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - ISUB R%d,%d,R%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I - op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  AND_REG_REG_REG  Int Logical AND op1=(op2 && op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(AND_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - AND R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI && op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  OR_REG_REG_REG  Int Logical OR op1=(op2 || op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(OR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - OR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI || op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  NOT_REG_REG  Int Logical NOT op1=!op2
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(NOT_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - NOT R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            if (op2RI) REG_RETURN_INT(0)
            else REG_RETURN_INT(1)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IEQ_REG_REG_REG  Int Equals op1=(op2==op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IEQ_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IEQ R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI == op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IEQ_REG_REG_INT  Int Equals op1=(op2==op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IEQ_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IEQ R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI == op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  INE_REG_REG_REG  Int Equals op1=(op2!=op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - INE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI != op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  INE_REG_REG_INT  Int Equals op1=(op2!=op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INE_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - INE R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI != op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IGT_REG_REG_REG  Int Greater than op1=(op2>op3)                      pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IGT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IGT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI > op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IGT_REG_REG_INT  Int Greater than op1=(op2>op3)                      pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IGT_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IGT R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI > op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IGT_REG_INT_REG  Int Greater than op1=(op2>op3)                      pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IGT_REG_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IGT R%d,%d,r%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I > op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ILT_REG_REG_REG  Int Less than op1=(op2<op3)                         pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ILT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - ILT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI < op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ILT_REG_REG_INT  Int Less than op1=(op2<op3)                         pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ILT_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - ILT R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI < op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ILT_REG_INT_REG  Int Less than op1=(op2<op3)                         pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ILT_REG_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - ILT R%d,%d,r%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I < op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IGTE_REG_REG_REG  Int Greater Equal than op1=(op2>=op3)              pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IGTE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IGTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI >= op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IGTE_REG_REG_INT  Int Greater Equal than op1=(op2>=op3)              pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IGTE_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IGTE R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI >= op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IGTE_REG_INT_REG  Int Greater Equal than op1=(op2>=op3)              pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IGTE_REG_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IGTE R%d,%d,r%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I >= op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ILTE_REG_REG_REG  Int Less Equal than op1=(op2<=op3)                 pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ILTE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - ILTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI <= op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ILTE_REG_REG_INT  Int Less Equal than op1=(op2<=op3)                 pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ILTE_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - ILTE R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI <= op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ILTE_REG_INT_REG  Int Less Equal than op1=(op2<=op3)                 pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ILTE_REG_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - ILTE R%d,%d,r%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I <= op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IGTBR_ID_REG_REG  if op2>op3 ; goto op1                             pej 12 June 2023
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(IGTBR_ID_REG_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - IGTBR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2),(int)REG_IDX(3));
    if (op2RI > op3RI) {
        next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
        CALC_DISPATCH_MANUAL
    }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  ILTBR_ID_REG_REG  if op2<op3 ; goto op1                             pej 14 June 2023
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(ILTBR_ID_REG_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - ILTBR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2),(int)REG_IDX(3));
    if (op2RI < op3RI) {
        next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
        CALC_DISPATCH_MANUAL
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  FEQ_REG_REG_REG  Float Equals op1=(op2==op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FEQ_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FEQ R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF == op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FEQ_REG_REG_FLOAT  Float Equals op1=(op2==op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FEQ_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FEQ R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF == op3F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FNE_REG_REG_REG  Float Not Equals op1=(op2!=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FNE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FNE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF != op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FNE_REG_REG_FLOAT  Float Not Equals op1=(op2!=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FNE_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FNE R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF != op3F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FGT_REG_REG_REG  Float Greater than op1=(op2>op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FGT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FGT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF > op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FGT_REG_REG_FLOAT  Float Greater than op1=(op2>op3)
 *
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FGT_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FGT R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF > op3F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FGT_REG_FLOAT_REG  Float Greater than op1=(op2>op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FGT_REG_FLOAT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FGT R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_INT(op2F > op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FLT_REG_REG_REG  Float Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FLT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FLT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF < op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FLT_REG_REG_FLOAT  Float Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FLT_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FLT R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF < op3F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FLT_REG_FLOAT_REG  Float Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FLT_REG_FLOAT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FLT R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_INT(op2F < op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FGTE_REG_REG_REG  Float Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FGTE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FGTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF >= op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FGTE_REG_REG_FLOAT  Float Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FGTE_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FGTE R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF >= op3F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FGTE_REG_FLOAT_REG  Float Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FGTE_REG_FLOAT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FGTE R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_INT(op2F >= op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FLTE_REG_REG_REG  Float Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FLTE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FLTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF <= op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FLTE_REG_REG_FLOAT  Float Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FLTE_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FLTE R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF <= op3F)
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  FLTE_REG_FLOAT_REG  Float Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FLTE_REG_FLOAT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FLTE R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_INT(op2F <= op3RF)
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  FGTBR_ID_REG_REG  if op2>op3 ; goto op1                            pej 14 June 2023
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FGTBR_ID_REG_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - FGTBR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2),(int)REG_IDX(3));
    if (op2RF > op3RF) {
        next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
        CALC_DISPATCH_MANUAL
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  FLTBR_ID_REG_REG  if op2>op3 ; goto op1                            pej 14 June 2023
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FLTBR_ID_REG_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - FLTBR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2),(int)REG_IDX(3));
    if (op2RF < op3RF) {
        next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
        CALC_DISPATCH_MANUAL
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  SEQ_REG_REG_REG  String Equals op1=(op2==op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SEQ_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SEQ R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(!string_cmp_value(op2R, op3R))
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SEQ_REG_REG_STRING String Equals op1=(op2==op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SEQ_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - SEQ R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);
            REG_RETURN_INT(!string_cmp_const(op2R, op3S))
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  RSEQ_REG_REG_REG  String Equals op1=(op2=op3) non strict REXX comparison  pej 29. Nov 2021
 *  -----------------------------------------------------------------------------------
*/
        START_INSTRUCTION(RSEQ_REG_REG_REG) CALC_DISPATCH(3)
        {
            DEBUG("TRACE - RSEQ R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            int ch;
            int p1, p2;
            int len1, len2;

            GETSTRLEN(len1, op2R)
            GETSTRLEN(len2, op3R)

            // step 1 find last not blank character
            for (p1 = len1 - 1; p1 >= 0; p1--, len1--) {
                GETSTRCHAR(ch, op2R, p1)
                if (ch != ' ') break;
            }
            for (p2 = len2 - 1; p2 >= 0; p2--, len2--) {
                GETSTRCHAR(ch, op3R, p2)
                if (ch != ' ') break;
            }

            // step 2 find first non blank
            for (p1 = 0; p1 < len1; p1++) {
                GETSTRCHAR(ch, op2R, p1)
                if (ch != ' ') break;
            }
            for (p2 = 0; p2 < len2; p2++) {
                GETSTRCHAR(ch, op3R, p2)
                if (ch != ' ') break;
            }
            if (len1 - p1 != len2 - p2) REG_RETURN_INT(0)
            else {
                if (string_cmp(op2R->string_value + p1, len1 - p1,
                           op3R->string_value + p2, len2 - p2) == 0)
                    REG_RETURN_INT(1)
                else REG_RETURN_INT(0)
            }
        }
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  RSEQ_REG_REG_STRING String Equals op1=(op2=op3)  non strict REXX comparison
 *  TODO !!! not yet implemented !!!
 *  -----------------------------------------------------------------------------------
*/
        START_INSTRUCTION(RSEQ_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - RSEQ R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                  REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                  (CONSTSTRING_OP(3))->string);
            REG_RETURN_INT(0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SNE_REG_REG_REG  String Not Equals op1=(op2!=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SNE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SNE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) != 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SNE_REG_REG_STRING  String Not Equals op1=(op2!=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SNE_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - SNE R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);
            REG_RETURN_INT(string_cmp_const(op2R, op3S) != 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SGT_REG_REG_REG  String Greater than op1=(op2>op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SGT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SGT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) > 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SGT_REG_REG_STRING  String Greater than op1=(op2>op3)
 *
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SGT_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - SGT R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);

            REG_RETURN_INT(string_cmp_const(op2R, op3S) > 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SGT_REG_STRING_REG  String Greater than op1=(op2>op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SGT_REG_STRING_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SGT R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                (int) (CONSTSTRING_OP(2))->string_len,
                (CONSTSTRING_OP(2))->string, REG_IDX(3));
            REG_RETURN_INT(string_cmp_const(op3R, op2S) < 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SLT_REG_REG_REG  String Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SLT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SLT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) < 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SLT_REG_REG_STRING  String Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SLT_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - SLT R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);

            REG_RETURN_INT(string_cmp_const(op2R, op3S) < 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SLT_REG_STRING_REG  String Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SLT_REG_STRING_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SLT R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                (int) (CONSTSTRING_OP(2))->string_len,
                (CONSTSTRING_OP(2))->string, REG_IDX(3));
            REG_RETURN_INT(string_cmp_const(op3R, op2S) > 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SGTE_REG_REG_REG  String Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SGTE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SGTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) >= 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SGTE_REG_REG_STRING  String Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SGTE_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - SGTE R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);

            REG_RETURN_INT(string_cmp_const(op2R, op3S) >= 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SGTE_REG_STRING_REG  String Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SGTE_REG_STRING_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SGTE R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                (int) (CONSTSTRING_OP(2))->string_len,
                (CONSTSTRING_OP(2))->string, REG_IDX(3));
            REG_RETURN_INT(string_cmp_const(op3R, op2S) <= 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SLTE_REG_REG_REG  String Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SLTE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SLTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) <= 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SLTE_REG_REG_STRING  String Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SLTE_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - SLTE R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);
            REG_RETURN_INT(string_cmp_const(op2R, op3S) <= 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SLTE_REG_STRING_REG  String Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SLTE_REG_STRING_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SLTE R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                (int) (CONSTSTRING_OP(2))->string_len,
                (CONSTSTRING_OP(2))->string, REG_IDX(3));
            REG_RETURN_INT(string_cmp_const(op3R, op2S) >= 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  COPY_REG_REG  Copy op2 to op1
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(COPY_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - COPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            copy_value(op1R, op2R);
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SCOPY_REG_REG  Copy String op2 to op1
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SCOPY_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SCOPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            copy_string_value(op1R, op2R);
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ICOPY_REG_REG  Copy Integer op2 to op1
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ICOPY_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - ICOPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            op1R->int_value = op2R->int_value;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FCOPY_REG_REG  Copy Float op2 to op1
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FCOPY_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - FCOPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            op1R->float_value = op2R->float_value;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ACOPY_REG_REG Copy status Attributes op2 to op1
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ACOPY_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - ACOPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            op1R->status.all_type_flags = op2R->status.all_type_flags;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  INC_REG  Increment Int (op1++)                                      pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INC_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - INC R%lu\n", REG_IDX(1));
            (current_frame->locals[REG_IDX(1)]->int_value)++;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IDIV_REG_REG_INT  Integer Divide (op1=op2/op3)                      pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IDIV_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IDIV R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI / op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IDIV_REG_INT_REG  Integer Divide (op1=op2/op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IDIV_REG_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IDIV R%d,%d,R%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I / op3RI)
            DISPATCH

/* -----------------------------------------------------------------------------------
 *  IDIV_REG_REG_REG  Integer Divide (op1=op2/op3)                      pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IDIV_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IDIV R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI / op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IMOD_REG_REG_INT  Integer Modulo (op1=op2 & op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IMOD_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IMOD R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI % op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IMOD_REG_INT_REG  Integer Modulo (op1=op2 % op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IMOD_REG_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IMOD R%d,%d,R%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I % op3RI)
            DISPATCH

/* -----------------------------------------------------------------------------------
 *  IMOD_REG_REG_REG  Integer Modulo (op1=op2 % op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IMOD_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IMOD R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI % op3RI)
            DISPATCH
 /* ------------------------------------------------------------------------------------
  *  IOR_REG_REG_REG bitwise OR (op1=op2|op3)                           pej 17 Oct 2021
  *  -----------------------------------------------------------------------------------
  */
        START_INSTRUCTION(IOR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IOR R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI | op3RI)
            DISPATCH
 /* -----------------------------------------------------------------------------------
  *  IOR_REG_REG_INT  bitwise OR (op1=op2|op3)                          pej 17 Oct 2021
  *  ----------------------------------------------------------------------------------
  */
        START_INSTRUCTION(IOR_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IOR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI | op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IAND_REG_REG_INT  bitwise AND (op1=op2&op3)                         pej 17 Oct 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IAND_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IAND R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI & op3I)
            DISPATCH
/* -----------------------------------------------------------------------------------
 *  IAND_REG_REG_REG  bitwise AND (op1=op2&op3)                        pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IAND_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IAND R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI & op3RI)
            DISPATCH

/* -----------------------------------------------------------------------------------
 *  IXOR_REG_REG_REG  bitwise XOR (op1=op2^op3)                        pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IXOR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IXOR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI ^ op3RI)
            DISPATCH

/* -----------------------------------------------------------------------------------
 *  IXOR_REG_REG_INT  bitwise XOR (op1=op2^op3)                        pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IXOR_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IXOR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI ^ op3I)
            DISPATCH

/* -----------------------------------------------------------------------------------
 *  ISHL_REG_REG_REG  bitwise shift logical left (op1=op2<<op3)         pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISHL_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - ISHL R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI << op3RI)
            DISPATCH

/* -----------------------------------------------------------------------------------
 *  ISHL_REG_REG_INT  bitwise shift logical left (op1=op2<<op3)         pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISHL_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - ISHL R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI << op3I)
            DISPATCH
/* -----------------------------------------------------------------------------------
 *  ISHR_REG_REG_REG  bitwise shift logical right (op1=op2>>op3)       pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISHR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - ISHR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI >> op3RI)
            DISPATCH

/* -----------------------------------------------------------------------------------
 *  ISHR_REG_REG_INT  bitwise shift logical right (op1=op2>>op3)       pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISHR_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IXSHL R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI >> op3I)
            DISPATCH
/* -----------------------------------------------------------------------------------
 *  INOT_REG_REG  inverts all bits of an integer (op1=~op2)            pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INOT_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - INOT R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            REG_RETURN_INT(~op2RI)
            DISPATCH

/* -----------------------------------------------------------------------------------
 *  INOT_REG_INT  inverts all bits of an integer (op1=~op2)            pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INOT_REG_INT) CALC_DISPATCH(2)
            DEBUG("TRACE - INOT R%d,R%d\n", (int)REG_IDX(1), (int)op2I);
            REG_RETURN_INT(~op2I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SAY_INT  Say op1                                                    pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SAY_INT) CALC_DISPATCH(1)
            DEBUG("TRACE - SAY %d\n", (int)op1I);
#ifdef __32BIT__
            mprintf("%ld\n", op1I);
#else
            mprintf("%lld\n", op1I);
#endif
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SAY_CHAR  Say op1                                                   pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SAY_CHAR) CALC_DISPATCH(1)
            DEBUG("TRACE - SAY \'%c\'\n", (pc + (1))->cconst);
            mprintf("%c\n", (pc + (1))->cconst);
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SAY_FLOAT  Say op1                                                  pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SAY_FLOAT) CALC_DISPATCH(1)
            DEBUG("TRACE - SAY %.15g\n", op1F);
            mprintf("%.15g\n", op1F);
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  LOAD_REG_FLOAT  Load op1 with op2                                   pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(LOAD_REG_FLOAT) CALC_DISPATCH(2)
            DEBUG("TRACE - LOAD R%d,%.15g\n",(int)REG_IDX(1),op2F);
            REG_RETURN_FLOAT(op2F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FADD_REG_REG_REG  Float Add (op1=op2+op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FADD_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FADD R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2RF + op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FSUB_REG_REG_REG  Float Sub (op1=op2-op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

        START_INSTRUCTION(FSUB_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FSUB R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2RF - op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FDIV_REG_REG_REG  Float Div (op1=op2/op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FDIV_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FDIV R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2RF / op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FMULT_REG_REG_REG  Float Mult (op1=op2/op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FMULT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FMULT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2RF * op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FADD_REG_REG_FLOAT  Float Add (op1=op2+op3)                          pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FADD_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FADD R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_FLOAT(op2RF + op3F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FSUB_REG_REG_FLOAT  Float Sub (op1=op2-op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FSUB_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FSUB R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_FLOAT(op2RF - op3F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FDIV_REG_REG_FLOAT  Float Div (op1=op2/op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FDIV_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FDIV R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_FLOAT(op2RF / op3F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FMULT_REG_REG_FLOAT  Float Mult (op1=op2/op3)                       pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FMULT_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FMULT R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_FLOAT(op2RF * op3F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FSUB_REG_FLOAT_REG  Float Sub (op1=op2-op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FSUB_REG_FLOAT_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - FSUB R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2F - op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FDIV_REG_FLOAT_REG  Float Div (op1=op2/op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FDIV_REG_FLOAT_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - FDIV R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2F / op3RF)
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  FPOW_REG_REG_FLOAT  op1=op2**op2w Float operationn                   pej 3 March 2022
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FPOW_REG_REG_FLOAT) CALC_DISPATCH(3)
    DEBUG("TRACE - DIVF R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        REG_RETURN_FLOAT(pow(op2RF,op3F))
    }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  FPOW_REG_REG_REG  op1=op2**op2 Float operation                     pej 3 March 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FPOW_REG_REG_REG) CALC_DISPATCH(3)
    {
        DEBUG("TRACE - FPOW R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2),
              (int) REG_IDX(3));
        REG_RETURN_FLOAT(pow(op2RF,op3RF))
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  IPOW_REG_REG_REG  op1=op2**op2w Integer operation
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(IPOW_REG_REG_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - IPOW R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));

        op1R->int_value = ipow(op2R->int_value, op3R->int_value);
        if (!op1R->int_value) SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  IPOW_REG_REG_INT  op1=op2**op2w Integer operationn
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(IPOW_REG_REG_INT) CALC_DISPATCH(3)
        DEBUG("TRACE - IPOW R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);

        op1R->int_value = ipow(op2R->int_value, op3I);
        if (!op1R->int_value) SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  IPOW_REG_INT_REG  op1=op2**op2w Integer operationn
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(IPOW_REG_INT_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - IPOW R%d,%d,R%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));

        op1R->int_value = ipow(op2I, op3R->int_value);
        if (!op1R->int_value) SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  LINKARG_REG_REG_INT  Link args[op2+op3] to op1
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(LINKARG_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - LINKARG R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            op1R = current_frame->locals[op2RI + op3I +
                                         current_frame->procedure->binarySpace->globals +
                                         current_frame->procedure->locals];
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  LINKARG_REG_INT  Link args[op2] to op1
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(LINKARG_REG_INT) CALC_DISPATCH(2)
            DEBUG("TRACE - LINKARG R%d,%d\n", (int)REG_IDX(1), (int)op2I);
            op1R = current_frame->locals[op2I +
                                         current_frame->procedure->binarySpace->globals +
                                         current_frame->procedure->locals];
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ITOS_REG  Set register string value from its int value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(ITOS_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - ITOS R%lu\n", REG_IDX(1));
            string_from_int(op1R);
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FTOS_REG  Set register string value from its float value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(FTOS_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - FTOS R%lu\n", REG_IDX(1));
            string_from_float(op1R);
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  ITOF_REG  Set register float value from its int value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(ITOF_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - ITOF R%lu\n", REG_IDX(1));
            op1R->float_value = op1R->int_value;
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  FTOI_REG  Set register int value from its float value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(FTOI_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - FTOI R%lu\n", REG_IDX(1));
            int_from_float(op1R);
            if (op1R->float_value != (double)op1R->int_value) {
                SET_SIGNAL(RXSIGNAL_CONVERSION_ERROR);
            }
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  FTOB_REG  Set register boolean (int set to 1 or 0) value from its float value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(FTOB_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - FTOB R%lu\n", REG_IDX(1));
            int_from_float(op1R);

            if (op1R->float_value) op1R->int_value = 1;
            else op1R->int_value = 0;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  BTOI_REG  Set register integer value from its boolean value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(BTOI_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - BTOI R%lu\n", REG_IDX(1));
            if (op1R->int_value) op1R->int_value = 1;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  BTOF_REG  Set register float value from its boolean value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(BTOF_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - BTOF R%lu\n", REG_IDX(1));
            if (op1R->int_value) op1R->float_value = 1.0;
            else op1R->float_value = 0.0;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  BTOS_REG  Set register string value from its boolean value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(BTOS_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - BTOS R%lu\n", REG_IDX(1));
            if (op1R->int_value) set_null_string(op1R,"1");
            else set_null_string(op1R,"0");
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  STOI_REG  Set register int value from its string value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(STOI_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - STOI R%lu\n", REG_IDX(1));
            /* Convert a string to a integer - returns 1 on error */
            if (string2integer(&op1R->int_value, op1R->string_value, op1R->string_length)) {
                SET_SIGNAL(RXSIGNAL_CONVERSION_ERROR);
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  STOF_REG  Set register float value from its string value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(STOF_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - STOF R%lu\n", REG_IDX(1));
            /* Convert a string to a float - returns 1 on error */
            if (string2float(&op1R->float_value, op1R->string_value, op1R->string_length)) {
                SET_SIGNAL(RXSIGNAL_CONVERSION_ERROR);
            }
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  FFORMAT_REG_REG_REG  Set string from float use format string   pej 3. November 2021
 *  DEPRECATED - use FEXTR_REG_REG_REG
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FFORMAT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FFORMAT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            prep_string_buffer(op1R,SMALLEST_STRING_BUFFER_LENGTH); // Large enough for a float
            op3R->string_value[op3R->string_length]='\0';    // terminate format string explicitly, rexx vars aren't!
            op1R->string_length = snprintf(op1R->string_value,SMALLEST_STRING_BUFFER_LENGTH,op3R->string_value,op2R->float_value);
            op1R->string_pos = 0;
  #ifndef NUTF8
            op1R->string_char_pos = 0;
            op1R->string_chars = op1R->string_length;
  #endif
            DISPATCH
/* ------------------------------------------------------------------------------------
 * FEXTR_REG_REG_REG Extract float to string coefficient and decimal exponent integer
 * R3 contains the float value
 * R1 will store the coefficient as a string (or nan, inf, -inf)
 * R2 will store the decimal exponent as an integer
 * Output normalised, rounded to DBL_DIG (typically 15) decimal places (i.e. DBL_DIG-1 fractional digits), and
 * trimmed of trailing zeros
 * This instruction is designed to allow the user to format the float as they wish
 */
        START_INSTRUCTION(FEXTR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FEXTR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            extract_double_decimal(op1R,op2R,op3R->float_value);
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  STRLOWER_REG_REG  translate string into lower case string              pej 23.10.21
 *  -----------------------------------------------------------------------------------
 */
// TODO: what to do if there is a length change of chars during translation
            START_INSTRUCTION(STRLOWER_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - STRLOWER R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                set_value_string(op1R, op2R);
                null_terminate_string_buffer(op1R); /* the logic requires a null terminator */
#ifdef NUTF8
                char *c;
                for (c = op1R->string_value; *c; ++c) *c = (char)tolower(*c);
#else
                utf8lwr(op1R->string_value);
#endif
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  STRUPPER_REG_REG  translate string into upper case string              pej 23.10.21
 *  -----------------------------------------------------------------------------------
 */
// TODO: what to do if there is a length change of chars during translation
            START_INSTRUCTION(STRUPPER_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - STRUPPER R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                set_value_string(op1R, op2R);
                null_terminate_string_buffer(op1R); /* the logic requires a null terminator */
#ifdef NUTF8
                char *c;
                for (c = op1R->string_value ; *c; ++c) *c = (char)toupper(*c);
#else
                utf8upr(op1R->string_value);
#endif
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  STRCHAR_REG_REG_REG  String to Int op1 = op2[op3]                   pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(STRCHAR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - STRCHAR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
#ifndef NUTF8
                int result;
                string_set_byte_pos(op2R, op3R->int_value);
                utf8codepoint(op2R->string_value + op2R->string_pos, &result);
                REG_RETURN_INT(result)
#else
                REG_RETURN_INT(op2R->string_value[op3R->int_value])
#endif
            }
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  HEXCHAR_REG_REG_REG  op1 = hex(op2[op3])                       pej 04 November 2021
 *  -----------------------------------------------------------------------------------
 */
            START_INSTRUCTION(HEXCHAR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - HEXCHAR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
                static const char hexconst[] = {'0','1','2','3','4','5','6','7','8','9','a', 'b', 'c', 'd', 'e', 'f','A', 'B', 'C', 'D', 'E', 'f'};
                int ch;
#ifndef NUTF8
                string_set_byte_pos(op2R, op3R->int_value);
                utf8codepoint(op2R->string_value + op2R->string_pos, &ch);
#else
                ch=op2R->string_value[op3R->int_value];
#endif
                rxinteger lhs = (ch >> 4) & 15;   // extract left hand side of value
                rxinteger rhs = (ch) & 15; // extract right hand side of value
                op1R->string_value[0] = hexconst[lhs];    // set first character of hex value
                op1R->string_value[1] = hexconst[rhs];    // set first character of hex value
                op1R->string_value[2] = '\0';            // set end of string, just to be safe
                PUTSTRLEN(op1R, 2)                  // hex length is 2
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  POSCHAR_REG_REG_REG  op1 position of op3 in op2                pej 05 November 2021
 *  -----------------------------------------------------------------------------------
 */
            START_INSTRUCTION(POSCHAR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - POSCHAR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
                rxinteger result = -1, i;
                int ch;

                for (i = 0; i < op2R->string_length; i++) {
#ifndef NUTF8
                    string_set_byte_pos(op2R, i);
                    utf8codepoint(op2R->string_value + op2R->string_pos, &ch);
#else
                    ch=op2R->string_value[i];
#endif
                    if (ch == op3R->int_value) {
                        result = i;
                        break;
                    }
                }
                REG_RETURN_INT(result)
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  BGT_ID_REG_REG  if op2>op3 goto op1                           pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BGT_ID_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - BGT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            if (current_frame->locals[REG_IDX(2)]->int_value > current_frame->locals[REG_IDX(3)]->int_value) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  BGT_ID_REG_INT  if op2>op3 goto op1                           pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BGT_ID_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - BGT 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            if (current_frame->locals[REG_IDX(2)]->int_value > op3I) {
               next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
               CALC_DISPATCH_MANUAL
            }
        DISPATCH
/* ------------------------------------------------------------------------------------
 *  BGE_ID_REG_REG  if op2>=op3 goto op1                          pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BGE_ID_REG_REG) CALC_DISPATCH(3)
       DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
       if (current_frame->locals[REG_IDX(2)]->int_value >= current_frame->locals[REG_IDX(3)]->int_value) {
          next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
          CALC_DISPATCH_MANUAL
       }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  BGE_ID_REG_INT  if op2>=op3 goto op1                         pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BGE_ID_REG_INT) CALC_DISPATCH(3)
        DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value >= op3I) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  BLT_ID_REG_REG  if op2<op3 goto op1                           pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BLT_ID_REG_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - BLT 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value < current_frame->locals[REG_IDX(3)]->int_value) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  BLT_ID_REG_INT  if op2<op3 goto op1                           pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BLT_ID_REG_INT) CALC_DISPATCH(3)
        DEBUG("TRACE - BGT 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value < op3I) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  BLE_ID_REG_REG  if op2<=op3 goto op1                          pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BLE_ID_REG_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value <= current_frame->locals[REG_IDX(3)]->int_value) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  BLE_ID_REG_INT  if op2<=op3 goto op1                          pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BLE_ID_REG_INT) CALC_DISPATCH(3)
        DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value <= op3I) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  BNE_ID_REG_INT  if op2!=op3 goto op1                          pej 14 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BNE_ID_REG_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value != current_frame->locals[REG_IDX(3)]->int_value) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  BNE_ID_REG_INT  if op2!=op3 goto op1                          pej 14 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BNE_ID_REG_INT) CALC_DISPATCH(3)
        DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value != op3I) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  BEQ_ID_REG_INT  if op2=op3 goto op1                           pej 14 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BEQ_ID_REG_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value == current_frame->locals[REG_IDX(3)]->int_value) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  BEQ_ID_REG_INT  if op2=op3 goto op1                           pej 14 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BEQ_ID_REG_INT) CALC_DISPATCH(3)
        DEBUG("TRACE - BGE 0x%x,R%d,%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
        if (current_frame->locals[REG_IDX(2)]->int_value == op3I) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        }
    DISPATCH
 /* ------------------------------------------------------------------------------------
 *  BCT_REG_ID  dec op2; if op2>0 goto op1                           pej 26 August 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCT_ID_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - BCT R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            (current_frame->locals[REG_IDX(2)]->int_value)--;
            if (current_frame->locals[REG_IDX(2)]->int_value > 0) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
        DISPATCH
/* ------------------------------------------------------------------------------------
 *  BCT_REG_REG_ID  dec op2, inc op3; if op2>0 goto op1              pej 26 August 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCT_ID_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - BCT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            (current_frame->locals[REG_IDX(2)]->int_value)--;
            (current_frame->locals[REG_IDX(3)]->int_value)++;
            if (current_frame->locals[REG_IDX(2)]->int_value>0) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
        DISPATCH
/* ------------------------------------------------------------------------------------
 *  BCF_ID_REG if op2=0 goto op1(if false) else dec op2
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCF_ID_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - BCF R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            if (current_frame->locals[REG_IDX(2)]->int_value == 0) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
            else (current_frame->locals[REG_IDX(2)]->int_value)--;
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  BCF_ID_REG_REG  if op2=0 goto op1(if false) else dec op2 and inc op3
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCF_ID_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - BCF R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            if (current_frame->locals[REG_IDX(2)]->int_value == 0) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
            else {
                (current_frame->locals[REG_IDX(2)]->int_value)--;
                (current_frame->locals[REG_IDX(3)]->int_value)++;
            }
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  BCTNM_REG_ID  dec op2; if op2>=0 goto op1                           pej 26 August 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCTNM_ID_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - BCTNM R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            (current_frame->locals[REG_IDX(2)]->int_value)--;
            if (current_frame->locals[REG_IDX(2)]->int_value>=0) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
        DISPATCH
/* ------------------------------------------------------------------------------------
 *  BCTNM_REG_REG_ID  dec op2, inc op3; if op2>=0 goto op1           pej 26 August 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCTNM_ID_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - BCTNM R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            (current_frame->locals[REG_IDX(2)]->int_value)--;
            (current_frame->locals[REG_IDX(3)]->int_value)++;
            if (current_frame->locals[REG_IDX(2)]->int_value>=0) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
        DISPATCH
/* ------------------------------------------------------------------------------------
 *  BCTP_ID_REG  inc op2; goto op1                                     pej 11 June 2023
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCTP_ID_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - BCTP R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            (current_frame->locals[REG_IDX(2)]->int_value)++;
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  FndBlnk REG_REG_REG  return first blank after op2[op3]          pej 27 August 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FNDBLNK_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FNDBLNK R%lu R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                rxinteger len;
                rxinteger result;
                int ch;
#ifndef NUTF8
                len = (rxinteger) op2R->string_chars;
#else
                len = (rxinteger) op2R->string_length;
#endif
                for (result = op3R->int_value; result < len; result++) {
#ifndef NUTF8
                    string_set_byte_pos(op2R, result);
                    utf8codepoint(op2R->string_value + op2R->string_pos, &ch);
#else
                    ch = op2R->string_value[result];
#endif
                    if (ch == ' ') goto blankfound;
                }
                result = -len;
            blankfound:
                REG_RETURN_INT(result)
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FndNBlnk REG_REG_REG  return first blank after op2[op3]          pej 27 August 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FNDNBLNK_REG_REG_REG) CALC_DISPATCH(3)
           DEBUG("TRACE - FNDNBLNK R%lu R%lu\n", REG_IDX(1),
              REG_IDX(2));
    {
        rxinteger result;
        rxinteger len;
        int ch;
#ifndef NUTF8
        len = (rxinteger)op2R->string_chars;
#else
        len = (rxinteger)op2R->string_length;
#endif
        for (result = op3R->int_value; result < len; result++) {
#ifndef NUTF8
            string_set_byte_pos(op2R, result);
            utf8codepoint(op2R->string_value + op2R->string_pos, &ch);
#else
            ch = op2R->string_value[result];
#endif
            if (ch != ' ') goto nonblankfound;
        }
        result = -len;

    nonblankfound:
        REG_RETURN_INT(result)
    }
    DISPATCH
 /* ------------------------------------------------------------------------------------
  *  GETBYTE_REG_REG_REG  Int op1 = op2[op3]                             pej 19 Oct 2021
  *  -----------------------------------------------------------------------------------
  */
    START_INSTRUCTION(GETBYTE_REG_REG_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - GETBYTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));

    /* TODO */
    REG_RETURN_INT(0)
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  CONCCHAR_REG_REG_REG  op1=op2[op3]                                pej 27 August 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(CONCCHAR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - CONCCHAR R%lu R%lu R%lu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                rxinteger temp = op3R->int_value;   // save offset, we misuse v3 later
#ifndef NUTF8
                int ch;
                string_set_byte_pos(op2R, op3R->int_value);
                utf8codepoint(op2R->string_value + op2R->string_pos, &ch);
                op3R->int_value = ch;
#else
                op3R->int_value=op2R->string_value[op3R->int_value - 1];
#endif
                string_concat_char(op1R, op3R);
                op3R->int_value = temp;   // restore original v3
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  TRANSCHAR_REG_REG_REG  replace op1 if it is in op3-list by char in op2-list pej 7 November 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(TRANSCHAR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - TRANSCHAR R%lu R%lu R%lu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                rxinteger val = op1R->int_value;
                rxinteger len, i;
                int ch;

                GETSTRLEN(len, op3R)

                for (i = 0; i < len; i++) {
                    GETSTRCHAR(ch, op3R, i)
                    if (val == ch) {
                        GETSTRCHAR(ch, op2R, i)
                        val = ch;
                        break;
                    }
                }
                REG_RETURN_INT(val)
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  DROPCHAR_REG_REG_REG  removes characters contained in op3-list pej 19 November 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(DROPCHAR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - DROPCHAR R%lu R%lu R%lu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                rxinteger i, len1, len2;
                int found;
                int ch;
                GETSTRLEN(len1, op2R)
                GETSTRLEN(len2, op3R)
                if (len2 == 0) len2 = (rxinteger) op3R->string_length;
                for (i = 0; i < len1; i++) {
                    rxinteger j;
                    GETSTRCHAR(ch, op2R, i)
                    op2R->int_value = ch;
                    found = 0;
                    for (j = 0; j < len2; j++) {
                        GETSTRCHAR(ch, op3R, j)
                        op3R->int_value = ch;
                        if (op2R->int_value == op3R->int_value) {
                            found = 1;  // found drop char
                            break;
                        }
                    }
                    if (found == 1) continue;
                    string_concat_char(op1R, op2R);
                }
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SUBSTRING_REG_REG_REG op1=substr(op2,op3) substring from  offset op3  pej 12 November 2021
 *
 *  !!! the position parameter is offset +1, this is an exception from normally     !!!
 *  !!! using the offset. Reason: this instruction will be used directly from rexx  !!!
 *  !!  so we save one instruction                                                  !!
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SUBSTRING_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SUBSTRING R%lu R%lu R%lu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                rxinteger offset = op3R->int_value - 1;   /* make position to offset  */
                rxinteger len, i;
                int ch;
                PUTSTRLEN(op1R, 0)      /* reset length of target  */
                GETSTRLEN(len, op3R)
                for (i = offset; i < len; i++) {
                    GETSTRCHAR(ch, op2R, i)
                    op2R->int_value = ch;
                    string_concat_char(op1R, op2R);
                }
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
*  SUBSTCUT_REG_REG_REG op1=substr(op1,,op2) cuts off after op2   pej 13 November 2021
*  -----------------------------------------------------------------------------------
*/
        START_INSTRUCTION(SUBSTCUT_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SUBSTCUT R%lu R%lu\n", REG_IDX(1), REG_IDX(2));

            PUTSTRLEN(op1R,op2R->int_value)

        DISPATCH

/* ------------------------------------------------------------------------------------
 *  PADSTR_REG_REG_REG op1=op2(repeated op3 times)                 pej 13 November 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(PADSTR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - PADSTR R%lu R%lu R%lu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                int pad;
                int i;
                PUTSTRLEN(op1R, 0)       /* reset length of target  */
                GETSTRCHAR(pad, op2R, 0)      /* fetch pad character   */
                op2R->int_value = pad;
                for (i = 0; i < op3R->int_value; i++) {
                    string_concat_char(op1R, op2R);
                }
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  CNOP Dummy instruction for testing purposes                     pej 11 November 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(CNOP) CALC_DISPATCH(0)
        DEBUG("TRACE - CNOP\n");
        DISPATCH

/*
 *   APPENDCHAR_REG_REG Append Concat Char op2 (as int) on op1
 */
        START_INSTRUCTION(APPENDCHAR_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - APPENDCHAR R%lu R%lu\n", REG_IDX(1),
                  REG_IDX(2));
            string_concat_char(op1R, op2R);
            DISPATCH

/*
 *   APPEND_REG_REG Append string op2 on op1
 */
        START_INSTRUCTION(APPEND_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - APPEND R%lu R%lu\n", REG_IDX(1),
                  REG_IDX(2));
            string_append(op1R, op2R);
            DISPATCH

/*
 *   SAPPEND_REG_REG Append with space string op2 on op1
 */
        START_INSTRUCTION(SAPPEND_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SAPPEND R%lu R%lu\n", REG_IDX(1),
                  REG_IDX(2));
            string_sappend(op1R, op2R);
            DISPATCH

/*
 *   STRLEN_REG_REG String Length op1 = length(op2)
 */
        START_INSTRUCTION(STRLEN_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - STRLEN R%lu R%lu\n", REG_IDX(1),
                  REG_IDX(2));
#ifndef NUTF8
            op1R->int_value = (rxinteger)op2R->string_chars;
#else
            op1R->int_value = (rxinteger)op2R->string_length;
#endif
            DISPATCH

/*
 * SETSTRPOS_REG_REG - Set String (op1) charpos set to op2
 */
        START_INSTRUCTION(SETSTRPOS_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SETSTRPOS R%lu R%lu\n", REG_IDX(1),
                  REG_IDX(2));
#ifndef NUTF8
            string_set_byte_pos(op1R, op2R->int_value);
#else
            op1R->string_pos = op2R->int_value;
#endif
            DISPATCH

/*
 * GETSTRPOS_REG_REG - Get String (op2) charpos into op1
 */
        START_INSTRUCTION(GETSTRPOS_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - GETSTRPOS R%lu R%lu\n", REG_IDX(1),
                  REG_IDX(2));
#ifndef NUTF8
            op1R->int_value = (int) op2R->string_char_pos;
#else
            op1R->int_value = op2R->string_pos;
#endif
            DISPATCH

/*
 * STRCHAR_REG_REG - op1 (as int) = op2[charpos]
 */
        START_INSTRUCTION(STRCHAR_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - STRCHAR R%lu R%lu\n", REG_IDX(1),
                  REG_IDX(2));
            {
                int ch;
#ifndef NUTF8
                utf8codepoint(op2R->string_value + op2R->string_pos, &ch);
                op1R->int_value = ch;
#else
                op1R->int_value = op2R->string_value[op2R->string_pos];
#endif
            }
            DISPATCH

/*
 * GETTP_REG_REG gets the register type flag (op1 = op2.typeflag)
 */
        START_INSTRUCTION(GETTP_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - GETTP R%d R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            op1R->int_value = op2R->status.all_type_flags;
            DISPATCH

/*
 * SETTP_REG_INT sets the register type flag (op1.typeflag = op2)
 */
        START_INSTRUCTION(SETTP_REG_INT) CALC_DISPATCH(2)
            DEBUG("TRACE - SETTP R%d %d\n", (int)REG_IDX(1), (int)op2I);
            op1R->status.all_type_flags = op2I;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  LOADSETTP_REG_INT load register & set the register type flag pej 11 November 2021
 *   op1=op2 and (op1.typeflag = op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(LOADSETTP_REG_INT_INT) CALC_DISPATCH(3)
        DEBUG("TRACE - LOADSETTP R%d %d %d\n", (int)REG_IDX(1),(int)op2I,(int)op3I);

            op1R->int_value = op2I;
            op1R->status.all_type_flags = op3I;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  LOADSETTP_REG_string load string to register & set the register type flag pej 11 November 2021
 *   op1=op2 and (op1.typeflag = op3)
 *  -----------------------------------------------------------------------------------
 */
            START_INSTRUCTION(LOADSETTP_REG_STRING_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - LOADSETTP R%d %s %d\n", (int)REG_IDX(1),op2S->string,(int) op3I);

            set_const_string(op1R, op2S);
            op1R->status.all_type_flags = op3I;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  LOADSETTP_REG_FLOAT float to load register & set the register type flag pej 11 November 2021
 *   op1=op2 and (op1.typeflag = op3)
 *  -----------------------------------------------------------------------------------
 */
            START_INSTRUCTION(LOADSETTP_REG_FLOAT_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - LOADSETTP R%d %.15g %d\n", (int)REG_IDX(1), op2F,(int) op3I);
            op1R->float_value = op2F;
            op1R->status.all_type_flags = op3I;
            DISPATCH

/*
 * SETORTP_REG_INT or the register type flag (op1.typeflag = op1.typeflag || op2)
 */
        START_INSTRUCTION(SETORTP_REG_INT) CALC_DISPATCH(2)
            DEBUG("TRACE - SETORTP R%d %d\n", (int)REG_IDX(1), (int)op2I);
            op1R->status.all_type_flags = op1R->status.all_type_flags | op2I;
            DISPATCH

/*
 * GETANDTP_REG_REG_INT get the register type flag with mask (op1(int) = op2.typeflag & op3)
 */
        START_INSTRUCTION(GETANDTP_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - GETANDTP R%d R%d %d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            op1R->int_value = op2R->status.all_type_flags & op3I;
            DISPATCH

/*
 * BRTPT_ID_REG if op2.typeflag true then goto op1
 */
        START_INSTRUCTION(BRTPT_ID_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - BRTPT_ID_REG 0x%x R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2));
            if (op2R->status.all_type_flags) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
            DISPATCH

/*
 * BRTPANDT_ID_REG_INT if op2.typeflag && op3 true then goto op1
*/
        START_INSTRUCTION(BRTPANDT_ID_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - BRTPANDT_ID_REG_INT 0x%x R%d %d\n",
                  (unsigned int)REG_IDX(1),
                  (int)REG_IDX(2),(int)op3I);
            if (op2R->status.all_type_flags & op3I) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  IRAND_REG_REG Random Number with seed register                 pej 27 February 2022
 *   op1=irand(op2)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IRAND_REG_REG) CALC_DISPATCH(2)
             DEBUG("TRACE - IRAND R%d R%d \n", (int)REG_IDX(1), (int)REG_IDX(2));

            if (op2R->int_value<0) {                 // no seed set
                if (!hasSeed)  {       // seed still initial, set time based seed
                   initSeed = (time((time_t *) 0) % (3600 * 24)); // initial seed still active
                   srand((unsigned) initSeed);
                   hasSeed = 1;
                }
            } else {                                 // seed set re-init with new seed
                initSeed=op2R->int_value;
                srand((unsigned) initSeed);
                hasSeed = 1;
            }
            set_int(op1R, rand());   // receive new random value
        DISPATCH
/* ------------------------------------------------------------------------------------
 *  IRAND_REG_REG Random Number with seed register                 pej 27 February 2022
 *   op1=irand(op2)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IRAND_REG_INT) CALC_DISPATCH(2)
             DEBUG("TRACE - IRAND R%d R%d \n", (int)REG_IDX(1), (int)op2I);
             if (op2I<0) {                                // no seed set
                if (!hasSeed)  {            // seed still initial, set time based seed
                    initSeed = (time((time_t *) 0) % (3600 * 24)); // initial seed still active
                    srand(initSeed);
                    hasSeed = 1;
                }
             } else {                                     // seed set and NE old seed, set it new
                initSeed=op2I;
                srand( initSeed);
                hasSeed = 1;
            }
            set_int(op1R, rand());   // receive new random value
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  rxvers  returns os information                                    pej 20. June 2023
 *  -----------------------------------------------------------------------------------
 */

// MACRO

#define FDATE (char const[]){ __DATE__[7], __DATE__[8], ..., ' ', ... , '\0' }

    START_INSTRUCTION(RXVERS_REG) CALC_DISPATCH(1)
    DEBUG("TRACE - RXVERS R%d\n", (int) REG_IDX(1));
    {
        char vers[64];

#if defined(__linux__)
        strcpy(vers, "linux ");
#elif defined(_WIN32)
        strcpy(vers, "windows ");
#elif defined(__APPLE__)
        strcpy(vers, "macosx ");
#elif defined(__CMS__)
        strcpy(vers, "cms ");
#else
        strcpy(vers, "unknown ");
#endif

#ifdef __32BIT__
        strcat(vers, "32 ");
#else
        strcat(vers, "64 ");
#endif

        strcat(vers, rxversion);
        strcat(vers, " ");
        strcat(vers, compile_date); /* compile_date defined earlier in this file */

        set_null_string(op1R, vers);
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  rxhash  returns hash of a string                                  pej 24. June 2023
 *  op1=hash(op2,op3)
 *      op2=string
 *      op3=length(string)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(RXHASH_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - RXHASH R%d R%d R%d \n", (int)REG_IDX(1),(int)REG_IDX(1),(int)REG_IDX(3));

    {
#ifdef __32BIT__
        uint32_t hash, FNVOffset, FNV_PRIME;
        FNV_PRIME=16777619;
        FNVOffset=2166136261U;
#else
        uint64_t hash, FNVOffset, FNV_PRIME;
        FNV_PRIME = 1099511628211;
        FNVOffset = 14695981039346656037U;
#endif
        int i1, ch, len;
        char str[128];

        hash = FNVOffset;
        GETSTRLEN(len, op2R)
        for (i1 = 0; i1 < len; i1++) {
            GETSTRCHAR(ch, op2R, i1)
            hash = hash ^ (ch);          // xor next byte into the bottom of the hash
            hash = hash * FNV_PRIME;     // Multiply by prime number found to work well
        }
#ifdef __32BIT__
        sprintf(str, "%lu", hash);
#else
        sprintf(str, "%llu", hash);
#endif
        set_null_string(op1R, str);
     }

            DISPATCH
/* ------------------------------------------------------------------------------------------
 *  OPENDLL_REG_REG Open DLL                                            pej 24. February 2022
 *  -----------------------------------------------------------------------------------------
 */

//    typedef char * (*strSubproc)(int arg1, int arg2, char str1[32]);   // for string
    typedef int    (*intSubproc)(int arg1, int arg2, char str1[32]);

START_INSTRUCTION(OPENDLL_REG_REG_REG) CALC_DISPATCH(3)
     DEBUG("TRACE - OPENDLL R%d R%d R%d \n", (int)REG_IDX(1),(int)REG_IDX(1),(int)REG_IDX(3));
#ifdef _WIN32
     HINSTANCE hDLL;               // Handle to DLL
//     strSubproc strProc;
     intSubproc intProc;
     HRESULT hrReturnVal;
     rxinteger i1=-16;
    // rxfuncadd(rexxname,module,sysname)
    op2R->string_value[op2R->string_length]=0;
    op3R->string_value[op3R->string_length]=0;
    printf("Module %s\n",op3R->string_value);

    hDLL = LoadLibrary(op2R->string_value);
    printf("DLL ADDR %d %s %d %d\n",hDLL, op2R->string_value,op2R->string_length,hDLL);

    if (hDLL==0 ) i1=-8;
    else {
         intProc = (intSubproc) GetProcAddress(hDLL, op3R->string_value);
        printf("Module ADDR %d\n",intProc);
        if (intProc==0 ) i1=-12 ;
        else i1 = (rxinteger)intProc;
      }
    FreeLibrary(hDLL);
    REG_RETURN_INT(i1);
#endif
#ifdef __APPLE__
    void *dl_handle;
    int (*func) (float);
    char *error;
    rxinteger i1=-16;
    // rxfuncadd(rexxname,module,sysname)
    op2R->string_value[op2R->string_length]=0;
    op3R->string_value[op3R->string_length]=0;
    printf("Module %s\n",op3R->string_value);
    
    /* Open the shared object */
    dl_handle = dlopen( op2R->string_value, RTLD_LAZY );
    if (dl_handle) i1=-8;
    else {
      func = dlsym( dl_handle, op3R->string_value );
      if (func==0 ) i1=-12 ;
      else i1= (rxinteger)func;
      }
    error = dlerror();
    if (error != NULL) {
      printf( "!!! %s\n", error );
      return (int)i1;
    }
    /* Close the object */
    dlclose( dl_handle );
    REG_RETURN_INT(i1)
#endif
    DISPATCH

    START_INSTRUCTION(DLLPARMS_REG_REG_REG) CALC_DISPATCH(3)

        DEBUG("TRACE - DLLPARMS R%d R%d R%d \n", (int)REG_IDX(1),(int)REG_IDX(2),(int)REG_IDX(3));
       /* Arguments - complex lets never have to change this code! */
        printf("Register containing number of Arguments %d\n",(int) REG_IDX(3));
        printf("                    number of Arguments %d\n",(int)op3RI);
   //     current_frame->locals[(pc + (3))->index];

        size_t j =
                current_frame->procedure->binarySpace->globals +
                current_frame->procedure->locals + 1; /* Callee register index */
        size_t k = (pc + 3)->index + 1; /* Caller register index */
        size_t i;
        printf("                    first data register %d\n",(int)k);
        for ( i = 0;
              i < op3RI;
              i++, j++,k++) {
              printf("                         Data register %d\n",(int)k);
         //   printf("              Register contentLocal Variables %d\n",current_frame->procedure->locals->???);
        }
    REG_RETURN_INT(i)
    DISPATCH

    /* Spawn - Spawn a process with io redirects - Spawn Process op1 = exec op2 redirect op3
     * reg 1 will be the return code of the process
     * reg 2 is the command (the path environment variable is used for search resolution)
     * reg 3 is an array of 3 opaque REDIRECT binary structures (e.g. generated by redrtoarr)
     *       reg3[1] = input, reg3[2] = output, reg3[3] = error
     * spawn generates a failure signal if the command is not found */
    START_INSTRUCTION(SPAWN_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SPAWN R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            {
                int command_rc = 0;
                int spawn_rc = 0;
                REDIRECT *pIn = 0;
                REDIRECT *pOut = 0;
                REDIRECT *pErr = 0;
                char *command;
                command = malloc(op2R->string_length + 1);
                memcpy(command,op2R->string_value, op2R->string_length);
                command[op2R->string_length] = 0;
                char* errorText = 0;

                if (op3R->num_attributes > 0) pIn = (REDIRECT*)(op3R->attributes[0])->binary_value;
                if (op3R->num_attributes > 1) pOut = (REDIRECT*)(op3R->attributes[1])->binary_value;
                if (op3R->num_attributes > 2) pErr = (REDIRECT*)(op3R->attributes[2])->binary_value;

                /* op3R->attributes[2] is the environment variables */
                spawn_rc = shellspawn(command, pIn, pOut, pErr, op3R->attributes[3], &command_rc, &errorText);
                if (spawn_rc == SHELLSPAWN_NOFOUND) {
                    SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Command Not Found");
                    DISPATCH
                }
                if (spawn_rc) {
                    SET_SIGNAL_MSG(RXSIGNAL_FAILURE, errorText);
                    DISPATCH
                }
                if (errorText) free(errorText);
                free(command);
                op1R->int_value = command_rc;
            }
            DISPATCH

    /* redir2str - Redirect op1 = to-string op2
     * reg 1 will be the redirect object
     * reg 2 is string that will have the redirected string appended to
     *       the redirect object MUST then be used in shellspawn() to cleanup/free memory
     */
    START_INSTRUCTION(REDIR2STR_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - REDIR2STR R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        redr2str(op1R, op2R);
        DISPATCH

    /* redir2arr - Redirect op1 = to-array op2
     * reg 1 will be the redirect object
     * reg 2 is array that will have the redirected output appended to
     *       the redirect object MUST then be used in shellspawn() to cleanup/free memory
     */
    START_INSTRUCTION(REDIR2ARR_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - REDIR2ARR R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        redr2arr(op1R, op2R);
        DISPATCH

    /* str2redir - Redirect op1 <- string op2
     * reg 1 will be the redirect object
     * reg 2 is string that will be redirected to the pipe
     *       the redirect object MUST then be used in shellspawn() to cleanup/free memory
     */
    START_INSTRUCTION(STR2REDIR_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - STR2REDIR R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        str2redr(op1R, op2R);
        DISPATCH

    /* arr2redir - Redirect op1 <- array op2
     * reg 1 will be the redirect object
     * reg 2 is array that will be redirected to the pipe
     *       the redirect object MUST then be used in shellspawn() to cleanup/free memory
     */
    START_INSTRUCTION(ARR2REDIR_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - ARR2REDIR R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        arr2redr(op1R, op2R);
        DISPATCH

    /* nullredir - Redirect op1 = to/from null
     * reg 1 will be the redirect object
    */
    START_INSTRUCTION(NULLREDIR_REG) CALC_DISPATCH(1)
        DEBUG("TRACE - NULLREDIR R%lu\n", REG_IDX(1));
        nullredr(op1R);
        DISPATCH

    /* File IO functions - mapped to C90 functions */

    /* fopen - op1 file*(int) = fopen filename op2(string) mode op3(string) */
    START_INSTRUCTION(FOPEN_REG_REG_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - FOPEN R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
        {
            int fd;
            char* filename = reg2nullstring(op2R);
            char* mode = reg2nullstring(op3R);
            op1R->int_value = (rxinteger)fopen(filename, mode);
            free(filename);
            free(mode);

            /* If the open succeeds, add the FD_CLOEXEC so that the file is not available to an ADDRESSed command! */
            if (op1R->int_value) {
#ifdef _WIN32
                // todo Windows use O_NOINHERIT
#else
                fd = fileno((FILE*)op1R->int_value);
                fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif
            }
        }
        DISPATCH

    /* fclose - op1 rc(int) = fclose op2 file*(int) */
    START_INSTRUCTION(FCLOSE_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FCLOSE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        op1R->int_value = fclose((FILE*)op2R->int_value);
        DISPATCH

    /* fflush - op1 rc(int) = fflush op2 file*(int) */
    START_INSTRUCTION(FFLUSH_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FFLUSH R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        op1R->int_value = fflush((FILE*)op1R->int_value);
        DISPATCH

    /* freadb - op1(binary) = fread op2 file*(int) op3 bytes(int) */
    START_INSTRUCTION(FREADB_REG_REG_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - FREADB R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
        {
            if (op3R->int_value == 0) {
                if (op1R->binary_value) free(op1R->binary_value);
                op1R->binary_value = 0;
                op1R->binary_length = 0;
                op1R->binary_buffer_length = 0;
            }

            else {
                /* If the binary size is the same (and this is likely to be the case if we are in a loop) we can just
                 * reuse the buffer - otherwise free/malloc */
                if (op1R->binary_length != op3R->int_value) {
                    if (op1R->binary_value) free(op1R->binary_value);
                    op1R->binary_value = malloc(op3R->int_value);
                    op1R->binary_length = op3R->int_value;
                    op1R->binary_buffer_length = op1R->binary_length;
                }

                op1R->binary_length = fread(op1R->binary_value, op1R->binary_length, 1, (FILE *) op2R->int_value);
            }
        }
        DISPATCH

    /* freadline - op1 (string) = read until newline op2 file*(int) */
    START_INSTRUCTION(FREADLINE_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FREADLINE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
    {
        int ch;

        op1R->string_length = 0;
        op1R->string_pos = 0;

        if (op2R->int_value == 0) {
            // no file - raise error
            SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "File not open");
        }

        else {
            /* Read until EOF or newline */
            while ((ch = fgetc((FILE*)op2R->int_value)) != EOF) {

                /* End of line detection (LF) */
                if (ch == '\n') break;

                /* End of line detection (CR LF, or CR (old macs) */
                if (ch == '\r') {
                    ch = fgetc((FILE*)op2R->int_value);\
                    if (ch != '\n') ungetc(ch, (FILE*)op2R->int_value);
                    break;
                }

                op1R->string_length++;
                extend_string_buffer(op1R, op1R->string_length);
                op1R->string_value[ op1R->string_length - 1] = (char)ch;
            }
        }
        }
#ifndef NUTF8
        op1R->string_char_pos = 0;
        op1R->string_chars = utf8nlen(op1R->string_value, op1R->string_length);
#endif
        DISPATCH

    /* freadbyte - op1 (int) = read byte op2 file*(int) */
    START_INSTRUCTION(FREADBYTE_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FREADBYTE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        op1R->int_value = fgetc((FILE *)op2R->int_value);
        DISPATCH

    /* freadcdpt - op1 (string and int) = read codepoint op2 file*(int) */
    START_INSTRUCTION(FREADCDPT_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FREADCDPT R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        {
#ifndef NUTF8
            int codepoint;
            op1R->string_pos = 0;
            prep_string_buffer(op1R, 4);

            /* Read the first byte - determines length */
            op1R->string_value[0] = (char)fgetc((FILE *)op2R->int_value);

            /* Read the rest of the code point */
            if ((unsigned char)op1R->string_value[0] < 128) {
                op1R->string_length = 1;
            } else if ((unsigned char)op1R->string_value[0] < 224) {
                op1R->string_length = 2;
                op1R->string_value[1] = (char)fgetc((FILE *)op2R->int_value);
            } else if ((unsigned char)op1R->string_value[0] < 240) {
                op1R->string_length = 3;
                op1R->string_value[1] = (char)fgetc((FILE *)op2R->int_value);
                op1R->string_value[2] = (char)fgetc((FILE *)op2R->int_value);
            } else {
                op1R->string_length = 4;
                op1R->string_value[1] = (char)fgetc((FILE *)op2R->int_value);
                op1R->string_value[2] = (char)fgetc((FILE *)op2R->int_value);
                op1R->string_value[3] = (char)fgetc((FILE *)op2R->int_value);
            }

            utf8codepoint(op1R->string_value, &codepoint);
            op1R->int_value = codepoint;
            op1R->string_char_pos = 0;
            op1R->string_chars = 1;
#elif
            prep_string_buffer(op1R, 1);
            op1R->int_value = (unsigned char)fgetc( (FILE*)op2R->int_value );
            op1R->string_value[0] = op1R->int_value;
            op1R->string_length = 1;
            op1R->string_pos = 0;
#endif
        }
        DISPATCH

    /* fwrite - fwrite to op1 file*(int) from op2(string) */
    START_INSTRUCTION(FWRITE_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FWRITE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        fwrite(op2R->string_value, op2R->string_length, 1, (FILE*)op1R->int_value);
        DISPATCH

    /* fwriteb - fwrite to op1 file*(int) from op2(binary) */
    START_INSTRUCTION(FWRITEB_REG_REG) CALC_DISPATCH(2)
    DEBUG("TRACE - FWRITEB R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
    fwrite(op2R->binary_value, op2R->binary_length, 1, (FILE*)op1R->int_value);
    DISPATCH

    /* fwritebyte - write byte to op1 file*(int) op2 source(int) */
    START_INSTRUCTION(FWRITEBYTE_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FWRITEBYTE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        fputc(op2R->int_value, (FILE*)op1R->int_value);
        DISPATCH

    /* fwritecdpt - write codepoint to op1 file*(int) op2 source(int) */
    START_INSTRUCTION(FWRITECDPT_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FWRITECDPT R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        {
            char codepoint[4];
            size_t length_of_codepoint;
#ifndef NUTF8
            char* end_of_codepoint;

            end_of_codepoint = utf8catcodepoint(codepoint, op2R->int_value, 4);
            length_of_codepoint = end_of_codepoint - codepoint;
#elif
            length_of_codepoint = 1;
            codepoint[0] = op2R->int_value;
#endif
            fwrite(codepoint, length_of_codepoint, 1, (FILE*)op1R->int_value);
        }
        DISPATCH

    /* fclearerr - clearerr op1 file*(int) */
    START_INSTRUCTION(FCLEARERR_REG) CALC_DISPATCH(1)
        DEBUG("TRACE - FCLEARERR R%lu\n", REG_IDX(1));
        clearerr((FILE*)op1R->int_value);
        DISPATCH

    /* feof - op1 rc(int) = feof op2 file*(int) */
    START_INSTRUCTION(FEOF_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FEOF R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        op1R->int_value = feof((FILE*)op2R->int_value);
        DISPATCH

    /* ferror - op1 rc(int) = ferror op2 file*(int) */
    START_INSTRUCTION(FERROR_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FERROR R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        op1R->int_value = ferror((FILE*)op2R->int_value);
        DISPATCH

    /* ichkrng - if op1<op2 | op1>op3 signal OUT_OF_RANGE */
    START_INSTRUCTION(ICHKRNG_REG_INT_INT) CALC_DISPATCH(3)
        DEBUG("TRACE - ICHKRNG R%lu,%d,%d\n", REG_IDX(1), (int)op2I, (int)op3I);
        if (op1R->int_value < op2I || op1R->int_value > op3I) SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE)
        DISPATCH

    /* ichkrng - if op1<op2 | op1>op3 signal OUT_OF_RANGE */
    START_INSTRUCTION(ICHKRNG_REG_INT_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - ICHKRNG R%lu,%d,R%lu\n", REG_IDX(1), (int)op2I, REG_IDX(3));
        if (op1R->int_value < op2I || op1R->int_value > op3R->int_value) SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE)
        DISPATCH

    /* ichkrng - if op1<op2 | op1>op3 signal OUT_OF_RANGE */
    START_INSTRUCTION(ICHKRNG_REG_REG_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - ICHKRNG R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
        if (op1R->int_value < op2R->int_value || op1R->int_value > op3R->int_value) SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE)
        DISPATCH

    /* ichkrng - if op1<op2 | op1>op3 signal OUT_OF_RANGE */
    START_INSTRUCTION(ICHKRNG_INT_INT_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - ICHKRNG %d,%d,R%lu\n", (int)op1I, (int)op2I, REG_IDX(3));
        if (op1I < op2I || op1I > op3R->int_value) SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE)
        DISPATCH

    /* ichkrng - if op1<op2 | op1>op3 signal OUT_OF_RANGE */
    START_INSTRUCTION(ICHKRNG_INT_REG_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - ICHKRNG %d,R%lu,R%lu\n", (int)op1I, REG_IDX(2), REG_IDX(3));
        if (op1I < op2R->int_value || op1I > op3R->int_value) SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE)
        DISPATCH

    /* getenv - get environment variable, op1=env[op2] */
    START_INSTRUCTION(GETENV_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - GETENV R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        {
            char *value;
            int should_free = getEnvVal(&value, op2R->string_value, op2R->string_length);
            set_null_string(op1R, value);
            if (should_free) free(value);
        }
        DISPATCH

    /* getenv - get environment variable, op1=env[op2] */
    START_INSTRUCTION(GETENV_REG_STRING) CALC_DISPATCH(2)
        DEBUG("TRACE - GETENV R%lu,\"%.*s\"\n", REG_IDX(1),
              (int)(CONSTSTRING_OP(2))->string_len, (CONSTSTRING_OP(2))->string);
        {
            char *value;
            int should_free = getEnvVal(&value, (CONSTSTRING_OP(2))->string, (CONSTSTRING_OP(2))->string_len);
            set_null_string(op1R, value);
            if (should_free) free(value);
        }
        DISPATCH

/* ---------------------------------------------------------------------------
 * load instructions not yet implemented generated from the instruction table
 *      and scan of this module                              pej 8. April 2021
 * ---------------------------------------------------------------------------
 */
#include "instrmiss.h"

        START_INSTRUCTION(IUNKNOWN)
        START_INSTRUCTION(INULL)
            SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
            DISPATCH

        START_INSTRUCTION(EXIT)
            DEBUG("TRACE - EXIT");
            rc = 0;
            goto interprt_finished;

        START_INSTRUCTION(EXIT_INT)
            DEBUG("TRACE - EXIT %d\n", (int)op1I);
            rc = op1I;
            goto interprt_finished;

        START_INSTRUCTION(EXIT_REG)
            DEBUG("TRACE - EXIT R%lu\n", REG_IDX(1));
            rc = op1RI;
            goto interprt_finished;

    END_OF_INSTRUCTIONS

    interprt_finished:

    /* Cleanup / Remove OS Interrupt handlers */
    cleanup_vm_signals();

    /* Unwind any stack frames */
    while (current_frame) {
        temp_frame = current_frame->parent;
        free_frame(current_frame);
        current_frame = temp_frame;
    }

    /* Deallocate Frames */
    /* We need to loop through each procedure in each module */
    DEBUG("Deallocating Frames and Registers\n");
    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        int i = context->modules[mod_index]->proc_head;
        while (i != -1) {
            proc_constant *c_entry = (proc_constant*) (context->modules[mod_index]->segment.const_pool + i);
            if ((c_entry)->start != SIZE_MAX) {
                /* Free frames in the procedures free list */
                while (*(c_entry->frame_free_list)) {
                    temp_frame = *(c_entry->frame_free_list);
                    *(c_entry->frame_free_list) = temp_frame->prev_free;
                    clear_frame(temp_frame);
                    free(temp_frame);
                }
            }
            i = c_entry->next;
        }
    }

    /* Deallocate Globals */
    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        int i;
        for (i = 0; i < context->modules[mod_index]->segment.globals; i++) {
            if (!context->modules[mod_index]->globals_dont_free[i]) {
                clear_value(context->modules[mod_index]->globals[i]);
                free(context->modules[mod_index]->globals[i]);
            }
        }
    }

    /* Free signal value */
    clear_value(signal_value);
    free(signal_value);

    /* Free interrupt argument */
    clear_value(interrupt_arg);
    free(interrupt_arg);

    /* Free array of interrupt objects - interrupt_object[] */
    {
        size_t i;
        for (i = 0; i < RXSIGNAL_MAX; i++) {
            clear_value(interrupt_object[i]);
            free(interrupt_object[i]);
        }
    }

    /* Free arguments array */
    if (arguments_array) {
        clear_value(arguments_array);
        free(arguments_array);
    }

#ifndef NDEBUG
    if (context->debug_mode) mprintf("Interpreter Finished\n");
#endif

    return rc;
}
