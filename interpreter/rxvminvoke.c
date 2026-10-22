/* cREXX License (MIT). Checked, owned ordinary bytecode/RXPA invocation.
 * DECL_ONLY exports the same declarations used by runtime registration. */
#ifndef DECL_ONLY
#define rxpa_getnativepayload rxvm_getnativepayload
#define rxpa_setnativepayload rxvm_setnativepayload
#define rxpa_isinitialized rxvm_isinitialized
#define rxpa_setint rxvm_setint
#define rxpa_setstring rxvm_setstring
#define rxpa_addfunc_for_plugin rxvm_addfunc_for_plugin
#define rxpa_addclass rxvm_addclass
#define rxpa_addmember rxvm_addmember
#define rxpa_register_static_plugin_manifest_v2 rxvm_register_static_plugin_manifest_v2
#endif
#ifndef PLUGIN_ID
#define PLUGIN_ID rxinvoke
#endif
#include "../rxpa/crexxpa.h"
#include "rxvminvoke.h"

#ifdef DECL_ONLY
#define INV_CLASS(owner) { RXVM_INVOKE_BIND_CLASS, owner, 0, 0, ".unknown", 0 },
#define INV_FACTORY(fn, owner, type, args) \
    { RXVM_INVOKE_BIND_MEMBER, owner, "factory", "*", type, args }, \
    { RXVM_INVOKE_BIND_FUNCTION, owner ".\xc2\xa7" "factory", 0, 0, type, args },
#define INV_METHOD(fn, owner, member, type, args) \
    { RXVM_INVOKE_BIND_MEMBER, owner, "method", member, type, args }, \
    { RXVM_INVOKE_BIND_FUNCTION, owner "." member, 0, 0, type, args },
#define INV_FUNCTION(fn, name, type, args) \
    { RXVM_INVOKE_BIND_FUNCTION, name, 0, 0, type, args },
const rxvm_invoke_binding rxvm_invoke_bindings[] = {
#else
#include "rxvmintp.h"
#include "rxvmvars.h"
#include "rxvmlinkindex.h"
#include "rxcontract.h"
#include <stdlib.h>
#include <string.h>
#if defined(CREXX_VM_SINGLE_THREADED)
#define INV_LOCAL
#elif defined(_WIN32)
#include <windows.h>
#if defined(_MSC_VER)
#define INV_LOCAL __declspec(thread)
#else
#define INV_LOCAL __thread
#endif
#else
#include <pthread.h>
#define INV_LOCAL __thread
#endif

typedef enum scalar_kind {
    INV_ABSENT, INV_INT, INV_BOOLEAN, INV_FLOAT, INV_DECIMAL,
    INV_STRING, INV_BINARY, INV_NOMINAL, INV_VOID
} scalar_kind;
typedef enum invocation_state {
    INV_SEALED, INV_RUNNING, INV_COMPLETE, INV_FAILED, INV_DISCARDED
} invocation_state;
typedef enum resource_kind { INV_CELL, INV_CATALOGUE, INV_BINDING, INV_REQUEST } resource_kind;
typedef struct invoke_session {
    const rxpa_host_services_v1 *host;
    rxvm_context *context;
    size_t refs;
    unsigned char closed;
} invoke_session;
typedef struct invoke_resource {
    invoke_session *session;
    size_t refs;
    resource_kind kind;
} invoke_resource;
typedef struct invoke_scalar {
    invoke_resource base;
    scalar_kind kind;
    value payload;
    char *nominal;
} invoke_scalar;
typedef struct invoke_target {
    proc_runtime *procedure;
    rx_contract_callable contract;
} invoke_target;
typedef struct invoke_catalogue {
    invoke_resource base;
    uint64_t generation;
    size_t count;
    invoke_target *targets;
} invoke_catalogue;
typedef struct invoke_binding {
    invoke_resource base;
    invoke_catalogue *catalogue;
    size_t ordinal;
} invoke_binding;
struct rxvm_invocation {
    invoke_resource base;
    invoke_binding *binding;
    invocation_state state;
    size_t extent, count, completion_budget;
    int64_t origin_module, origin_address;
    unsigned char *presence;
    value *cells;
    value **arguments;
    value result, event;
    scalar_kind result_kind;
};
static INV_LOCAL invoke_session *current;
static const char *const scalar_names[] = {
    "absent", "int", "boolean", "float", "decimal", "string", "binary", "nominal", "void"
};
static const char *const state_names[] = {"sealed", "running", "complete", "failed", "discarded"};

static void release_resource(invoke_resource *);
static void copy_resource(void *, void *);
static void finalize_resource(void *);
static const rxpa_native_payload_ops resource_ops = {"rxinvoke.owned", copy_resource, finalize_resource};
static void *old_session(void) { return NULL; }
static void *create_session(const rxpa_host_services_v1 *host) {
    invoke_session *session;
    rxvm_context *context = rxvm_active_context_current();
    if (!context || !rxpa_host_has_object_set_type(host) || !rxpa_host_has_string_view(host)) return NULL;
    session = calloc(1, sizeof(*session));
    if (session) { session->host = host; session->context = context; session->refs = 1; }
    return session;
}
static void release_session(invoke_session *session) {
    if (session && !--session->refs) free(session);
}
static void destroy_session(void *opaque) {
    invoke_session *session = opaque;
    if (session) session->closed = 1;
    release_session(session);
}
static int enter_session(void *session, uint32_t capabilities, void **previous) {
    (void)capabilities;
    *previous = current; current = session; return 0;
}
static void leave_session(void *previous) { current = previous; }
static uint32_t capabilities(const char *name) {
    return name ? RXPA_PROCEDURE_CAP_SESSION_AFFINE : 0;
}
#undef INITIALIZER
#define INITIALIZER(fn) static void fn(void) {
RXPA_PLUGIN_SESSION_WITH_HOST(old_session, destroy_session, enter_session,
                             leave_session, capabilities, create_session)
static invoke_resource *resource(void *handle) {
    size_t length;
    const rxpa_native_payload_ops *ops;
    invoke_resource *owner = NULL;
    void *data = get_native_payload(handle, &length, &ops, NULL);
    if (data && length == sizeof(owner) && ops == (const rxvm_native_payload_ops *)&resource_ops)
        memcpy(&owner, data, length);
    return owner;
}
static int valid(invoke_resource *owner, resource_kind kind) {
    return owner && owner->kind == kind && current && owner->session == current &&
           !current->closed && current->context == rxvm_active_context_current();
}
static void initialize(invoke_resource *owner, resource_kind kind) {
    owner->session = current; owner->refs = 1; owner->kind = kind; ++current->refs;
}
static void clear_window(rxvm_invocation *request) {
    size_t i;
    /* Revoke every window location before releasing any cell/payload owner. */
    for (i = 0; i < request->count; ++i) release_value_reference_lifetime(&request->cells[i]);
    for (i = 0; i < request->count; ++i) clear_value(&request->cells[i]);
    free(request->cells); free(request->arguments);
    request->cells = NULL; request->arguments = NULL; request->count = 0;
}
static void release_resource(invoke_resource *owner) {
    size_t i;
    if (!owner || --owner->refs) return;
    switch (owner->kind) {
    case INV_CELL: {
        invoke_scalar *cell = (invoke_scalar *)owner;
        clear_value(&cell->payload); free(cell->nominal); break;
    }
    case INV_CATALOGUE: {
        invoke_catalogue *catalogue = (invoke_catalogue *)owner;
        for (i = 0; i < catalogue->count; ++i) rx_contract_callable_free(&catalogue->targets[i].contract);
        free(catalogue->targets); break;
    }
    case INV_BINDING:
        release_resource(&((invoke_binding *)owner)->catalogue->base); break;
    case INV_REQUEST: {
        rxvm_invocation *request = (rxvm_invocation *)owner;
        clear_window(request); free(request->presence);
        clear_value(&request->result); clear_value(&request->event);
        release_resource(&request->binding->base); break;
    }
    }
    release_session(owner->session); free(owner);
}
static void copy_resource(void *destination, void *source) {
    invoke_resource *owner = resource(source);
    if (!owner) return;
    ++owner->refs;
    if (SETNATIVEPAYLOAD(destination, &owner, sizeof(owner), &resource_ops, 0)) release_resource(owner);
}
static void finalize_resource(void *handle) { release_resource(resource(handle)); }
static int publish(void *destination, invoke_resource *owner, const char *type) {
    if (!destination || SETNATIVEPAYLOAD(destination, &owner, sizeof(owner), &resource_ops, 0)) {
        release_resource(owner); return -1;
    }
    if (SETOBJECTTYPE(current->host, destination, type)) {
        (void)SETNATIVEPAYLOAD(destination, NULL, 0, NULL, 0); return -1;
    }
    return 0;
}
static char *exact_text(value *v) {
    const char *text;
    size_t length;
    char *result;
    if (!v || !current || current->host->string_view(v, &text, &length) ||
        length == SIZE_MAX || memchr(text, 0, length)) return NULL;
    result = malloc(length + 1);
    if (result) { memcpy(result, text, length); result[length] = 0; }
    return result;
}
static int same_type(const rx_contract_type *a, const rx_contract_type *b) {
    size_t i;
    if (!a->name || !b->name || strcmp(a->name, b->name) || a->dimensions != b->dimensions || !!a->referent != !!b->referent) return 0;
    if (a->referent && !same_type(a->referent, b->referent)) return 0;
    for (i = 0; i < a->dimensions; ++i) {
        if (!!a->ranges[i].lower != !!b->ranges[i].lower || !!a->ranges[i].upper != !!b->ranges[i].upper) return 0;
        if (a->ranges[i].lower && strcmp(a->ranges[i].lower, b->ranges[i].lower)) return 0;
        if (a->ranges[i].upper && strcmp(a->ranges[i].upper, b->ranges[i].upper)) return 0;
    }
    return 1;
}
static int same_contract(const rx_contract_callable *a, const rx_contract_callable *b) {
    size_t i;
    if (a->task || b->task || a->kind != b->kind || strcmp(a->name,b->name) ||
        !same_type(&a->result, &b->result) || a->arg_count != b->arg_count) return 0;
    for (i = 0; i < a->arg_count; ++i) {
        const rx_contract_arg *x = &a->args[i], *y = &b->args[i];
        if (strcmp(x->name,y->name) || !same_type(&x->type,&y->type) || x->by_reference != y->by_reference ||
            x->optional != y->optional || x->vararg != y->vararg || x->default_kind != y->default_kind ||
            !!x->default_literal != !!y->default_literal ||
            (x->default_literal && strcmp(x->default_literal,y->default_literal))) return 0;
    }
    return 1;
}
static scalar_kind type_kind(const rx_contract_type *type) {
    if (!type || !type->name || type->referent || type->dimensions) return INV_ABSENT;
    if (!strcmp(type->name,".int")) return INV_INT;
    if (!strcmp(type->name,".boolean")) return INV_BOOLEAN;
    if (!strcmp(type->name,".float")) return INV_FLOAT;
    if (!strcmp(type->name,".decimal")) return INV_DECIMAL;
    if (!strcmp(type->name,".string")) return INV_STRING;
    if (!strcmp(type->name,".binary")) return INV_BINARY;
    if (!strcmp(type->name,".void")) return INV_VOID;
    if (!strcmp(type->name,".object") || !strcmp(type->name,".unknown")) return INV_ABSENT;
    return INV_NOMINAL;
}
static int object_matches(value *v, const char *type) {
    const char *source;
    char *name, *dest;
    int result;
    if (!v || !v->object_type || (v->status.all_type_flags & RXFLAG_VM_OBJECT_UNINITIALIZED) ||
        !type || type[0] != '.') return 0;
    name = malloc(strlen(type) + 1);
    if (!name) return 0;
    dest = name; source = type + 1;
    while (*source) {
        *dest++ = *source++;
        if (source[-1] == '.' && *source == '.') ++source;
    }
    *dest = 0;
    result = v->object_type->name_length == (size_t)(dest - name) && !memcmp(v->object_type->name,name,(size_t)(dest-name));
    free(name); return result;
}
/* Bound the deep copy and reject reference-valued descendants before copying.
 * Native nominal payload identity remains with its registered copy owner. */
static int value_cost(value *v, size_t *remaining, unsigned depth) {
    size_t i, cost = sizeof(value);
    if (!v || depth > 128 || v->reference_payload || v->string_length > SIZE_MAX - cost) return 0;
    cost += v->string_length;
    if (v->binary_length > SIZE_MAX - cost) return 0;
    cost += v->binary_length;
    if (rxvm_value_decimal_length(v) > SIZE_MAX - cost) return 0;
    cost += rxvm_value_decimal_length(v);
    if (cost > *remaining) return 0;
    *remaining -= cost;
    for (i = 0; i < v->num_attributes; ++i) if (!value_cost(v->attributes[i],remaining,depth+1)) return 0;
    return 1;
}
static int contract_admitted(const rx_contract_callable *contract) {
    size_t i;
    if (contract->kind != RX_CONTRACT_PROCEDURE || contract->task || type_kind(&contract->result) == INV_ABSENT) return 0;
    for (i = 0; i < contract->arg_count; ++i) {
        const rx_contract_arg *arg = &contract->args[i];
        rx_contract_type scalar = arg->type;
        if (arg->vararg) {
            if (i + 1 != contract->arg_count || arg->type.dimensions) return 0;
            scalar.dimensions = 0; scalar.ranges = NULL;
        }
        if (type_kind(&scalar) == INV_ABSENT || type_kind(&scalar) == INV_VOID) return 0;
    }
    return 1;
}
PROCEDURE(make_catalogue) {
    invoke_catalogue *catalogue;
    size_t i, count;
    value *names, *results, *arguments, *providers;
    if (NUM_ARGS != 4 || !_arg || !current) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Invocation catalogue requires four vectors") }
    names=ARG0; results=ARG1; arguments=ARG2; providers=ARG3;
    if (!names || !results || !arguments || !providers ||
        names->num_attributes != results->num_attributes || names->num_attributes != arguments->num_attributes ||
        names->num_attributes != providers->num_attributes) {
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Invocation catalogue vectors differ")
    }
    count = names->num_attributes;
    if (count > SIZE_MAX / sizeof(invoke_target)) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Invocation catalogue too large") }
    catalogue = calloc(1,sizeof(*catalogue));
    if (!catalogue) { RETURNSIGNAL(SIGNAL_FAILURE,"Cannot allocate invocation catalogue") }
    initialize(&catalogue->base,INV_CATALOGUE);
    catalogue->targets = calloc(count ? count : 1,sizeof(*catalogue->targets));
    catalogue->count = count; catalogue->generation = current->context->semantic_generation;
    if (!catalogue->targets) { catalogue->count=0; release_resource(&catalogue->base); RETURNSIGNAL(SIGNAL_FAILURE,"Cannot allocate catalogue entries") }
    for (i=0; i<count; ++i) {
        char *name=exact_text(names->attributes[i]), *result=exact_text(results->attributes[i]);
        char *args=exact_text(arguments->attributes[i]), *provider=exact_text(providers->attributes[i]);
        module *mod=NULL;
        meta_func_constant *meta;
        rx_contract_callable actual={0};
        rx_contract_error error;
        const char *actual_name=NULL, *actual_result=NULL, *actual_args=NULL;
        invoke_target *target=&catalogue->targets[i];
        int accepted=0;
        if (name && result && args && provider && strncmp(name,"rxinvoke.",9) &&
            rx_contract_parse_callable(RX_CONTRACT_PROCEDURE,name,result,args,0,&target->contract,&error)) {
            target->procedure=rxvm_link_index_resolve_procedure(current->context,NULL,name,strlen(name));
            meta=target->procedure ? rxvm_link_index_find_function(current->context,NULL,target->procedure,&mod) : NULL;
            if (meta && target->procedure->start != SIZE_MAX && rxvm_invoke_provider_matches(mod,name,provider) && !rxvm_invoke_is_task(mod,name)) {
                actual_name=rxvm_invoke_metadata_string(mod,meta->symbol);
                actual_result=rxvm_invoke_metadata_string(mod,meta->type); actual_args=rxvm_invoke_metadata_string(mod,meta->args);
                if (actual_name && !strcmp(name,actual_name) && actual_result && actual_args && rx_contract_parse_callable(RX_CONTRACT_PROCEDURE,name,actual_result,actual_args,0,&actual,&error))
                    accepted=same_contract(&target->contract,&actual) && contract_admitted(&actual);
            }
        }
        rx_contract_callable_free(&actual); free(name); free(result); free(args); free(provider);
        if (!accepted) { release_resource(&catalogue->base); RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Unavailable or conflicting retained invocation target") }
    }
    if (publish(RETURN,&catalogue->base,"rxinvoke.catalogue")) { RETURNSIGNAL(SIGNAL_FAILURE,"Cannot publish invocation catalogue") }
    RESETSIGNAL
}
PROCEDURE(make_binding) {
    invoke_catalogue *catalogue;
    invoke_binding *binding;
    rxinteger ordinal;
    if (NUM_ARGS!=2 || !_arg || !ARG0 || !ARG1) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Binding requires catalogue and index") }
    catalogue=(invoke_catalogue *)resource(ARG0); ordinal=((value *)ARG1)->int_value;
    if (!valid((invoke_resource *)catalogue,INV_CATALOGUE) || ordinal<1 ||
        (uint64_t)ordinal>catalogue->count || catalogue->generation!=current->context->semantic_generation) {
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Stale, foreign or out-of-range invocation binding")
    }
    binding=calloc(1,sizeof(*binding));
    if (!binding) { RETURNSIGNAL(SIGNAL_FAILURE,"Cannot allocate invocation binding") }
    initialize(&binding->base,INV_BINDING); binding->catalogue=catalogue; ++catalogue->base.refs;
    binding->ordinal=(size_t)ordinal-1;
    if (publish(RETURN,&binding->base,"rxinvoke.binding")) { RETURNSIGNAL(SIGNAL_FAILURE,"Cannot publish invocation binding") }
    RESETSIGNAL
}
static void make_scalar(scalar_kind kind, value *payload, const char *nominal, void *destination, void *signal) {
    invoke_scalar *scalar;
    size_t budget=SIZE_MAX;
    if (!current || (payload && !value_cost(payload,&budget,0)) ||
        (kind==INV_BOOLEAN && (!payload || (payload->int_value!=0 && payload->int_value!=1))) ||
        (kind==INV_BINARY && payload && rxvm_value_native_ops(payload)) ||
        (kind==INV_NOMINAL && !object_matches(payload,nominal))) {
        SETINT(signal,SIGNAL_INVALID_ARGUMENTS); SETSTRING(signal,"Invalid invocation scalar"); return;
    }
    scalar=calloc(1,sizeof(*scalar));
    if (!scalar) { SETINT(signal,SIGNAL_FAILURE); SETSTRING(signal,"Cannot allocate invocation scalar"); return; }
    initialize(&scalar->base,INV_CELL); scalar->kind=kind;
    if (payload) {
        switch (kind) {
        case INV_INT: case INV_BOOLEAN: scalar->payload.int_value=payload->int_value; break;
        case INV_FLOAT: scalar->payload.float_value=payload->float_value; break;
        case INV_DECIMAL: {
            size_t length=rxvm_value_decimal_length(payload);
            void *data=rxvm_value_reserve_decimal(&scalar->payload,length);
            if (length && !data) { release_resource(&scalar->base); SETINT(signal,SIGNAL_FAILURE); SETSTRING(signal,"Cannot copy decimal scalar"); return; }
            if (length) memcpy(data,rxvm_value_decimal_data(payload),length);
            break;
        }
        case INV_STRING: set_string(&scalar->payload,payload->string_value ? payload->string_value : "",payload->string_length); break;
        case INV_BINARY:
            if (prep_binary_buffer(&scalar->payload,payload->binary_length)) { release_resource(&scalar->base); SETINT(signal,SIGNAL_FAILURE); SETSTRING(signal,"Cannot copy binary scalar"); return; }
            if (payload->binary_length) memcpy(scalar->payload.binary_value,payload->binary_value,payload->binary_length);
            break;
        case INV_NOMINAL: copy_value(&scalar->payload,payload); break;
        default: break;
        }
    }
    scalar->payload.status.all_type_flags &= ~RXFLAG_COMPILER_ABI_MASK;
    if (nominal) {
        scalar->nominal=malloc(strlen(nominal)+1);
        if (!scalar->nominal) { release_resource(&scalar->base); SETINT(signal,SIGNAL_FAILURE); SETSTRING(signal,"Cannot retain scalar type"); return; }
        strcpy(scalar->nominal,nominal);
    }
    if (publish(destination,&scalar->base,"rxinvoke.scalar")) { SETINT(signal,SIGNAL_FAILURE); SETSTRING(signal,"Cannot publish invocation scalar"); return; }
    SETINT(signal,SIGNAL_NONE); SETSTRING(signal,"");
}
PROCEDURE(make_absent) { (void)_arg; if (NUM_ARGS) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Absent capsule takes no value") } make_scalar(INV_ABSENT,NULL,NULL,RETURN,SIGNAL); }
#define SCALAR_FACTORY(fn,kind) PROCEDURE(fn) { \
    if (NUM_ARGS!=1 || !_arg || !ARG0) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Scalar factory requires one value") } \
    make_scalar(kind,ARG0,NULL,RETURN,SIGNAL); }
SCALAR_FACTORY(make_integer,INV_INT)
SCALAR_FACTORY(make_boolean,INV_BOOLEAN)
SCALAR_FACTORY(make_float,INV_FLOAT)
SCALAR_FACTORY(make_decimal,INV_DECIMAL)
SCALAR_FACTORY(make_text,INV_STRING)
SCALAR_FACTORY(make_binary,INV_BINARY)
PROCEDURE(make_nominal) {
    char *name;
    if (NUM_ARGS!=2 || !_arg || !ARG0 || !ARG1 || !(name=exact_text(ARG1))) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Nominal scalar requires type name") }
    make_scalar(INV_NOMINAL,ARG0,name,RETURN,SIGNAL); free(name);
}
#define REQUIRE_CELL \
    invoke_scalar *scalar=(invoke_scalar *)resource(ARG0); \
    if (!valid((invoke_resource *)scalar,INV_CELL) || NUM_ARGS!=1) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Foreign or forged invocation scalar") }
METHODPROCEDURE(scalar_tag) { REQUIRE_CELL SETSTRING(RETURN,scalar_names[scalar->kind]); RESETSIGNAL }
#define SCALAR_GETTER(fn,kind_) METHODPROCEDURE(fn) { REQUIRE_CELL \
    if (scalar->kind!=kind_) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Incompatible invocation scalar getter") } \
    copy_value(RETURN,&scalar->payload); RESETSIGNAL }
SCALAR_GETTER(scalar_integer,INV_INT)
SCALAR_GETTER(scalar_boolean,INV_BOOLEAN)
SCALAR_GETTER(scalar_float,INV_FLOAT)
SCALAR_GETTER(scalar_decimal,INV_DECIMAL)
SCALAR_GETTER(scalar_text,INV_STRING)
SCALAR_GETTER(scalar_binary,INV_BINARY)
SCALAR_GETTER(scalar_nominal,INV_NOMINAL)

PROCEDURE(prepare_invocation) {
    invoke_binding *binding;
    invoke_target *target;
    rxvm_invocation *request;
    value *vector, *presence;
    size_t i, fixed, count, extent, budget, storage;
    int vararg;
    if (NUM_ARGS!=5 || !_arg || !ARG0 || !ARG1 || !ARG2 || !ARG3 || !ARG4 ||
        !valid(resource(ARG0),INV_BINDING)) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Invalid invocation preparation") }
    binding=(invoke_binding *)resource(ARG0);
    if (binding->catalogue->generation!=current->context->semantic_generation ||
        ((value *)ARG3)->int_value<0 || ((value *)ARG4)->int_value<0 ||
        (uint64_t)((value *)ARG3)->int_value>SIZE_MAX || (uint64_t)((value *)ARG4)->int_value>SIZE_MAX) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Stale invocation binding or invalid budget") }
    target=&binding->catalogue->targets[binding->ordinal]; vector=ARG1; presence=ARG2;
    extent=(size_t)((value *)ARG3)->int_value; budget=(size_t)((value *)ARG4)->int_value;
    vararg=target->contract.arg_count && target->contract.args[target->contract.arg_count-1].vararg;
    fixed=target->contract.arg_count-(vararg ? 1u : 0u); count=extent>fixed ? extent : fixed;
    if (vector->num_attributes!=extent || presence->num_attributes!=extent || (!vararg && extent>fixed) ||
        count>INT_MAX || count>(SIZE_MAX-sizeof(*request))/(sizeof(value)+sizeof(value *)+1)) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Invalid invocation argument extent") }
    storage=sizeof(*request)+count*(sizeof(value)+sizeof(value *)+1);
    if (storage<sizeof(*request) || storage>budget) { RETURNSIGNAL(SIGNAL_FAILURE,"Invocation resource limit") }
    budget-=storage;
    for (i=0; i<count; ++i) {
        const rx_contract_arg *formal=&target->contract.args[i<fixed ? i : fixed];
        invoke_scalar *scalar=i<extent ? (invoke_scalar *)resource(vector->attributes[i]) : NULL;
        rxinteger present=i<extent ? presence->attributes[i]->int_value : 0;
        rx_contract_type type=formal->type;
        if (formal->vararg) { type.dimensions=0; type.ranges=NULL; }
        if ((present!=0 && present!=1) || (i<extent && !valid((invoke_resource *)scalar,INV_CELL)) ||
            (!present && i<extent && scalar->kind!=INV_ABSENT) ||
            (!present && (i>=fixed || (!formal->optional && formal->default_kind==RX_CONTRACT_NO_DEFAULT))) ||
            (present && (scalar->kind!=type_kind(&type) ||
             (scalar->kind==INV_NOMINAL && strcmp(scalar->nominal,type.name)) || !value_cost(&scalar->payload,&budget,0)))) {
            RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Invocation argument does not match retained contract")
        }
    }
    request=calloc(1,sizeof(*request));
    if (!request) { RETURNSIGNAL(SIGNAL_FAILURE,"Cannot allocate invocation") }
    initialize(&request->base,INV_REQUEST); request->binding=binding; ++binding->base.refs;
    request->state=INV_SEALED; request->extent=extent; request->count=count; request->completion_budget=budget;
    request->result_kind=type_kind(&target->contract.result);
    request->cells=calloc(count ? count : 1,sizeof(*request->cells));
    request->arguments=calloc(count ? count : 1,sizeof(*request->arguments));
    request->presence=calloc(extent ? extent : 1,1);
    if (!request->cells || !request->arguments || !request->presence) {
        /* clear_window requires a complete cells allocation. */
        if (!request->cells) request->count=0;
        release_resource(&request->base); RETURNSIGNAL(SIGNAL_FAILURE,"Cannot reserve invocation window")
    }
    for (i=0; i<count; ++i) {
        request->arguments[i]=&request->cells[i];
        if (i<extent) {
            invoke_scalar *scalar=(invoke_scalar *)resource(vector->attributes[i]);
            request->presence[i]=(unsigned char)presence->attributes[i]->int_value;
            if (request->presence[i]) {
                const rx_contract_arg *formal=&target->contract.args[i<fixed ? i : fixed];
                copy_value(&request->cells[i],&scalar->payload);
                request->cells[i].status.all_type_flags=(request->cells[i].status.all_type_flags & ~RXFLAG_COMPILER_ABI_MASK) |
                    (formal->by_reference ? 256u : 768u);
            }
        }
    }
    if (publish(RETURN,&request->base,"rxinvoke.invocation")) { RETURNSIGNAL(SIGNAL_FAILURE,"Cannot publish invocation") }
    RESETSIGNAL
}
PROCEDURE(invoke_once) {
    rxvm_invocation *request=NUM_ARGS==1 && _arg ? (rxvm_invocation *)resource(ARG0) : NULL;
    rxvm_context *context=rxvm_active_context_current();
    rxvm_invocation_boundary *boundary=context ? context->active.invocation_boundary : NULL;
    (void)RETURN;
    if (!valid((invoke_resource *)request,INV_REQUEST) || request->state!=INV_SEALED || !boundary ||
        !boundary->parent || boundary->request || boundary->owner_context!=context ||
        boundary->generation!=context->semantic_generation ||
        request->binding->catalogue->generation!=context->semantic_generation ||
        !boundary->procedure || strcmp(boundary->procedure->name,"rxinvoke.invoke")) {
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Invocation requires its own ordinary native-call boundary")
    }
    request->state=INV_RUNNING; request->origin_module=boundary->origin_module; request->origin_address=boundary->origin_address;
    ++request->base.refs; boundary->request=request;
    RESETSIGNAL
}
#define REQUIRE_REQUEST \
    rxvm_invocation *request=(rxvm_invocation *)resource(ARG0); \
    if (!valid((invoke_resource *)request,INV_REQUEST) || NUM_ARGS!=1) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Foreign or forged invocation") }
METHODPROCEDURE(invocation_state_get) { REQUIRE_REQUEST SETSTRING(RETURN,state_names[request->state]); RESETSIGNAL }
METHODPROCEDURE(invocation_extent_get) { REQUIRE_REQUEST SETINT(RETURN,(rxinteger)request->extent); RESETSIGNAL }
METHODPROCEDURE(invocation_present_get) {
    rxvm_invocation *request=(rxvm_invocation *)resource(ARG0);
    rxinteger index;
    if (!valid((invoke_resource *)request,INV_REQUEST) || NUM_ARGS!=2 || !ARG1) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Invalid invocation presence query") }
    index=((value *)ARG1)->int_value;
    if (index<1 || (uint64_t)index>request->extent) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Invocation presence index out of range") }
    SETINT(RETURN,request->presence[(size_t)index-1]); RESETSIGNAL
}
METHODPROCEDURE(invocation_kind_get) { REQUIRE_REQUEST \
    if (request->state!=INV_COMPLETE) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Invocation has no normal completion") } \
    SETSTRING(RETURN,scalar_names[request->result_kind]); RESETSIGNAL }
METHODPROCEDURE(invocation_result_get) { REQUIRE_REQUEST \
    if (request->state!=INV_COMPLETE || request->result_kind==INV_VOID) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Invocation has no value completion") } \
    make_scalar(request->result_kind,&request->result,request->result_kind==INV_NOMINAL ? \
       request->binding->catalogue->targets[request->binding->ordinal].contract.result.name : NULL,RETURN,SIGNAL); }
METHODPROCEDURE(invocation_event_get) { REQUIRE_REQUEST \
    if (request->state!=INV_FAILED) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Invocation has no failure event") } \
    copy_value(RETURN,&request->event); RESETSIGNAL }
METHODPROCEDURE(invocation_discard) { REQUIRE_REQUEST (void)RETURN; \
    if (request->state==INV_RUNNING) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"Cannot discard a running invocation") } \
    if (request->state==INV_SEALED) { request->state=INV_DISCARDED; clear_window(request); } RESETSIGNAL }
proc_runtime *rxvm_invoke_target(rxvm_invocation *request) {
    return request->binding->catalogue->targets[request->binding->ordinal].procedure;
}
size_t rxvm_invoke_count(rxvm_invocation *request) { return request->count; }
value **rxvm_invoke_arguments(rxvm_invocation *request) { return request->arguments; }
value *rxvm_invoke_return(rxvm_invocation *request) { return &request->result; }
void rxvm_invoke_origin(rxvm_invocation *request, int64_t *module, int64_t *address) {
    *module=request->origin_module; *address=request->origin_address;
}
void rxvm_invoke_complete(rxvm_invocation *request, int returned_value) {
    const rx_contract_type *type;
    size_t budget;
    value payload={0};
    if (!request || request->state!=INV_RUNNING) return;
    type=&request->binding->catalogue->targets[request->binding->ordinal].contract.result;
    budget=request->completion_budget;
    if ((request->result_kind!=INV_VOID && !returned_value) ||
        (request->result_kind==INV_BOOLEAN && request->result.int_value!=0 && request->result.int_value!=1) ||
        (request->result_kind==INV_NOMINAL && !object_matches(&request->result,type->name)) || (request->result_kind==INV_BINARY && rxvm_value_native_ops(&request->result)) ||
        !value_cost(&request->result,&budget,0)) {
        request->state=INV_FAILED;
        set_null_string(&payload,"Target returned an incompatible or oversized invocation value");
        rxvm_invoke_failure_event(&request->event,SIGNAL_INVALID_ARGUMENTS,request->origin_module,request->origin_address,&payload);
        clear_value(&payload);
        clear_value(&request->result);
    } else {
        request->state=INV_COMPLETE;
        if (request->result_kind==INV_VOID) clear_value(&request->result);
    }
}
void rxvm_invoke_fail(rxvm_invocation *request, const value *event) {
    if (!request || request->state!=INV_RUNNING) return;
    if (event) copy_value(&request->event,(value *)event);
    request->state=INV_FAILED;
    clear_value(&request->result);
}
void rxvm_invoke_abandon(rxvm_invocation *request) {
    if (!request) return;
    if (request->state==INV_RUNNING) request->state=INV_DISCARDED;
    clear_window(request);
}
void rxvm_invoke_unpin(rxvm_invocation *request) { if (request) release_resource(&request->base); }
#define INV_CLASS(owner) ADDCLASS(owner);
#define INV_FACTORY(fn,owner,type,args) ADDFACTORYPROC(fn,owner,type,args);
#define INV_METHOD(fn,owner,member,type,args) ADDMETHODPROC(fn,owner,member,type,args);
#define INV_FUNCTION(fn,name,type,args) ADDPROC(fn,name,"b",type,args);
static void register_invocation(void) {
    UNIQUE_CAPABILITY_FUNCTION_NAME(PLUGIN_ID)();
#endif
    INV_CLASS("rxinvoke.scalar")
    INV_CLASS("rxinvoke.catalogue")
    INV_CLASS("rxinvoke.binding")
    INV_CLASS("rxinvoke.invocation")
    INV_FACTORY(make_catalogue,"rxinvoke.catalogue",".rxinvoke..catalogue","names=.string[],results=.string[],args=.string[],providers=.string[]")
    INV_FACTORY(make_binding,"rxinvoke.binding",".rxinvoke..binding","catalogue=.rxinvoke..catalogue,index=.int")
    INV_FUNCTION(make_absent,"rxinvoke.absent",".rxinvoke..scalar","")
    INV_FUNCTION(make_integer,"rxinvoke.integer",".rxinvoke..scalar","value=.int")
    INV_FUNCTION(make_boolean,"rxinvoke.boolean",".rxinvoke..scalar","value=.boolean")
    INV_FUNCTION(make_float,"rxinvoke.floating",".rxinvoke..scalar","value=.float")
    INV_FUNCTION(make_decimal,"rxinvoke.decimal",".rxinvoke..scalar","value=.decimal")
    INV_FUNCTION(make_text,"rxinvoke.text",".rxinvoke..scalar","value=.string")
    INV_FUNCTION(make_binary,"rxinvoke.binary",".rxinvoke..scalar","value=.binary")
    INV_FUNCTION(make_nominal,"rxinvoke.nominal",".rxinvoke..scalar","value=.object,typename=.string")
    INV_METHOD(scalar_tag,"rxinvoke.scalar","kind",".string","")
    INV_METHOD(scalar_integer,"rxinvoke.scalar","asint",".int","")
    INV_METHOD(scalar_boolean,"rxinvoke.scalar","asboolean",".boolean","")
    INV_METHOD(scalar_float,"rxinvoke.scalar","asfloat",".float","")
    INV_METHOD(scalar_decimal,"rxinvoke.scalar","asdecimal",".decimal","")
    INV_METHOD(scalar_text,"rxinvoke.scalar","astext",".string","")
    INV_METHOD(scalar_binary,"rxinvoke.scalar","asbinary",".binary","")
    INV_METHOD(scalar_nominal,"rxinvoke.scalar","asnominal",".object","")
    INV_FUNCTION(prepare_invocation,"rxinvoke.prepare",".rxinvoke..invocation","binding=.rxinvoke..binding,cells=.rxinvoke..scalar[],presence=.int[],extent=.int,bytebudget=.int")
    INV_FUNCTION(invoke_once,"rxinvoke.invoke",".void","invocation=.rxinvoke..invocation")
    INV_METHOD(invocation_state_get,"rxinvoke.invocation","state",".string","")
    INV_METHOD(invocation_extent_get,"rxinvoke.invocation","extent",".int","")
    INV_METHOD(invocation_present_get,"rxinvoke.invocation","present",".boolean","index=.int")
    INV_METHOD(invocation_kind_get,"rxinvoke.invocation","kind",".string","")
    INV_METHOD(invocation_result_get,"rxinvoke.invocation","result",".rxinvoke..scalar","")
    INV_METHOD(invocation_event_get,"rxinvoke.invocation","event",".object","")
    INV_METHOD(invocation_discard,"rxinvoke.invocation","discard",".void","")
#ifdef DECL_ONLY
};
const size_t rxvm_invoke_binding_count=sizeof(rxvm_invoke_bindings)/sizeof(rxvm_invoke_bindings[0]);
#else
}
#if defined(CREXX_VM_SINGLE_THREADED)
void rxvm_invoke_init(void) { static int initialized; if (!initialized) { register_invocation(); initialized=1; } }
#elif defined(_WIN32)
static INIT_ONCE invocation_once=INIT_ONCE_STATIC_INIT;
static BOOL CALLBACK initialize_invocation(PINIT_ONCE once,PVOID parameter,PVOID *context) {
    (void)once; (void)parameter; (void)context; register_invocation(); return TRUE;
}
void rxvm_invoke_init(void) { (void)InitOnceExecuteOnce(&invocation_once,initialize_invocation,NULL,NULL); }
#else
static pthread_once_t invocation_once=PTHREAD_ONCE_INIT;
void rxvm_invoke_init(void) { (void)pthread_once(&invocation_once,register_invocation); }
#endif
#endif
