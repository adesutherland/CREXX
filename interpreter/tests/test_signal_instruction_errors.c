/* cREXX License (MIT). Independent signal instruction and real-child oracle. */
#include "rxvm.h"
#include "rxvmintp.h"
#include "rxvmvars.h"
#include "rxvml.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "FAIL signal instruction line %d: %s\n", __LINE__, #condition); \
    ++failures; } } while (0)

typedef struct probe {
    int arms, native_raises;
#ifdef CREXX_VM_INSTRUMENTATION_TEST
    int observations;
    const stack_frame *frame;
    interrupt_entry *table;
    interrupt_saved_entry *stack;
    unsigned char owned;
    interrupt_entry entries[RXSIGNAL_MAX];
#endif
} probe;

#ifdef CREXX_VM_INSTRUMENTATION_TEST
/* Observe immediately before the denied allocation and at terminal exit,
 * before cleanup. Neither a save nor a private policy may be published. */
static void observe_failure(const stack_frame *frame, void *userdata) {
    probe *state = userdata;
    CHECK(frame && frame->interrupt_table);
    if (!frame || !frame->interrupt_table) return;
    if (!state->observations) {
        state->frame = frame;
        state->table = frame->interrupt_table;
        state->stack = frame->interrupt_stack;
        state->owned = frame->interrupt_table_owned;
        memcpy(state->entries, frame->interrupt_table, sizeof(state->entries));
    } else {
        CHECK(state->frame == frame && state->table == frame->interrupt_table);
        CHECK(state->stack == frame->interrupt_stack && state->owned == frame->interrupt_table_owned);
        CHECK(!memcmp(state->entries, frame->interrupt_table, sizeof(state->entries)));
    }
    ++state->observations;
}
#endif

static int output_writer(int stream, const char *bytes, size_t length, void *userdata) {
    probe *state = userdata;
    CHECK(stream == RXVML_OUTPUT_STDOUT);
    if (length == 3 && !memcmp(bytes, "arm", 3)) {
        ++state->arms;
#ifdef CREXX_VM_INSTRUMENTATION_TEST
        rxvm_signal_instruction_test_observe(observe_failure, state);
        rxvm_signal_instruction_test_fail_after(0);
#endif
    } else if (length == 12 && !memcmp(bytes, "raise-native", 12)) {
        ++state->native_raises;
#if !defined(_WIN32) && !defined(CREXX_VM_SINGLE_THREADED)
        CHECK(raise(SIGUSR1) == 0);
#endif
    } else CHECK(0);
    return 0;
}

static proc_runtime *find_procedure(module *owner, const char *name) {
    size_t index;
    for (index = 0; index < owner->procedure_count; ++index)
        if (!strcmp(owner->procedures[index].name, name)) return &owner->procedures[index];
    return NULL;
}

static rxvm_context *open_context(const char *image, probe *state) {
    rxvm_context *context = rxvm_create();
    CHECK(context);
    if (!context) return NULL;
    CHECK(rxvm_set_autoload(context, 0) == 0);
    CHECK(rxvm_load_file(context, (char *)image) != 0);
    CHECK(rxvm_link(context) == 0 && rxvm_prepare(context) == 0);
    CHECK(rxvm_initialize(context) == 0);
    CHECK(context->num_modules == 1 && context->modules[0]->segment.globals == 3);
    set_int(context->modules[0]->globals[0], 0);
    set_int(context->modules[0]->globals[1], 0);
    set_int(context->modules[0]->globals[2], 0);
    context->active.output_writer = output_writer;
    context->active.output_userdata = state;
    return context;
}

static void check_self_error(const char *image, int selector, int expected, int target_kind) {
    probe state = {0};
    rxvm_context *context = open_context(image, &state);
    module *owner;
    proc_runtime *target;
    size_t original_start = 0;
    unsigned char original_entry = 0;
    size_t address;
    int status;
    if (!context) return;
    owner = context->modules[0];
    target = find_procedure(owner, "target");
    CHECK(target);
    if (!target) { rxvm_destroy(context); return; }
    set_int(owner->globals[2], selector);
    if (selector == 33) set_string(owner->globals[0], "OTHER\0BAD", 9);
    if (selector == 25 || selector == 26) {
        original_start = target->start;
        original_entry = target->entry_state_kind;
        if (target_kind) target->entry_state_kind = RXBIN_ENTRY_PROGRAM_DEFAULT;
        else target->start = SIZE_MAX;
    }
    if (selector >= 27 && selector <= 29) {
        int opcode = selector == 27 ? OP_SIGBR_ID_STRING :
                     selector == 28 ? OP_SIGBRV_ID_REG_STRING : OP_SIGCALLBR_ID_FUNC_STRING;
        for (address = 0; address < owner->segment.inst_size;
             address += 1u + owner->segment.binary[address].instruction.no_ops)
            if (owner->segment.binary[address].instruction.opcode == opcode) {
                owner->segment.binary[address + 1u].index = SIZE_MAX;
                if (owner->execution_image) owner->execution_image[address + 1u].index = SIZE_MAX;
            }
    }
    status = rxvm_call(context, "signaltest.selferror", 0, NULL);
    if (status != expected) fprintf(stderr, "selector=%d target=%d status=%d expected=%d\n",
                                   selector, target_kind, status, expected);
    CHECK(status == expected);
    CHECK(owner->globals[0]->int_value == 0 && owner->globals[1]->int_value == 0);
    if (selector >= 30 && selector <= 32) {
        CHECK(state.arms == 1);
#ifdef CREXX_VM_INSTRUMENTATION_TEST
        CHECK(state.observations == 2);
        rxvm_signal_instruction_test_reset();
#endif
    }
    if (selector == 25 || selector == 26) {
        target->start = original_start;
        target->entry_state_kind = original_entry;
    }
    /* A failed run must leave its embedding context reusable. The requested
     * OTHER event is caught, while false conditional invalid tags stay idle. */
    CHECK(rxvm_call(context, "signaltest.good", 0, NULL) == 0);
    CHECK(owner->globals[1]->int_value == 1);
    rxvm_destroy(context);
}

static void check_native_modes(const char *image) {
#if !defined(_WIN32) && !defined(CREXX_VM_SINGLE_THREADED)
    probe state = {0};
    struct sigaction original, expected, restored;
    rxvm_context *context = open_context(image, &state);
    const char *entries[] = {"signaltest.native.normal", "signaltest.native.unwind"};
    size_t index;
    if (!context) return;
    if (rxvm_signal_bind_process_main(context) != 0) {
        CHECK(0);
        rxvm_destroy(context);
        return;
    }
    CHECK(sigaction(SIGUSR1, NULL, &original) == 0);
    memset(&expected, 0, sizeof(expected));
    expected.sa_handler = SIG_IGN;
    sigemptyset(&expected.sa_mask);
    sigaddset(&expected.sa_mask, SIGUSR2);
    expected.sa_flags = SA_RESTART;
    CHECK(sigaction(SIGUSR1, &expected, NULL) == 0);
    for (index = 0; index < sizeof(entries) / sizeof(entries[0]); ++index) {
        state.native_raises = 0;
        set_int(context->modules[0]->globals[0], 0);
        CHECK(rxvm_call(context, (char *)entries[index], 0, NULL) == 0);
        CHECK(state.native_raises == 1 && context->modules[0]->globals[0]->int_value == 1);
        CHECK(sigaction(SIGUSR1, NULL, &restored) == 0);
        CHECK(restored.sa_handler == expected.sa_handler &&
              sigismember(&restored.sa_mask, SIGUSR2) == 1 &&
              (restored.sa_flags & SA_RESTART) == SA_RESTART);
    }
    CHECK(sigaction(SIGUSR1, &original, NULL) == 0);
    rxvm_destroy(context);
#else
    (void)image;
#endif
}

int main(int argc, char **argv) {
    int selector;
    if (argc != 2) return 2;
    for (selector = 1; selector <= 29; ++selector) {
        int expected = selector == 20 || selector >= 25 ?
                       RXSIGNAL_INVALID_ARGUMENTS : RXSIGNAL_INVALID_SIGNAL_CODE;
        check_self_error(argv[1], selector, expected, 0);
        if (selector == 25 || selector == 26)
            check_self_error(argv[1], selector, expected, 1);
    }
#ifdef CREXX_VM_INSTRUMENTATION_TEST
    for (selector = 30; selector <= 32; ++selector)
        check_self_error(argv[1], selector, RXSIGNAL_FAILURE, 0);
#endif
    check_self_error(argv[1], 33, RXSIGNAL_INVALID_SIGNAL_CODE, 0);
    check_native_modes(argv[1]);
    printf("SIGNAL_INSTRUCTION_ERRORS|failures=%d\n", failures);
    return failures ? 1 : 0;
}
