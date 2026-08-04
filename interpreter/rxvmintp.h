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

#ifndef CREXX_RXVMINTP_H
#define CREXX_RXVMINTP_H

#include "rxas.h"
#include "rxbin.h"
#include "rxpa.h"
#include "rxvalue.h"
#include "rxsignal.h"
#include "rxsignature.h"
#include "rxvminstrument.h"
#include "crexx_version.h"
#include <assert.h>
#include <stdint.h>

typedef enum { RXVM_MOD_LOADED, RXVM_MOD_LINKED, RXVM_MOD_THREADED } rxvm_mod_state;

typedef struct module module;
typedef struct proc_runtime {
    proc_constant *definition;
    int locals;
    bin_space *binarySpace;
    stack_frame **frame_free_list;
    stack_frame *frame_free_list_head;
    size_t start;
    char *name;
#ifdef CREXX_VM_PROFILING
    size_t profile_id;
#endif
} proc_runtime;

typedef struct rxvm_graph_provider_binding {
    const char *class_name;
    proc_runtime *factory_target;
    proc_runtime *match_target;
    uint32_t requires_match;
} rxvm_graph_provider_binding;

typedef struct rxvm_graph_factory_binding {
    const rxvm_graph_provider_binding *providers;
    const char *interface_name;
    const char *member_name;
    size_t provider_count;
    proc_runtime *direct_target;
} rxvm_graph_factory_binding;

typedef struct rxvm_graph_binding {
    const RxGraph *graph;
    proc_runtime **callable_targets;
    size_t callable_count;
    rxvm_graph_factory_binding *factory_bindings;
    size_t factory_count;
    rxvm_graph_provider_binding *provider_bindings;
    size_t provider_count;
} rxvm_graph_binding;

#define RXVM_METHOD_CACHE_WAYS 2u

typedef struct rxvm_dynamic_site_cache {
    uint64_t generation;
    union {
        struct {
            const RxGraphTypeRef *types[RXVM_METHOD_CACHE_WAYS];
            proc_runtime *targets[RXVM_METHOD_CACHE_WAYS];
            uint32_t next_way;
        } method;
        struct {
            const rxvm_graph_factory_binding *binding;
            proc_runtime *target;
            uint32_t state;
        } factory;
    } value;
} rxvm_dynamic_site_cache;

enum rxvm_factory_cache_state {
    RXVM_FACTORY_CACHE_UNKNOWN = 0,
    RXVM_FACTORY_CACHE_DIRECT = 1,
    RXVM_FACTORY_CACHE_GENERAL = 2
};

typedef struct proc_runtime_lookup_entry {
    size_t offset;
    proc_runtime *runtime;
} proc_runtime_lookup_entry;

/* Module Structure */
struct module {
    bin_space segment;         /* Binary and Constant Pool */
    char *name;                /* Module Name */
    char *description;         /* Module Description */
    unsigned char native;      /* Native Module */
    value **globals;           /* Globals registers array */
    int proc_head;             /* Offset to the head of the procs in the constant pool */
    int expose_head;           /* Offset to the head of the exposed procs in the constant pool */
    int meta_head;             /* Offset to the head of the meta data in the constant pool */
    char *globals_dont_free;   /* Indicates linked global value that should not be freed */
    size_t module_number;      /* Module Index - 1 base */
    size_t unresolved_symbols; /* Number of symbols not yet resolved by linking */
    size_t duplicated_symbols; /* Number of duplicated symbols ignored in module */
    module_file *file;         /* File section the module was loaded from */
    rxvm_mod_state state;      /* Module lifecycle state */
    proc_runtime *procedures;  /* Runtime procedure state */
    size_t procedure_count;    /* Number of runtime procedures */
    proc_runtime_lookup_entry *proc_runtime_lookup; /* Sorted constant pool offsets -> runtime procedures */
    size_t proc_runtime_lookup_size;
    bin_code *execution_image; /* Owned process-local operand/dispatch image; canonical segment.binary stays immutable */
    rxvm_graph_binding *graph_binding; /* Shared process-local callable bindings for file->semantic_graph */
    uint32_t *dynamic_site_cache_slots; /* Instruction-word index -> cache slot, or UINT32_MAX */
    rxvm_dynamic_site_cache *dynamic_site_caches;
    size_t dynamic_site_cache_count;
};

static inline proc_runtime *rxvm_bound_graph_callable(
        const rxvm_graph_binding *binding,
        RxCallableId callable) {
    if (!binding || callable >= binding->callable_count ||
        !binding->callable_targets) return 0;
    return binding->callable_targets[callable];
}

static inline const rxvm_graph_factory_binding *rxvm_bound_graph_factory(
        const rxvm_graph_binding *binding,
        RxFactoryId factory) {
    if (!binding || factory >= binding->factory_count ||
        !binding->factory_bindings) return 0;
    return &binding->factory_bindings[factory];
}

static inline rxvm_dynamic_site_cache *rxvm_dynamic_cache_for_site(
        module *mod,
        size_t instruction_index) {
    uint32_t slot;

    if (!mod || instruction_index >= mod->segment.inst_size ||
        !mod->dynamic_site_cache_slots) return 0;
    slot = mod->dynamic_site_cache_slots[instruction_index];
    if (slot == UINT32_MAX || slot >= mod->dynamic_site_cache_count) return 0;
    return &mod->dynamic_site_caches[slot];
}

static inline proc_runtime *rxvm_get_module_runtime_procedure(module *mod, size_t proc_offset) {
    size_t left;
    size_t right;

    if (!mod || proc_offset >= mod->segment.const_size || !mod->proc_runtime_lookup) return 0;

    left = 0;
    right = mod->proc_runtime_lookup_size;
    while (left < right) {
        size_t mid = left + ((right - left) >> 1);
        size_t offset = mod->proc_runtime_lookup[mid].offset;
        if (offset == proc_offset) return mod->proc_runtime_lookup[mid].runtime;
        if (offset < proc_offset) left = mid + 1;
        else right = mid;
    }

    return 0;
}

/* Interrupt Response Codes */
typedef enum interrupt_response {
    RXSIGNAL_RESPONSE_IGNORE = 0,   /* Ignore the interrupt */
    RXSIGNAL_RESPONSE_RETURN,       /* Return to the stack frame - the stack is unwound to the frame */
    RXSIGNAL_RESPONSE_BRANCH,       /* Branch to an address in a stack frame - the stack is unwound to the frame */
    RXSIGNAL_RESPONSE_BRANCH_VALUE, /* Branch to an address and bind a .signal object in the handler frame */
    RXSIGNAL_RESPONSE_CALL,         /* Call a function - will return to the current instruction */
    RXSIGNAL_RESPONSE_CALL_BRANCH,  /* Call a function - will return to the BRANCH address (the stack is unwound to the frame) */
    RXSIGNAL_RESPONSE_CALL_ACTION,  /* Call a function and interpret its returned signal action */
    RXSIGNAL_RESPONSE_HALT,         /* Unmasked Interrupt, halt the interpreter with an error message and return interrupt code */
    RXSIGNAL_RESPONSE_SILENT_HALT   /* Halt the interpreter without any message - return 0 */
} interrupt_response;

/* Interrupt Table Entry */
typedef struct interrupt_entry {
    interrupt_response response;    /* Response to the interrupt */
    proc_runtime *function;         /* Address of the function to call */
    size_t jump;                    /* Address to jump to */
    stack_frame *frame;             /* Frame that owns branch/jump handlers */
    size_t value_register;          /* Destination register for branch handlers with a signal object */
} interrupt_entry;

typedef struct interrupt_saved_entry {
    unsigned char signal;
    interrupt_entry entry;
    struct interrupt_saved_entry *next;
} interrupt_saved_entry;

typedef struct rxvm_interface_factory_entry {
    char *interface_name;
    size_t interface_name_length;
    char *factory_name;
    size_t factory_name_length;
    char *descriptor;
    size_t descriptor_length;
    rx_callable_signature signature;
    char *class_name;
    size_t class_name_length;
    proc_runtime *match_proc;
    proc_runtime *factory_proc;
} rxvm_interface_factory_entry;

typedef struct rxvm_interface_method_entry {
    char *class_name;
    size_t class_name_length;
    char *descriptor;
    size_t descriptor_length;
    proc_runtime *method_proc;
} rxvm_interface_method_entry;

struct stack_frame {
    stack_frame *prev_free;
    stack_frame *parent;
    proc_runtime *procedure;
    bin_code *return_pc;
    value *return_reg;
    size_t number_locals;
    size_t nominal_number_locals;
    size_t number_args;
    unsigned char has_reference_lifetimes; /* Frame-owned storage has reference cells to release on exit */
    unsigned char is_interrupt;  /* Set to the interrupt number that the frame is handling (or zero) */
    unsigned char is_interrupt_action; /* Set when an interrupt handler return value is action-aware */
    uint32_t caller_arg_base; /* First caller call-window argument, or UINT32_MAX */
    interrupt_entry *interrupt_table; /* Inherited table; private after first frame-local mutation */
    unsigned char interrupt_table_owned; /* This frame owns and must release interrupt_table */
    interrupt_saved_entry *interrupt_stack; /* Block-scoped saved interrupt handlers */
    numeric_context num_context; /* Numeric context for the procedure */
    struct decplugin *decimal;
    char decimal_loaded_here;
    struct uniplugin *unicode;
    char unicode_loaded_here;
    value **baselocals; /* Initial / base / fixed local pointers */
    value **locals;     /* Locals pointer mapping (after swaps / links) */
};

#ifdef NDEBUG  // RELEASE
    #define DEBUG(...) (void)0
#else          // DEBUG
    #define DEBUG(...) if (context->debug_mode) fprintf(stderr, __VA_ARGS__)
#endif

#define RXERROR(...)   { fprintf(stderr, __VA_ARGS__); goto SIGNAL; }
#define MAP_ADDR(instr, op1, op2, op3, target, msg)             \
                instruction = src_inst(instr, op1,op2,op3);     \
                address_map[instruction->opcode] = target;

#define VM_CANONICAL_INDEX(pointer_) ((size_t)((pointer_) - current_execution_base))
#define VM_CANONICAL_POINTER(index_) (current_canonical_base + (size_t)(index_))
#define VM_EXECUTION_POINTER(index_) (current_execution_base + (size_t)(index_))

#define VM_MODULE_EXECUTION_BASE(module_) ((module_)->execution_image)

#ifndef NDEBUG
#define VM_ASSERT_ACTIVE_FRAME()                                                \
    do {                                                                        \
        assert(current_frame);                                                  \
        assert(current_frame->procedure);                                       \
        assert(current_frame->procedure->binarySpace == current_binary_space);  \
        assert(current_binary_space->module == current_module);                 \
        assert(current_binary_space->binary == current_canonical_base);         \
        assert(current_execution_base == VM_MODULE_EXECUTION_BASE(current_module)); \
        assert(current_binary_space->const_pool == current_const_pool);         \
        assert(current_frame->locals == current_locals);                        \
    } while (0)
#else
#define VM_ASSERT_ACTIVE_FRAME() ((void)0)
#endif

#define VM_ACTIVATE_FRAME(frame_, reason_)                                      \
    do {                                                                        \
        stack_frame *vm_frame__ = (frame_);                                     \
        current_frame = vm_frame__;                                             \
        current_binary_space = vm_frame__->procedure->binarySpace;              \
        current_module = current_binary_space->module;                          \
        current_execution_base = VM_MODULE_EXECUTION_BASE(current_module);      \
        current_canonical_base = current_binary_space->binary;                  \
        current_const_pool = current_binary_space->const_pool;                  \
        current_locals = vm_frame__->locals;                                    \
        VM_ASSERT_ACTIVE_FRAME();                                               \
        RXVM_INSTRUMENTATION_FRAME_ACTIVATE(                                    \
                vm_frame__,                                                     \
                current_module->module_number,                                  \
                vm_frame__->procedure->start,                                   \
                (reason_));                                                     \
    } while (0)

#define VM_DEACTIVATE_FRAME()                                                   \
    do {                                                                        \
        RXVM_INSTRUMENTATION_INSTRUCTION_TERMINAL(                              \
                current_module ? current_module->module_number : 0,             \
                (current_execution_base && pc)                                  \
                    ? VM_CANONICAL_INDEX(pc) : 0,                               \
                RXVM_TRANSITION_TERMINAL);                                      \
        current_frame = 0;                                                      \
        current_binary_space = 0;                                               \
        current_module = 0;                                                     \
        current_execution_base = 0;                                             \
        current_canonical_base = 0;                                             \
        current_const_pool = 0;                                                 \
        current_locals = 0;                                                     \
    } while (0)

#define VM_ACTIVATE_FRAME_OR_NULL(frame_, reason_)                              \
    do {                                                                        \
        stack_frame *vm_frame_or_null__ = (frame_);                             \
        if (vm_frame_or_null__) VM_ACTIVATE_FRAME(vm_frame_or_null__, reason_); \
        else VM_DEACTIVATE_FRAME();                                             \
    } while (0)

#ifdef NTHREADED

#define START_OF_INSTRUCTIONS CASE_START:; switch ((instructions)(pc->instruction.opcode)) {
#define END_OF_INSTRUCTIONS default: SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION); DISPATCH; }
#define START_INSTRUCTION(inst) case OP_ ## inst: RXVM_INSTRUMENTATION_INSTRUCTION_BEGIN(current_module->module_number, VM_CANONICAL_INDEX(pc), OP_ ## inst);
#define START_INTERRUPT INTERRUPT: RXVM_INSTRUMENTATION_INTERRUPT_SCAN_BEGIN(current_module->module_number, VM_CANONICAL_INDEX(pc));
#define END_INTERRUPT do { goto CASE_START; } while (0);
#define VM_RESOLVE_SELECTED() do { } while (0)
#define VM_DISPATCH_TARGET() goto CASE_START

#else

#define START_OF_INSTRUCTIONS
#define END_OF_INSTRUCTIONS
#define START_INSTRUCTION(inst) inst: RXVM_INSTRUMENTATION_INSTRUCTION_BEGIN(current_module->module_number, VM_CANONICAL_INDEX(pc), OP_ ## inst);
#define START_INTERRUPT INTERRUPT: RXVM_INSTRUMENTATION_INTERRUPT_SCAN_BEGIN(current_module->module_number, VM_CANONICAL_INDEX(pc));
#define END_INTERRUPT do { goto *next_inst; } while (0);
#define VM_RESOLVE_SELECTED()                                                   \
    do {                                                                        \
        next_inst = next_pc->handler;                                           \
    } while (0)
#define VM_DISPATCH_TARGET() goto *next_inst

#endif

#define VM_ADVANCE(operand_count_)                                              \
    do {                                                                        \
        size_t vm_operand_count__ = (size_t)(operand_count_);                   \
        next_pc = pc + vm_operand_count__ + 1;                                  \
        VM_RESOLVE_SELECTED();                                                  \
        RXVM_INSTRUMENTATION_TRANSITION(RXVM_TRANSITION_SEQUENTIAL);            \
    } while (0)

#define VM_SELECT_INDEX(index_, reason_)                                        \
    do {                                                                        \
        size_t vm_index__ = (size_t)(index_);                                   \
        next_pc = VM_EXECUTION_POINTER(vm_index__);                             \
        VM_RESOLVE_SELECTED();                                                  \
        RXVM_INSTRUMENTATION_TRANSITION(reason_);                               \
    } while (0)

#define VM_SELECT_POINTER(pointer_, reason_)                                    \
    do {                                                                        \
        bin_code *vm_pointer__ = (pointer_);                                    \
        next_pc = vm_pointer__;                                                 \
        VM_RESOLVE_SELECTED();                                                  \
        RXVM_INSTRUMENTATION_TRANSITION(reason_);                               \
    } while (0)

#define DISPATCH                                                                \
    do {                                                                        \
        RXVM_INSTRUMENTATION_INSTRUCTION_RETIRE(                                \
                current_module->module_number, VM_CANONICAL_INDEX(next_pc),     \
                RXVM_INSTRUMENTATION_CURRENT_TRANSITION());                     \
        pc = next_pc;                                                           \
        RXVM_INSTRUMENTATION_INTERRUPT_POLL();                                  \
        if (interrupts && !current_frame->is_interrupt) goto INTERRUPT;          \
        VM_DISPATCH_TARGET();                                                   \
    } while (0)

#define VM_RESUME_INTERRUPTED(signal_)                                          \
    do {                                                                        \
        unsigned char vm_signal__ = (unsigned char)(signal_);                   \
        RXVM_INSTRUMENTATION_INSTRUCTION_RETIRE(                                \
                current_module->module_number, VM_CANONICAL_INDEX(next_pc),     \
                RXVM_TRANSITION_INTERRUPT_RESUME);                              \
        pc = next_pc;                                                           \
        RXVM_INSTRUMENTATION_INTERRUPT_RESUME(                                  \
                vm_signal__, current_module->module_number,                     \
                VM_CANONICAL_INDEX(pc));                                        \
        VM_DISPATCH_TARGET();                                                   \
    } while (0)

#define REG_OP(n)                    current_locals[(pc+(n))->index]
#define REG_VAL(n)                   current_locals[n]
#define REG_IDX(n)                   (pc+(n))->index
#define INT_OP(n)                    (pc+(n))->iconst
#define FLOAT_OP(n)                  FLOAT_CONST_VALUE(current_const_pool, (pc+(n))->index)

#define CONSTSTRING_OP(n)            ((string_constant *)(current_const_pool + (pc+(n))->index))
#define PROC_OP(n)                   ((proc_runtime *)((pc+(n))->handler))
#define INT_VAL(vx)                  vx->int_value
#define FLOAT_VAL(vx)                vx->float_value

//
// PEJ Macros   April 2021

#define REG_RETURN_INT(val)        { set_int(REG_OP(1),val);}
#define REG_RETURN_FLOAT(val)      { set_float(REG_OP(1),val);  }
#define REG_RETURN_STRING(val)     { set_const_string(REG_OP(1), val);}

#define REG_RET_CHAR(val)          { v1=REG_OP(1); if (v1) set_char(v1,val);                               \
                                    else REG_OP(1) = value_char_f(current_frame,val); }
// TODO: String to integer just for real integers, or stop converting at "."
// maximum size of rxinteger is 20 digits plus sign
// maximum size of double is about 16 decimal digits plus sign

#define S2INT(t,s)                 { if ((s)->string_length>20)  goto convlength;                       \
                                    (s)->string_value[(s)->string_length]='\0';                         \
                                    (t) = strtol((s)->string_value, &converr, 0);                         \
                                    if (converr[0] != '\0' && converr[0]!='.') goto converror; }

#define S2FLOAT(t,s)              { if ((s)->string_length>16)  goto convlength;                       \
                                    (s)->string_value[(s)->string_length]='\0';                         \
                                    (t) = strtod((s)->string_value, &converr);                          \
                                    if (converr[0] != '\0') goto converror; }

#define CONV2INT(i,v)             { S2INT(i,v); }

#define CONV2FLOAT(i,v)           { S2FLOAT(i,v); }                                               \
                                                                                                      \
// Get Character
#ifndef NUTF8
  #define GETSTRCHAR(c,v,p) {string_cache_seek_char((v),(p)); utf8codepoint((v)->string_value+(v)->string_cache_byte_pos, &(c));}
#else
  #define GETSTRCHAR(c,v,p) {c=(v)->string_value[(p)]; }
#endif

#ifndef NUTF8
  #define GETSTRLEN(i,v)   { i = (rxinteger) v->string_chars; }
#else
  #define GETSTRLEN(i,v)   { i = (rxinteger) v->string_length; }
#endif

#define PUTSTRLEN(v,i)      { string_set_ascii_length((v), (size_t)(i)); }



// TODO PEJ what kind of checks must be performed in runtime/debug mode
#define REG_TEST(v)            { if (!(v)) goto notreg; }
#define op1R                     (REG_OP(1))
#define op2R                     (REG_OP(2))
#define op3R                     (REG_OP(3))
#define op4R                     (REG_OP(4))
#define op1I                     (INT_OP(1))
#define op2I                     (INT_OP(2))
#define op3I                     (INT_OP(3))
#define op1F                     (FLOAT_OP(1))
#define op2F                     (FLOAT_OP(2))
#define op3F                     (FLOAT_OP(3))
#define op1S                     (CONSTSTRING_OP(1))
#define op2S                     (CONSTSTRING_OP(2))
#define op3S                     (CONSTSTRING_OP(3))
#define op1RI                    (INT_VAL(op1R))
#define op2RI                    (INT_VAL(op2R))
#define op3RI                    (INT_VAL(op3R))
#define op4RI                    (INT_VAL(op4R))
#define op2RF                    (FLOAT_VAL(op2R))
#define op3RF                    (FLOAT_VAL(op3R))
#define REG_OP_TEST(v,n)        { (v) = REG_OP(n);}
//#define REG_OP_TEST_INT(v,n)   { (v) = REG_OP(n); REG_TEST(v);                                      \
//                                 if ((v)->status.type_int==0)  goto notint; }
//#define REG_OP_TEST_FLOAT(v,n)  { (v) = REG_OP(n); REG_TEST(v);                                      \
//                                   if ((v)->status.type_float==0)  goto notfloat; }
#define REG_OP_TEST_INT(v,n)    { (v) = REG_OP(n);}
#define REG_OP_TEST_FLOAT(v,n)  { (v) = REG_OP(n);}

/* Runtime context */
typedef struct rxvm_context {
    char *location;
    size_t num_modules;
    size_t module_buffer_size;
    module **modules;
    struct avl_tree_node *exposed_proc_tree;
    struct avl_tree_node *exposed_reg_tree;
    char debug_mode;

    /* Extra fields for direct procedure call */
    proc_runtime *ext_proc;
    int ext_argc;
    value **ext_args;
    value *ext_ret;
    int prepare_only;

    rxvm_interface_factory_entry *interface_factories;
    size_t num_interface_factories;
    size_t interface_factory_capacity;
    rxvm_interface_method_entry *interface_methods;
    size_t num_interface_methods;
    size_t interface_method_capacity;
    rxvm_graph_binding **graph_bindings;
    size_t graph_binding_count;
    size_t graph_binding_capacity;
    uint64_t semantic_generation;
    struct rxvm_socket_registry *socket_registry;
    rxvm_reference_context references;
    char link_dirty;
    char interface_method_registry_dirty;
    char interface_factory_registry_dirty;
#ifdef CREXX_VM_PROFILING
    /* Keep optional build-local fields last so existing field offsets stay stable. */
    char profile_mode;
    const char *profile_output;
    unsigned int sequence_count;
    const char *sequence_output;
#endif
} rxvm_context;

/* Function to get signal text from a signal code  */
char* rxvm_getsignaltext(rxsignal signal);

/* Function to get a signal code from a signal text */
rxsignal rxvm_getsignalcode(char* signalText);

int initialz();
int finalize();
int run(rxvm_context *context, int argc, char *argv[]);

/* Initialise modules context */
void rxinimod(rxvm_context *context);

/* Free Module Context */
void rxfremod(rxvm_context *context);
void completely_free_frame(stack_frame *frame);

/* Link a loaded module */
void rxvm_link_module(rxvm_context *context, size_t module_number_to_link);
void rxvm_rebuild_interface_factory_registry(rxvm_context *context);
void rxvm_rebuild_interface_method_registry(rxvm_context *context);
void rxvm_rebuild_graph_bindings(rxvm_context *context);
void rxvm_free_graph_bindings(rxvm_context *context);

/* Loads a new module
 * returns 0  - Error
 *         >0 - Last Module Number loaded (1 based) (more than one might have been loaded ...)  */
int rxldmod(rxvm_context *context, char *new_module_file);

/* Loads a module from a memory buffer
 * returns 0  - Error
 *         >0 - Last Module Number loaded (1 based) (more than one might have been loaded ...)  */
int rxldmodm(rxvm_context *context, char *buffer_start, size_t buffer_length);

/* Loads statically loaded plugins
 * returns -1  - Error
 *         >=0 - Last Module Number loaded (1 based) (more than one, or none, might have been loaded ...)  */
int rxldmodp(rxvm_context *context);

/* Function to call a native RXPA (CREXX Plugin Architecture) function */
void rxvm_callfunc(void* function, int args, value** argv, value* ret, value* signal);

/* Private structure for output to string thread */
typedef struct redirect REDIRECT;

/* Resolve an internal native-payload redirect endpoint value. */
REDIRECT *rxspawn_redirect_from_value(value *redirect_reg);

/* Get Environment Value
 * Sets value (null terminated) (and a handle) from env variable name length name_length (not null terminated)
 * Value can be set to point to a zero length string (if the variable is not set)
 *
 * Returns 1 if value should bee free()d
 * Otherwise returns 0
 */
int getEnvVal(char **value, char *name, size_t name_length);

/*
 * - A pin, pout or perr does not need to be specified ... in this case the std streams are used.
 * - Command contains the commands string to execute
 * - rc will contain the return code from the command
 * - errorText contains a descriptive text of any error in the spawn
 *   (i.e. NOT from the executed child process). This is set if this returns
 *   a non-zero return code.
 *
 * Return codes
 *  0 - SHELLSPAWN_OK         - All OK
 *  4 - SHELLSPAWN_NOFOUND    - The command was not found
 *  5 - SHELLSPAWN_FAILURE    - Spawn failed unexpectedly (see error text for details)
*/
int shellspawn(const char *command,
               REDIRECT* pIn,
               REDIRECT* pOut,
               REDIRECT* pErr,
               value* variables,
               value* crexx_bindings,
               int mode,
               int *rc,
               char **errorText);

// SPAWN Error codes
#define SHELLSPAWN_OK         0
#define SHELLSPAWN_NOFOUND    4
#define SHELLSPAWN_FAILURE    5

// SPAWN execution modes
#define SHELLSPAWN_MODE_PATH              0
#define SHELLSPAWN_MODE_SHELL             1
#define SHELLSPAWN_MODE_CREXX             2
#define SHELLSPAWN_MODE_CONFIGURED_SHELL  3

/* Create a redirect pipe to string */
/* the redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void redr2str(value* redirect_reg, value* string_reg);

/* Create a redirect pipe to string */
/* the redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void redr2arr(value* redirect_reg, value* string_reg);

/* Create a redirect pipe from a string */
/* the redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void str2redr(value* redirect_reg, value* string_reg);

/* Create a redirect pipe from an array */
/* the redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void arr2redr(value* redirect_reg, value* string_reg);

/* Create a null redirect pipe */
/* In general, he redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void nullredr(value* redirect_reg);

/* Write to and finalize a redirect endpoint without launching a process. */
int redrwriteclose(value* redirect_reg, const char* data, size_t nBytes);

/* EXIT Function Support */
void rxvm_setsayexit(say_exit_func sayExitFunc);
void rxvm_resetsayexit();
void rxvm_mprintf(const char* format, ...); /* printf replacement - prints to the say exit function (or stdout) */

/**
 * @brief Enables handling for a specific VM interrupt code.
 * Translates the VM code to an OS signal and registers the master handler.
 * Does nothing if the VM code doesn't map to a catchable OS signal.
 *
 * @param vm_signal The RXSIGNAL_* code to enable.
 * @return 0 on success or if no action is needed, -1 on failure to register handler.
 */
int enable_interrupt(int vm_signal);

/**
 * @brief Sets the specific VM interrupt code to be ignored, meaning the linked OS signal is ignored.
 * Does nothing if the VM code doesn't map to a catchable OS signal.
 *
 * @param vm_signal The RXSIGNAL_* code to ignore.
 * return 0 on success or if no action is needed, -1 on failure to register handler.
 */
int ignore_interrupt(int vm_signal);

/**
 * @brief Restores handling for a specific VM interrupt code.
 * Restores the original signal handler for the corresponding OS signal.
 *
 * @param vm_signal The RXSIGNAL_* code to disable.
 * @return 0 on success or if no action needed, -1 on failure to restore handler.
 */
int restore_interrupt(int vm_signal);

/**
 * @brief Initializes the VM signal handling system.
 * Clears flags, initializes storage, and prepares for enabling interrupts.
 * Should be called once at VM startup.
 * @return 0 on success.
 */
int initialize_vm_signals(void);

/**
 * @brief Cleans up signal handlers, restoring originals for active ones.
 * Intended to be called via atexit or manually before VM shutdown.
 */
void cleanup_vm_signals(void);

/**
 * @brief Sets an interrupt signal.
 * This function is used to set a specific VM interrupt signal.
 * @param signal The signal to set.
 */
void raise_signal(unsigned char signal);

/**
 * @brief Clears an interrupt signal.
 * This function is used to clear a specific VM interrupt signal.
 * @param signal The signal to clear.
 */
void clear_signal(unsigned char signal);

#endif //CREXX_RXVMINTP_H
