/*
 * rxcp_opt.c
 * cREXX Optimisations
 */

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include "platform.h"
#include "rxcpmain.h"
#include "rxcpbgmr.h"

/* Optimiser payload */
typedef struct Payload {
    Context *context;
    Scope *current_scope;
    char changed;
} Payload;

/* Update the string representation if a CONSTANT node */
static void update_string(ASTNode* node) {
    char* buffer;
    size_t length;
    if (node->node_type == ERROR) return; /* Don't over write error codes */
    switch (node->value_type) {
        case TP_INTEGER:
        case TP_BOOLEAN:
            buffer = malloc(32); /* Large enough for any int */
#ifdef __32BIT__
            length = snprintf(buffer,32,"%ld",node->int_value);
#else
            length = snprintf(buffer,32,"%lld",node->int_value);
#endif
            /* Update the node's string - this also takes ownership of the memory */
            ast_sstr(node, buffer, length);
            return;

        case TP_FLOAT:
            buffer = malloc(32); /* Large enough for any float */
#ifdef __CMS__
            length = snprintf(buffer,32,"%.14g",node->float_value);
#else
            length = snprintf(buffer,32,"%.15g",node->float_value);
#endif
            /* Update the node's string - this also takes ownership of the memory */
            ast_sstr(node, buffer, length);
            return;

        default:
            /* Do nothing */
            return;
    }
}

/* Convert a string to an integer - returns 1 on error */
static int string2integer(rxinteger *out, char *string, size_t length) {
    char *buffer = malloc(length + 1);
    char *end = buffer;
    int rc = 0;
    errno = 0;

    /* Null terminated buffer */
    buffer[length] = 0;
    memcpy(buffer, string, length);

    /* Convert */
    #ifdef __32BIT__
    rxinteger l = strtol(buffer, &end, 10);
    #else
    rxinteger l = strtoll(buffer, &end, 10);
    #endif

    /* Convert error */
    if (errno == ERANGE || end == buffer) {
        rc = 1;
        goto end_string2integer;
    }

    /* Check only trailing spaces */
    while (*end != 0) {
        if (!isspace(*end)) {
            rc = 1;
            goto end_string2integer;
        }
        end++;
    }

    /* All good */
    *out = l;

    end_string2integer:
    free(buffer);
    return rc;
}

/* Convert a string to a float - returns 1 on error */
static int string2float(double *out, char *string, size_t length) {
    char *buffer = malloc(length + 1);
    char *end = buffer;
    int rc = 0;
    errno = 0;

    /* Null terminated buffer */
    buffer[length] = 0;
    memcpy(buffer, string, length);

    /* Convert */
    double l = strtod(buffer, &end);

    /* Convert error */
    if (errno == ERANGE || end == buffer) {
        rc = 1;
        goto end_string2float;
    }

    /* Check only trailing spaces */
    while (*end != 0) {
        if (!isspace(*end)) {
            rc = 1;
            goto end_string2float;
        }
        end++;
    }

    /* All good */
    *out = l;

    end_string2float:
    free(buffer);
    return rc;
}

/* Convert a CONSTANT node from a STRING to new_type */
static void string_to_type(ASTNode* node, ValueType new_type) {
    double f, floor_val;
    switch (new_type) {
        case TP_FLOAT:
            node->value_type = TP_FLOAT;
            node->target_type = TP_FLOAT;
            if (string2float(&node->float_value,node->node_string,node->node_string_length)) {
                mknd_err(node, "31.1");
            }
            return;

        case TP_BOOLEAN:
            node->value_type = TP_BOOLEAN;
            node->target_type = TP_BOOLEAN;
            /* Convert it to a float and then to 1/0 - slow but safe */
            if (string2float(&f,node->node_string,node->node_string_length)) {
                mknd_err(node, "31.1");
            }
            if (f) node->int_value = 1;
            else node->int_value = 0;
            update_string(node);
            break;

        case TP_INTEGER:
            node->value_type = new_type;
            node->target_type = new_type;
            if (string2integer(&node->int_value,node->node_string,node->node_string_length)) {
                /* Check if it is an int in float format */
                if (string2float(&f,node->node_string,node->node_string_length)) {
                    mknd_err(node, "31.1");
                }
                floor_val = floor(f);
                /* Less than an "EPSILON" above the floor? */
                if (f - floor_val < 1e-015 ) {
                    /* Yes - so and integer */
                    node->int_value = (rxinteger)floor_val;
                    return;
                }
                /* Less than an "EPSILON" below the next floor (ceiling)? */
                floor_val += 1.0;
                if (floor_val - f < 1e-015 ) {
                    /* Yes - so and integer */
                    node->int_value = (rxinteger)floor_val;
                    return;
                }
                /* Not an integer */
                mknd_err(node, "31.1");
            }
            return;

        case TP_STRING:
            node->value_type = TP_STRING;
            node->target_type = TP_STRING;
            return;

        default: ;
    }
}

/* Note the string has to be malloced and memory management is then owned by the
 * node (ie. malloc string but DONT free it after the call */
static void rewrite_to_string_constant(ASTNode* node, Payload* payload, char* string, size_t length) {
    node->child = 0;
    node->value_type = TP_STRING;
    node->node_type = CONSTANT;
    ast_sstr(node, string, length);
    string_to_type(node, node->target_type);
    payload->changed = 1;
}

static void rewrite_to_float_constant(ASTNode* node, Payload* payload, double value) {
    node->child = 0;
    node->value_type = TP_FLOAT;
    node->node_type = CONSTANT;
    node->float_value = value;
    update_string(node);
    string_to_type(node, node->target_type);
    payload->changed = 1;
}

static void rewrite_to_integer_constant(ASTNode* node, Payload* payload, rxinteger value) {
    node->child = 0;
    node->value_type = TP_INTEGER;
    node->node_type = CONSTANT;
    node->int_value = value;
    update_string(node);
    string_to_type(node, node->target_type);
    payload->changed = 1;
}

static void rewrite_to_boolean_constant(ASTNode* node, Payload* payload, int value) {
    if (value) value = 1;
    node->child = 0;
    node->value_type = TP_BOOLEAN;
    node->node_type = CONSTANT;
    node->int_value = value;
    update_string(node);
    string_to_type(node, node->target_type);
    payload->changed = 1;
}

/* Compares two nodes returns -1, 0, 1 as appropriate */
#define MIN(a,b) (((a)<(b))?(a):(b))
static int compare_nodes(ASTNode* node1, ASTNode* node2) {
    double fdiff;
    rxinteger idiff;
    if (node1->value_type == TP_FLOAT) {
        fdiff = node1->float_value - node2->float_value;
        return fdiff>0 ? 1 : (fdiff<0 ? -1 : 0);
    }

    else if (node1->value_type == TP_INTEGER) {
        idiff = node1->int_value - node2->int_value;
        return idiff>0 ? 1 : (idiff<0 ? -1 : 0);
    }

    else if (node1->value_type == TP_BOOLEAN) {
        idiff = (node1->int_value != 0) - (node2->int_value != 0); /* Weird */
        return idiff>0 ? 1 : (idiff<0 ? -1 : 0);
    }

    else { /* Use STRING */
        if ((idiff = memcmp(node1->node_string, node2->node_string,
                            MIN(node1->node_string_length,
                                node2->node_string_length)) != 0))
            return (int)idiff;
        else {
            idiff = (rxinteger)node1->node_string_length - (rxinteger)node2->node_string_length;
            return idiff>0 ? 1 : (idiff<0 ? -1 : 0);
        }
    }
}

/* Step 1
 * - Constant Folding
 */
static walker_result opt1_walker(walker_direction direction,
                                  ASTNode* node,
                                  void *pload) {

    Payload* payload = (Payload*)pload;
    ASTNode *child1, *child2, *child3, *keep_node;
    char* buffer;
    size_t buffer_length;
    int can_do_code_folding;
    int compare;

    if (direction == in) {
        /* IN - TOP DOWN */
        if (node->scope) payload->current_scope = node->scope;
    }
    else {
        /* OUT - BOTTOM UP */
        child1 = node->child;
        if (child1) child2 = child1->sibling;
        else child2 = NULL;
        if (child2) child3 = child2->sibling;
        else child3 = NULL;

        if (node->node_type == IF) {
            if (child1 && child1->node_type == CONSTANT) {
                /* Evaluate IF statement */
                if (child1->int_value) keep_node = child2; /* true -> THEN */
                else keep_node = child3; /* false -> ELSE */
                if (keep_node && keep_node->node_type != NOP) {
                    ast_rpl(node, keep_node);
                }
                else {
                    /* Just a NOP - prune the whole if node from the tree */
                    ast_del(node);
                }
                payload->changed = 1;
            }
        }

        else if (node->node_type == OP_AND) {
            /* Are both operands constants? - If so fold */
            if (child1->node_type == CONSTANT && child2->node_type == CONSTANT)
                rewrite_to_boolean_constant(node, payload,
                                            child1->int_value &&
                                            child2->int_value);

            /* If child1 is constant - either fold or prune */
            else if (child1->node_type == CONSTANT) {
                if (child1->int_value) {
                    /* The result == child2 */
                    ast_rpl(node, child2);
                    payload->changed = 1;
                }
                else {
                    /* The result must be false */
                    rewrite_to_boolean_constant(node, payload,0);
                }
            }

            /* If child2 is constant - either fold or prune */
            else if (child2->node_type == CONSTANT) {
                if (child2->int_value) {
                    /* The result == child1 */
                    ast_rpl(node, child1);
                    payload->changed = 1;
                }
                else {
                    /* The result must be false */
                    rewrite_to_boolean_constant(node, payload,0);
                }
            }
        }

        else if (node->node_type == OP_OR) {
            /* Are both operands constants? - If so fold */
            if (child1->node_type == CONSTANT && child2->node_type == CONSTANT)
                rewrite_to_boolean_constant(node, payload,
                                            child1->int_value ||
                                            child2->int_value);

            /* If child1 is constant - either fold or prune */
            else if (child1->node_type == CONSTANT) {
                if (child1->int_value) {
                    /* The result must be true */
                    rewrite_to_boolean_constant(node, payload,1);
                }
                else {
                    /* The result == child2 */
                    ast_rpl(node, child2);
                    payload->changed = 1;
                }
            }

            /* If child2 is constant - either fold or prune */
            else if (child2->node_type == CONSTANT) {
                if (child2->int_value) {
                    /* The result must be true */
                    rewrite_to_boolean_constant(node, payload,1);
                }
                else {

                    /* The result == child1 */
                    ast_rpl(node, child1);
                    payload->changed = 1;
                }
            }
        }

        else {
            /* 'Normal' Cases */

            /* If any children aren't constant then there is no constant folding to
             * be done */
            can_do_code_folding = 1;
            if (child1 && child1->node_type != CONSTANT)
                can_do_code_folding =
                        0;
            if (child2 && child2->node_type != CONSTANT)
                can_do_code_folding =
                        0;

            if (can_do_code_folding)
                switch (node->node_type) {

                    case OP_COMPARE_EQUAL:
                        compare = compare_nodes(child1, child2);
                        rewrite_to_boolean_constant(node, payload,
                                                    compare == 0);
                        break;

                    case OP_COMPARE_NEQ:
                        compare = compare_nodes(child1, child2);
                        rewrite_to_boolean_constant(node, payload,
                                                    compare != 0);
                        break;

                    case OP_COMPARE_GT:
                        compare = compare_nodes(child1, child2);
                        rewrite_to_boolean_constant(node, payload, compare > 0);
                        break;

                    case OP_COMPARE_LT:
                        compare = compare_nodes(child1, child2);
                        rewrite_to_boolean_constant(node, payload, compare < 0);
                        break;

                    case OP_COMPARE_GTE:
                        compare = compare_nodes(child1, child2);
                        rewrite_to_boolean_constant(node, payload,
                                                    compare >= 0);
                        break;

                    case OP_COMPARE_LTE:
                        compare = compare_nodes(child1, child2);
                        rewrite_to_boolean_constant(node, payload,
                                                    compare <= 0);
                        break;

                    case OP_COMPARE_S_EQ:
                        compare = compare_nodes(child1, child2);
                        rewrite_to_boolean_constant(node, payload,
                                                    compare == 0);
                        break;

                    case OP_COMPARE_S_NEQ:
                        compare = compare_nodes(child1, child2);
                        rewrite_to_boolean_constant(node, payload,
                                                    compare != 0);
                        break;

                    case OP_COMPARE_S_GT:
                        compare = compare_nodes(child1, child2);
                        rewrite_to_boolean_constant(node, payload, compare > 0);
                        break;

                    case OP_COMPARE_S_LT:
                        compare = compare_nodes(child1, child2);
                        rewrite_to_boolean_constant(node, payload, compare < 0);
                        break;

                    case OP_COMPARE_S_GTE:
                        compare = compare_nodes(child1, child2);
                        rewrite_to_boolean_constant(node, payload,
                                                    compare >= 0);
                        break;

                    case OP_COMPARE_S_LTE:
                        compare = compare_nodes(child1, child2);
                        rewrite_to_boolean_constant(node, payload,
                                                    compare <= 0);
                        break;

                    case OP_SCONCAT:
                        buffer_length =
                                child1->node_string_length +
                                child2->node_string_length + 1;
                        buffer = malloc(buffer_length);
                        memcpy(buffer, child1->node_string,
                               child1->node_string_length);
                        buffer[child1->node_string_length] = ' ';
                        memcpy(buffer + child1->node_string_length + 1,
                               child2->node_string, child2->node_string_length);
                        rewrite_to_string_constant(node, payload, buffer,
                                                   buffer_length);
                        break;

                    case OP_CONCAT:
                        buffer_length =
                                child1->node_string_length +
                                child2->node_string_length;
                        buffer = malloc(buffer_length);
                        memcpy(buffer, child1->node_string,
                               child1->node_string_length);
                        memcpy(buffer + child1->node_string_length,
                               child2->node_string, child2->node_string_length);
                        rewrite_to_string_constant(node, payload, buffer,
                                                   buffer_length);
                        break;

                    case OP_ADD:
                        if (node->value_type == TP_FLOAT)
                            rewrite_to_float_constant(node, payload,
                                                      child1->float_value +
                                                      child2->float_value);
                        else
                            rewrite_to_integer_constant(node, payload,
                                                        child1->int_value +
                                                        child2->int_value);
                        break;

                    case OP_MINUS:
                        if (node->value_type == TP_FLOAT)
                            rewrite_to_float_constant(node, payload,
                                                      child1->float_value -
                                                      child2->float_value);
                        else
                            rewrite_to_integer_constant(node, payload,
                                                        child1->int_value -
                                                        child2->int_value);
                        break;

                    case OP_MULT:
                        if (node->value_type == TP_FLOAT)
                            rewrite_to_float_constant(node, payload,
                                                      child1->float_value *
                                                      child2->float_value);
                        else
                            rewrite_to_integer_constant(node, payload,
                                                        child1->int_value *
                                                        child2->int_value);
                        break;

                    case OP_POWER:
                        if (node->value_type == TP_FLOAT)
                            rewrite_to_float_constant(node, payload,
                                                      pow(child1->float_value,
                                                          child2->float_value));
                        else
                            rewrite_to_integer_constant(node, payload,
                                                        (rxinteger) pow(
                                                                (double)child1->int_value,
                                                                (double)child2->int_value));
                        break;

                    case OP_DIV:
                        if (node->value_type == TP_FLOAT)
                            rewrite_to_float_constant(node, payload,
                                                      child1->float_value /
                                                      child2->float_value);
                        else /* Never Happens */
                            rewrite_to_integer_constant(node, payload,
                                                        child1->int_value /
                                                        child2->int_value);
                        break;

                    case OP_IDIV:
                        if (node->value_type == TP_FLOAT)
                            rewrite_to_float_constant(node, payload,
                                                      floor(child1->float_value /
                                                            child2->float_value));
                        else
                            rewrite_to_integer_constant(node, payload,
                                                        child1->int_value /
                                                        child2->int_value);
                        break;

                    case OP_MOD:
                        if (node->value_type == TP_FLOAT)
                            rewrite_to_float_constant(node, payload,
                                                      fmod(child1->float_value,
                                                           child2->float_value));
                        else
                            rewrite_to_integer_constant(node, payload,
                                                        child1->int_value %
                                                        child2->int_value);
                        break;

                    case OP_NOT:
                        if (node->value_type == TP_FLOAT)
                            rewrite_to_boolean_constant(node, payload,
                                                            child1->float_value ==
                                                            0.0);
                        else
                            rewrite_to_boolean_constant(node, payload,
                                                            child1->int_value ==
                                                            0);
                        break;

                    case OP_NEG:
                        if (node->value_type == TP_FLOAT)
                            rewrite_to_float_constant(node, payload,
                                                          -child1->float_value);
                        else
                            rewrite_to_integer_constant(node, payload,
                                                            -child1->int_value);
                        break;

                    case OP_PLUS:
                        if (node->value_type == TP_FLOAT)
                            rewrite_to_float_constant(node, payload,
                                                          child1->float_value);
                        else
                            rewrite_to_integer_constant(node, payload,
                                                            child1->int_value);
                        break;

                    case CONST_SYMBOL: /* Should not be being used in the AST at this stage - but for safety */
                    case FLOAT:
                    case INTEGER:
                    case STRING:
                        node->node_type = CONSTANT;
                        string_to_type(node, node->target_type);
                        update_string(node);
                        payload->changed = 1;
                        break;

                    default:;
                }
        }

        if (node->scope) payload->current_scope = payload->current_scope->parent;
    }

    return result_normal;
}


/* Propagate constant symbols handler */
static void constant_symbols_in_scope(Symbol *symbol, void *pload) {
    /* TODO the logic here must be revisited once we have control flow analysis */
    Payload* payload = pload;
    ASTNode* n;
    ASTNode* value_node;
    size_t i;

    if (symbol->is_constant) return; /* already done */

    n = sym_trnd(symbol, 0)->node;
    if (n->parent->node_type == ASSIGN  && n->node_type == VAR_TARGET && n->sibling->node_type == CONSTANT) {
        /* OK this could be a constant symbol */
        value_node = n->sibling;
        /* Check to see if it is updated */
        for (i=1; i<sym_nond(symbol); i++) {
            if (sym_trnd(symbol, i)->writeUsage) return; /* Not a constant as it is updated */
        }
    }
    else return;

    /* Set all the AST nodes to constants - copied from the value_node */
    symbol->is_constant = 1;
    payload->changed = 1;
    for (i=1; i<sym_nond(symbol); i++) {
        n = sym_trnd(symbol, i)->node;
        n->node_type = CONSTANT;
        n->value_type = value_node->value_type;
        n->int_value = value_node->int_value;
        n->float_value = value_node->float_value;
        if (n->free_node_string) {
            free(n->node_string);
            n->free_node_string = 0;
        }
        n->node_string = value_node->node_string;
        n->node_string_length = value_node->node_string_length;

        /* Patchup to the right target type */
        string_to_type(n, n->target_type);
        update_string(n);
        payload->changed = 1;
    }

    /* Prune the original assignment from the tree */
    ast_del(value_node->parent);
}

/* Propagate constant symbols  */
static void propagete_constant_symbols(Scope* scope, Payload* payload) {
    int i;
    if (!scope) return;

    scp_4all(scope, constant_symbols_in_scope, payload);
    for (i=0; i < scp_noch(scope); i++) {
        propagete_constant_symbols(scp_chd(scope, i), payload);
    }
}

/* Step 2
 * - Convert copy by value to copy by reference if the argument is a constant
 */
static walker_result opt2_walker(walker_direction direction,
                                 ASTNode* node,
                                 void *pload) {

    Payload *payload = (Payload *) pload;
    Symbol *symbol;
    size_t i;
    int is_constant;

    if (direction == in) {
        /* IN - TOP DOWN */
        if (node->scope) payload->current_scope = node->scope;

        if (node->node_type == ARG) {
            if (!node->is_ref_arg) { /* Only if it is pass by reference */
                is_constant = 1;
                symbol = node->child->symbolNode->symbol; /* The symbol is linked to the child node */
                /* Check to see if the symbol is written to in the procedure */
                for (i=1; i<sym_nond(symbol); i++) {
                    if (sym_trnd(symbol, i)->writeUsage) {
                        is_constant = 0; /* Not a constant as it is updated */
                        break;
                    }
                }
                /* If it us readonly make the argument pass by reference */
                if (is_constant) node->is_ref_arg = 1;
            }
        }
    }

    return result_normal;
}

/* Optimise AST Tree */
void optimise(Context *context) {
    Payload payload;

    payload.current_scope = 0;
    payload.context = context;

    payload.changed = 0;

    while (1) {
        payload.changed = 0;

        /* Constant Folding */
        ast_wlkr(context->ast, opt1_walker, (void *) &payload);

        /* Propagate constant symbols  */
        propagete_constant_symbols(context->ast->scope, &payload);

        if (!payload.changed) break;
    }

    /* Constant Folding */
    ast_wlkr(context->ast, opt2_walker, (void *) &payload);

}