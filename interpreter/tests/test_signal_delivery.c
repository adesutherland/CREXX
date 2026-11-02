/* cREXX License (MIT). Public embedding oracle for atomic event publication. */
#include "rxvml.h"
#include <stdio.h>
#include <string.h>

typedef struct delivery_probe {
    rxvml_context *context;
    int mutate;
    int hooks;
    int ready_copies;
    int nested_calls;
    int errors;
} delivery_probe;

static int failures;
static void copy_payload(void *destination, void *source);
static const rxvm_native_payload_ops payload_ops = {
    "signal.delivery.probe", copy_payload, NULL
};

static void check(int condition, const char *message) {
    if (condition) return;
    fprintf(stderr, "FAIL: %s\n", message);
    ++failures;
}

static void copy_payload(void *destination, void *source) {
    delivery_probe *probe = NULL;
    const rxvm_native_payload_ops *ops = NULL;
    unsigned int flags = 0;
    size_t length = 0;
    void *data = rxvml_get_native_payload((rxvml_value *)source,
                                         &length, &ops, &flags);
    if (!data || ops != &payload_ops || length != sizeof(probe)) {
        check(0, "COPY received an invalid native probe payload");
        return;
    }
    memcpy(&probe, data, sizeof(probe));
    ++probe->hooks;
    if (rxvml_num_attributes((rxvml_value *)source) >= 2) {
        rxvml_value *ready = rxvml_get_attribute(probe->context,
                                                (rxvml_value *)source, 2);
        rxinteger enabled = 0;
        if (!ready || rxvml_to_int(probe->context, ready, &enabled)) ++probe->errors;
        if (ready) rxvml_value_free(ready);
        if (enabled == 1) {
            ++probe->ready_copies;
            /* The first armed COPY captures the SIGNAL operand while its
             * event is pending. Exercise delivery and later COPY callbacks. */
            if (probe->mutate && probe->ready_copies > 1) {
                rxvml_value *reference = rxvml_get_attribute(probe->context,
                                                        (rxvml_value *)source, 1);
                rxvml_value *reply = NULL;
                int status = -1;
                ++probe->nested_calls;
                if (reference) {
                    status = rxvml_call_procedure_descriptor(probe->context,
                        "rxsig1|signal_delivery.mutate|.int|reference=.object",
                        1, &reference, &reply);
                }
                if (status != 0) ++probe->errors;
                if (reply) rxvml_value_free(reply);
                if (reference) rxvml_value_free(reference);
            }
        }
    }
    if (rxvml_set_native_payload((rxvml_value *)destination,
                                 &probe, sizeof(probe), &payload_ops, flags))
        ++probe->errors;
}

static void run_case(const char *image, int mutate) {
    delivery_probe probe;
    delivery_probe *payload_probe = &probe;
    rxvml_value *target = NULL, *payload = NULL, *reply = NULL;
    rxvml_value *arguments[2];
    rxinteger handler_tag = -1;
    int status;
    memset(&probe, 0, sizeof(probe));
    probe.mutate = mutate;
    probe.context = rxvml_create(NULL, 0);
    check(probe.context != NULL, "could not create delivery context");
    if (!probe.context) return;
    if (rxvml_load_module_file(probe.context, image) <= 0) {
        check(0, "could not load delivery image");
        goto cleanup;
    }
    target = rxvml_value_new(probe.context);
    payload = rxvml_object_new(probe.context, 2);
    if (!target || !payload) {
        check(0, "could not allocate owned delivery values");
        goto cleanup;
    }
    if (rxvml_set_native_payload(payload, &payload_probe, sizeof(payload_probe),
                                &payload_ops, 0)) {
        check(0, "could not attach delivery payload");
        goto cleanup;
    }
    arguments[0] = target; arguments[1] = payload;
    status = rxvml_call_procedure_descriptor(probe.context,
        "rxsig1|signal_delivery.trigger|.int|target=.object,payload=.object",
        2, arguments, &reply);
    check(status == 0, "event delivery or nested callback did not return normally");
    if (reply) check(rxvml_to_int(probe.context, reply, &handler_tag) == 0,
                     "handler did not return its scalar observation");
    check(handler_tag == 1, "handler did not receive an initialized runtime_signal shell");
    check(probe.hooks == 5 && probe.ready_copies == 4,
          "event publication changed the real payload COPY count");
    check(probe.nested_calls == (mutate ? 3 : 0) && probe.errors == 0,
          "delivery callback lost reference identity, effects or nested-call cleanup");
    if (status || handler_tag != 1 || probe.hooks != 5 || probe.ready_copies != 4 ||
        probe.nested_calls != (mutate ? 3 : 0) || probe.errors) {
        fprintf(stderr, "delivery mode=%d status=%d handler=%lld hooks=%d ready=%d nested=%d errors=%d\n",
                mutate, status, (long long)handler_tag, probe.hooks,
                probe.ready_copies, probe.nested_calls, probe.errors);
    }
cleanup:
    if (reply) rxvml_value_free(reply);
    if (payload) rxvml_value_free(payload);
    if (target) rxvml_value_free(target);
    rxvml_destroy(probe.context);
}

int main(int argc, char **argv) {
    int image;
    if (argc != 3) return 2;
    for (image = 1; image < argc; ++image) {
        run_case(argv[image], 0);
        run_case(argv[image], 1);
    }
    printf("SIGNAL_DELIVERY|failures=%d\n", failures);
    return failures ? 1 : 0;
}
