/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#ifndef CREXX_RXCONTRACT_H
#define CREXX_RXCONTRACT_H
#include <stddef.h>

/* Owned current-format metadata records. No compiler AST, source grammar or
 * wire enum is exposed here. See compiler/docs/metadata_conformance.md. */
typedef struct rx_contract_range {
    char *lower;                 /* NULL means the default lower bound. */
    char *upper;                 /* NULL means the open upper bound. */
} rx_contract_range;
typedef struct rx_contract_type {
    char *name;                  /* Source-qualified spelling, e.g. .ns..item. */
    struct rx_contract_type *referent;
    rx_contract_range *ranges;
    size_t dimensions;
} rx_contract_type;
typedef enum rx_contract_callable_kind {
    RX_CONTRACT_PROCEDURE, RX_CONTRACT_METHOD, RX_CONTRACT_FACTORY
} rx_contract_callable_kind;
typedef enum rx_contract_default_kind {
    RX_CONTRACT_NO_DEFAULT, RX_CONTRACT_INTEGER_DEFAULT,
    RX_CONTRACT_FLOAT_DEFAULT, RX_CONTRACT_STRING_DEFAULT
} rx_contract_default_kind;
typedef struct rx_contract_arg {
    char *name;
    rx_contract_type type;
    unsigned char by_reference, optional, vararg;
    rx_contract_default_kind default_kind;
    char *default_literal;       /* Historical authored RXAS; never executed here. */
} rx_contract_arg;
typedef struct rx_contract_callable {
    char *name;
    rx_contract_callable_kind kind;
    rx_contract_type result;
    rx_contract_arg *args;
    size_t arg_count;
    unsigned char task;
} rx_contract_callable;
typedef struct rx_contract_attribute {
    char *name;
    rx_contract_type type;
    int register_index;         /* -1 means no explicit mapping. */
    char *view;
} rx_contract_attribute;
typedef struct rx_contract_class_header {
    char *namespace_name, *name;
    unsigned char is_interface;
} rx_contract_class_header;
typedef struct rx_contract_error {
    const char *field, *message; /* Static strings, not caller-owned. */
    size_t offset;              /* Byte offset in the failing field, when known. */
} rx_contract_error;

/* Outputs are initialized even on failure; free them after success or failure.
 * Initialize a fresh output, or free its previous contents, before reusing it.
 * Types accept exact byte spans and reject embedded NUL. Other fields follow
 * the existing NUL-terminated RXAS/RXBIN/RXPA field interface. */
int rx_contract_parse_type(const char *, size_t, rx_contract_type *, rx_contract_error *);
int rx_contract_parse_callable(rx_contract_callable_kind, const char *name,
    const char *result, const char *args, int task, rx_contract_callable *, rx_contract_error *);
int rx_contract_parse_attribute(const char *name, const char *type, int reg,
    const char *view, rx_contract_attribute *, rx_contract_error *);
int rx_contract_parse_class_header(const char *ns, const char *name, int is_interface,
    rx_contract_class_header *, rx_contract_error *);
void rx_contract_type_free(rx_contract_type *);
void rx_contract_callable_free(rx_contract_callable *);
void rx_contract_attribute_free(rx_contract_attribute *);
void rx_contract_class_header_free(rx_contract_class_header *);
#endif
