/* cREXX License (MIT). Compiler-host-only projection of binutils contracts.
 * No signature grammar or source-language admission policy lives here. */
#include "crexxpa.h"
#if defined(CREXX_VM_SINGLE_THREADED)
#undef INITIALIZER
#define INITIALIZER(f) static void f(void) {
#endif
#include "rxcontract.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(CREXX_VM_SINGLE_THREADED)
#define CONTRACT_LOCAL
#elif defined(_MSC_VER)
#define CONTRACT_LOCAL __declspec(thread)
#else
#define CONTRACT_LOCAL __thread
#endif
typedef struct contract_session { const rxpa_host_services_v1 *host; } contract_session;
static CONTRACT_LOCAL contract_session *current;
static void *old_session(void) { return NULL; }
static void *create_session(const rxpa_host_services_v1 *host) {
    contract_session *session;
    if (!rxpa_host_has_string_view(host) || !rxpa_host_has_string_set(host)) return NULL;
    session = malloc(sizeof(*session));
    if (session) session->host = host;
    return session;
}
static void destroy_session(void *session) { free(session); }
static int enter_session(void *session, uint32_t capabilities, void **previous) {
    (void)capabilities; *previous = current; current = session; return 0;
}
static void leave_session(void *previous) { current = previous; }
static uint32_t capabilities(const char *name) {
    return name ? RXPA_PROCEDURE_CAP_SESSION_AFFINE : 0u;
}
RXPA_PLUGIN_SESSION_WITH_HOST(old_session, destroy_session, enter_session,
        leave_session, capabilities, create_session)

typedef struct allowance { size_t bytes, work, used_bytes, used_work; } allowance;
static int reserve(size_t count, size_t limit, size_t *used) {
    if (count > limit - *used) return 0;
    *used += count; return 1;
}
static char *owned(const char *text, size_t length) {
    char *copy;
    if (length == SIZE_MAX) return NULL;
    copy = malloc(length + 1u);
    if (copy) { memcpy(copy, text, length); copy[length] = 0; }
    return copy;
}
static char *number(size_t value) {
    char text[3u * sizeof(size_t) + 1u];
    int count = snprintf(text, sizeof(text), "%zu", value);
    return count < 0 ? NULL : owned(text, (size_t)count);
}
static int view(rxpa_attribute_value value, const char **text, size_t *length) {
    return current && !current->host->string_view(value, text, length) &&
        (!*length || !memchr(*text, 0, *length));
}
static int add_length(size_t *length, size_t count) {
    if (count > SIZE_MAX - *length) return 0;
    *length += count; return 1;
}
static int type_length(const rx_contract_type *type, size_t *length) {
    size_t index;
    if (type->referent)
        return add_length(length, 10u) && type_length(type->referent, length);
    if (!type->name || !add_length(length, strlen(type->name))) return 0;
    if (!type->dimensions) return 1;
    if (!add_length(length, 2u)) return 0;
    for (index = 0u; index < type->dimensions; ++index) {
        const rx_contract_range *range = &type->ranges[index];
        if (index && !add_length(length, 1u)) return 0;
        if (range->lower &&
            (!add_length(length, strlen(range->lower)) || !add_length(length, 4u))) return 0;
        if (!add_length(length, range->upper ? strlen(range->upper) : 1u)) return 0;
    }
    return 1;
}
static char *put(char *out, const char *text) {
    size_t length = strlen(text); memcpy(out, text, length); return out + length;
}
static char *type_write(char *out, const rx_contract_type *type) {
    size_t index;
    if (type->referent) return type_write(put(out, "reference "), type->referent);
    out = put(out, type->name);
    if (!type->dimensions) return out;
    *out++ = '[';
    for (index = 0u; index < type->dimensions; ++index) {
        const rx_contract_range *range = &type->ranges[index];
        if (index) *out++ = ',';
        if (range->lower) { out = put(out, range->lower); out = put(out, " to "); }
        out = put(out, range->upper ? range->upper : "*");
    }
    *out++ = ']'; return out;
}
static char *type_text(const rx_contract_type *type, allowance *budget, int *status) {
    size_t length = 0u;
    char *text;
    if (!type_length(type, &length) ||
        !reserve(length, budget->bytes, &budget->used_bytes) ||
        !reserve(length, budget->work, &budget->used_work)) { *status = 2; return NULL; }
    text = malloc(length + 1u);
    if (!text) { *status = 3; return NULL; }
    *type_write(text, type) = 0; return text;
}
static char *field_text(const char *text, allowance *budget, int *status) {
    size_t length = strlen(text ? text : "");
    char *copy;
    if (!reserve(length, budget->bytes, &budget->used_bytes) ||
        !reserve(length, budget->work, &budget->used_work)) { *status = 2; return NULL; }
    copy = owned(text ? text : "", length);
    if (!copy) *status = 3;
    return copy;
}
static void free_fields(char **fields, size_t count) {
    size_t index;
    if (!fields) return;
    for (index = 0u; index < count; ++index) free(fields[index]);
    free(fields);
}
static int publish(rxpa_attribute_value result, const char *const *fields, size_t count) {
    size_t index;
    SETNUMATTRS(result, (rxinteger)count);
    for (index = 0u; index < count; ++index)
        if (SETSTRINGLENGTH(current->host, GETATTR(result, (rxinteger)index),
                fields[index] ? fields[index] : "", strlen(fields[index] ? fields[index] : ""))) {
            SETNUMATTRS(result, 0); return 0;
        }
    return 1;
}
static const char *status_name(int status) {
    static const char *const names[] = {"ok", "invalid", "limit", "allocation", "unavailable"};
    return status >= 0 && status < 5 ? names[status] : "invalid";
}
static int failure(rxpa_attribute_value result, const char *version, int status,
        const char *field, const char *message, size_t offset, const allowance *budget) {
    char byte_text[32], work_text[32], offset_text[32];
    const char *fields[13] = {0};
    snprintf(byte_text, sizeof(byte_text), "%zu", budget->used_bytes);
    snprintf(work_text, sizeof(work_text), "%zu", budget->used_work);
    snprintf(offset_text, sizeof(offset_text), "%zu", offset);
    fields[0] = version; fields[1] = status_name(status);
    if (strcmp(version, "rxnative1") == 0) {
        fields[2] = message; fields[3] = byte_text; fields[4] = work_text;
    } else {
        fields[2] = field; fields[3] = message; fields[4] = offset_text;
        fields[5] = byte_text; fields[6] = work_text; fields[8] = "0";
        fields[9] = "0"; fields[12] = "0";
    }
    return publish(result, fields, 13u);
}

PROCEDURE(project) {
    const char *inputs[4]; size_t lengths[4], index, count = 0u;
    char *copies[4] = {0}, **fields = NULL;
    rx_contract_callable contract;
    rx_contract_error error = {0};
    allowance budget = {0};
    int status = 0, kind, task;
    const char *problem = "Invalid contract projection arguments", *field = "input";
    if (NUM_ARGS != 8 || !current) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Contract projection requires eight arguments") }
    RESETSIGNAL
    memset(&contract, 0, sizeof(contract));
    if (GETINT(ARG6) < 0 || GETINT(ARG7) < 0 ||
        (uintmax_t)GETINT(ARG6) > SIZE_MAX || (uintmax_t)GETINT(ARG7) > SIZE_MAX) { status = 2; goto failed; }
    budget.bytes = (size_t)GETINT(ARG6); budget.work = (size_t)GETINT(ARG7);
    if (GETINT(ARG4) < 0 || GETINT(ARG4) > 2 ||
        (GETINT(ARG5) != 0 && GETINT(ARG5) != 1)) { status = 1; goto failed; }
    kind = (int)GETINT(ARG4); task = (int)GETINT(ARG5);
    for (index = 0u; index < 4u; ++index) {
        if (!view(_arg[index], &inputs[index], &lengths[index])) { status = 1; goto failed; }
        if (!reserve(lengths[index], budget.bytes, &budget.used_bytes) ||
            !reserve(lengths[index], budget.work, &budget.used_work)) {
            status = 2; problem = "Contract input budget"; goto failed;
        }
    }
    /* Reserve the decoder's repeated text passes before entering it. This is a
     * logical scan/payload allowance, not a CPU/RSS bound for trusted native code. */
    if (budget.used_work > (budget.work - budget.used_work) / 7u) {
        status = 2; problem = "Contract decode work budget"; goto failed;
    }
    budget.used_work *= 8u;
    for (index = 0u; index < 4u; ++index) {
        copies[index] = owned(inputs[index], lengths[index]);
        if (!copies[index]) { status = 3; problem = "Contract input allocation"; goto failed; }
    }
    if (!rx_contract_parse_callable((rx_contract_callable_kind)kind, copies[0], copies[1],
            copies[2], task, &contract, &error)) {
        status = 1; field = error.field; problem = error.message; goto failed;
    }
    if (contract.arg_count > (SIZE_MAX - 13u) / 7u) { status = 2; goto failed; }
    count = 13u + contract.arg_count * 7u;
    if (count > budget.work - budget.used_work) { status = 2; problem = "Contract field budget"; goto failed; }
    budget.used_work += count;
    fields = calloc(count, sizeof(*fields));
    if (!fields) { status = 3; goto failed; }
    fields[0] = owned("rxcontract1", 11u); fields[1] = owned("ok", 2u);
    fields[2] = owned("", 0u); fields[3] = owned("", 0u); fields[4] = number(0u);
    fields[7] = field_text(contract.name, &budget, &status);
    fields[8] = number((size_t)kind); fields[9] = number((size_t)task);
    fields[10] = type_text(&contract.result, &budget, &status);
    fields[11] = field_text(copies[3], &budget, &status); fields[12] = number(contract.arg_count);
    for (index = 0u; index < contract.arg_count && !status; ++index) {
        const rx_contract_arg *argument = &contract.args[index];
        static const char *const defaults[] = {"none", "integer", "float", "string"};
        size_t at = 13u + index * 7u;
        fields[at] = field_text(argument->name, &budget, &status);
        fields[at + 1u] = type_text(&argument->type, &budget, &status);
        fields[at + 2u] = number(argument->by_reference);
        fields[at + 3u] = number(argument->optional);
        fields[at + 4u] = number(argument->vararg);
        fields[at + 5u] = field_text(defaults[argument->default_kind], &budget, &status);
        fields[at + 6u] = field_text(argument->default_literal, &budget, &status);
    }
    fields[5] = number(budget.used_bytes); fields[6] = number(budget.used_work);
    for (index = 0u; index < count && !status; ++index) if (!fields[index]) status = 3;
    if (!status && !publish(RETURN, (const char *const *)fields, count)) status = 3;
    if (status) { problem = status == 2 ? "Contract output budget" : "Contract output allocation"; goto failed; }
    goto done;
failed:
    if (!failure(RETURN, "rxcontract1", status ? status : 1, field, problem,
            error.offset, &budget)) { SETINT(SIGNAL, SIGNAL_FAILURE); SETSTRING(SIGNAL, "Cannot publish contract failure"); }
done:
    rx_contract_callable_free(&contract); free_fields(fields, count);
    for (index = 0u; index < 4u; ++index) free(copies[index]);
}

PROCEDURE(acquire) {
    const char *path, *callable, *problem = "Invalid native acquisition arguments";
    size_t path_length, callable_length, index;
    char *path_copy = NULL, *callable_copy = NULL;
    char kind[16], task[16], byte_text[32], work_text[32];
    const char *fields[13];
    rxpa_native_declaration_v1 declaration;
    allowance budget = {0}; int status = 1;
    if (NUM_ARGS != 4 || !current) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Native acquisition requires four arguments") }
    RESETSIGNAL
    if (GETINT(ARG2) < 0 || GETINT(ARG3) < 0 ||
        (uintmax_t)GETINT(ARG2) > SIZE_MAX || (uintmax_t)GETINT(ARG3) > SIZE_MAX) { status = 2; goto failed; }
    budget.bytes = (size_t)GETINT(ARG2); budget.work = (size_t)GETINT(ARG3);
    if (!view(ARG0, &path, &path_length) || !view(ARG1, &callable, &callable_length)) goto failed;
    if (path_length > budget.bytes || callable_length > budget.bytes - path_length ||
        path_length > budget.work || callable_length > budget.work - path_length) {
        status = 2; problem = "Native acquisition input budget"; goto failed;
    }
    if (!rxpa_host_has_native_declaration(current->host)) {
        status = 4; problem = "Host has no native declaration service"; goto failed;
    }
    path_copy = owned(path, path_length); callable_copy = owned(callable, callable_length);
    if (!path_copy || !callable_copy) { status = 3; problem = "Native acquisition input allocation"; goto failed; }
    memset(&declaration, 0, sizeof(declaration)); declaration.struct_size = sizeof(declaration);
    status = current->host->native_declaration(path_copy, callable_copy, budget.bytes, budget.work,
            &declaration, &problem, &budget.used_bytes, &budget.used_work);
    if (status) goto failed;
    snprintf(kind, sizeof(kind), "%u", declaration.kind); snprintf(task, sizeof(task), "%u", declaration.task);
    snprintf(byte_text, sizeof(byte_text), "%zu", budget.used_bytes);
    snprintf(work_text, sizeof(work_text), "%zu", budget.used_work);
    fields[0] = "rxnative1"; fields[1] = "ok"; fields[2] = "";
    fields[3] = byte_text; fields[4] = work_text; fields[5] = declaration.provider_id;
    fields[6] = declaration.name; fields[7] = kind; fields[8] = declaration.owner;
    fields[9] = declaration.result; fields[10] = declaration.arguments;
    fields[11] = declaration.flags; fields[12] = task;
    for (index = 0u; index < 13u; ++index) if (!fields[index]) { status = 1; goto failed; }
    if (!publish(RETURN, fields, 13u)) { status = 3; problem = "Native acquisition output allocation"; goto failed; }
    goto done;
failed:
    if (!failure(RETURN, "rxnative1", status, "", problem, 0u, &budget)) {
        SETINT(SIGNAL, SIGNAL_FAILURE); SETSTRING(SIGNAL, "Cannot publish native acquisition failure");
    }
done:
    free(path_copy); free(callable_copy);
}

LOADFUNCS
    ADDPROC(project, "rxcontract.project", "b", ".string[]",
        "name=.string,result=.string,args=.string,flags=.string,kind=.int,task=.int,bytes=.int,work=.int");
    ADDPROC(acquire, "rxcontract.acquire", "b", ".string[]",
        "artifact=.string,callable=.string,bytes=.int,work=.int");
ENDLOADFUNCS

#if defined(CREXX_VM_SINGLE_THREADED)
void crexx_contract_init(void) {
    static int initialized;
    if (initialized) return;
    initialized = 1;
    UNIQUE_CAPABILITY_FUNCTION_NAME(PLUGIN_ID)();
    UNIQUE_INIT_FUNCTION_NAME(PLUGIN_ID)();
}
#endif
