/* cREXX License (MIT). Internal C boundary; no C++ exception crosses RXPA. */
#ifndef CREXX_LLAMA_BRIDGE_H
#define CREXX_LLAMA_BRIDGE_H
#include <stdint.h>
#include <stddef.h>
#ifdef _WIN32
# ifdef CREXX_LLAMA_BUILD
#  define LLAMA_BRIDGE_API __declspec(dllexport)
# else
#  define LLAMA_BRIDGE_API __declspec(dllimport)
# endif
#else
# define LLAMA_BRIDGE_API __attribute__((visibility("default")))
#endif
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { uint64_t owner, id; } rxllama_token;
typedef struct { rxllama_token handle; int64_t integer; double number; const char *text; size_t text_length; } rxllama_argument;
/* Private, opt-in diagnostic counters; never a VM value or cross-worker handle. */
typedef struct {
    uint64_t admit_ns, submit_ns, setup_ns, decode_ns, normalize_ns, copy_ns;
    uint64_t copies, bytes, requests;
} rxllama_glue_probes;
typedef struct {
    rxllama_token handle; int64_t integer; const char *text, *operation, *message;
    const double *values; int64_t value_count, rows, dimensions;
    /* Borrow only for the immediate packed-copy operation in this native call. */
    rxllama_glue_probes *probes;
    size_t text_length;
    const char *finish;
    int64_t row;
    /* Internal oracle view of newly sampled token IDs, valid only this call. */
    const int32_t *token_ids;
} rxllama_result;
enum rxllama_operation {
    RXLLAMA_CONFIG_CREATE, RXLLAMA_CONFIG_INT, RXLLAMA_CONFIG_FLOAT,
    RXLLAMA_CONFIG_TEXT, RXLLAMA_RUNTIME_OPEN, RXLLAMA_DEVICE_COUNT,
    RXLLAMA_DEVICE_INFO, RXLLAMA_MODEL_OPEN, RXLLAMA_MODEL_STATE,
    RXLLAMA_MODEL_CANCEL, RXLLAMA_SESSION_OPEN, RXLLAMA_PREPARE,
    RXLLAMA_INFO_TEXT, RXLLAMA_INFO_INT, RXLLAMA_DIAGNOSTIC, RXLLAMA_CLOSE,
    RXLLAMA_REQUEST_OPEN, RXLLAMA_ADD_EMBEDDING, RXLLAMA_SUBMIT,
    RXLLAMA_PROCESS, RXLLAMA_EMBEDDINGS, RXLLAMA_CANCEL,
    RXLLAMA_ADD_PROMPT, RXLLAMA_READ_TEXT
};
LLAMA_BRIDGE_API uint64_t rxllama_probe_clock_ns(void);
LLAMA_BRIDGE_API void *rxllama_vm_create(void);
LLAMA_BRIDGE_API void rxllama_vm_destroy(void *vm);
LLAMA_BRIDGE_API int rxllama_call(void *vm, int operation, const rxllama_argument *args, rxllama_result *result);
/* Only adjust references. Deferred destruction runs on procedure/session exit. */
LLAMA_BRIDGE_API int rxllama_retain(rxllama_token token);
LLAMA_BRIDGE_API void rxllama_release(rxllama_token token);
#ifdef __cplusplus
}
#endif
#endif
