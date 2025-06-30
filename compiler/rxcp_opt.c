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
#include "rxvmplugin_framework.h"
#include "rxbin.h" /* Needed for rxvmvars.h */
#include "rxvmvars.h"

/* Optimiser payload */
typedef struct Payload {
    Context *context;
    Scope *current_scope;
    char changed;
} Payload;

static void rewrite_to_string_constant(ASTNode* node, Payload* payload, char* string, size_t length);
static void rewrite_to_float_constant(ASTNode* node, Payload* payload, double value);
static void rewrite_to_decimal_constant(ASTNode* node, Payload* payload, char* value);
static void rewrite_to_integer_constant(ASTNode* node, Payload* payload, rxinteger value);
static void rewrite_to_boolean_constant(ASTNode* node, Payload* payload, int value);

/* Update the string representation if a CONSTANT node */
static void update_string(ASTNode* node) {
    char* buffer;
    size_t length;
    if (ast_hase(node)) return; /* Don't over write error codes */
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

        case TP_DECIMAL:
            {
                /* We need to convert the decimal to a string via the plugin to get the correct format / digits */
                if (node->decimal_value == 0) {
                    /* No decimal value - so just return an empty string */
                    buffer = malloc(1);
                    buffer[0] = '\0';
                    ast_sstr(node, buffer, 0);
                    return;
                }
                Context* context = node->context;
                decplugin* decplugin = context->decimal_plugin;
                value* value = value_f();

                decplugin->decimalFromString(decplugin, value, node->decimal_value);
                char* result_string = malloc(decplugin->getRequiredStringSize(decplugin) );
                decplugin->decimalToString(decplugin, value, result_string);
                /* Update the node's string - this also takes ownership of the memory */
                ast_sstr(node, result_string, strlen(result_string));
                clear_value(value);
                free(value);
            }
            return;

        default:
            /* Do nothing */
            return;
    }
}

/* Convert a CONSTANT node from a STRING to new_type */
static void string_to_type(ASTNode* node, ValueType new_type) {
        double f, floor_val;
        switch (new_type) {
            case TP_FLOAT:
                node->value_type = TP_FLOAT;
                node->target_type = TP_FLOAT;
                if (string2float(&node->float_value,node->node_string,node->node_string_length)) {
                    mknd_err(node, "BAD_CONVERSION");
                }
                return;
            case TP_DECIMAL:
                node->value_type = TP_DECIMAL;
                node->target_type = TP_DECIMAL;
                if (stringtodecimal(&node->decimal_value,node->node_string,node->node_string_length)) {
                    mknd_err(node, "BAD_CONVERSION");
                }
                return;
            case TP_BOOLEAN:
                node->value_type = TP_BOOLEAN;
                node->target_type = TP_BOOLEAN;
                /* Convert it to a float and then to 1/0 - slow but safe */
                if (string2float(&f,node->node_string,node->node_string_length)) {
                    mknd_err(node, "BAD_CONVERSION");
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
                        mknd_err(node, "BAD_CONVERSION");
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
                    mknd_err(node, "BAD_CONVERSION");
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
        ast_prnc(node);
        node->value_type = TP_STRING;
        node->node_type = CONSTANT;
        ast_sstr(node, string, length);
        string_to_type(node, node->target_type);
        payload->changed = 1;
    }

    static void rewrite_to_float_constant(ASTNode* node, Payload* payload, double value) {
        ast_prnc(node);
        node->value_type = TP_FLOAT;
        node->node_type = CONSTANT;
        node->float_value = value;
        update_string(node);
        string_to_type(node, node->target_type);
        payload->changed = 1;
    }

    static void rewrite_to_decimal_constant(ASTNode* node, Payload* payload, char* value) {
        size_t length = strlen(value);
        ast_prnc(node);
        node->value_type = TP_DECIMAL;
        node->node_type = CONSTANT;
        node->decimal_value = malloc(length + 1); // +1 for null terminator
        strcpy(node->decimal_value, value);
        update_string(node);
        string_to_type(node, node->target_type);
        payload->changed = 1;
    }

    static void rewrite_to_integer_constant(ASTNode* node, Payload* payload, rxinteger value) {
        ast_prnc(node);
        node->value_type = TP_INTEGER;
        node->node_type = CONSTANT;
        node->int_value = value;
        update_string(node);
        string_to_type(node, node->target_type);
        payload->changed = 1;
    }

    static void rewrite_to_boolean_constant(ASTNode* node, Payload* payload, int value) {
        if (value) value = 1;
        ast_prnc(node);
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

    if (node1->value_type == TP_INTEGER) {
        idiff = node1->int_value - node2->int_value;
        return idiff>0 ? 1 : (idiff<0 ? -1 : 0);
    }

    if (node1->value_type == TP_BOOLEAN) {
        idiff = (node1->int_value != 0) - (node2->int_value != 0); /* Weird */
        return idiff>0 ? 1 : (idiff<0 ? -1 : 0);
    }

    if (node1->value_type == TP_FLOAT) {
        fdiff = node1->float_value - node2->float_value;
        return fdiff>0 ? 1 : (fdiff<0 ? -1 : 0);
    }

    if (node1->value_type == TP_DECIMAL) {
        Context* context = node1->context;
        decplugin* decplugin = context->decimal_plugin;
        value* val1 = value_f();
        value* val2 = value_f();
        decplugin->decimalFromString(decplugin, val1, node1->decimal_value);
        decplugin->decimalFromString(decplugin, val2, node2->decimal_value);
        int cmp = decplugin->decimalCompare(decplugin, val1, val2);
        clear_value(val1);
        free(val1);
        clear_value(val2);
        free(val2);
        return cmp;
    }

    /* Use STRING */
    if ((idiff = memcmp(node1->node_string, node2->node_string,
                        MIN(node1->node_string_length,
                            node2->node_string_length)) != 0))
        return (int)idiff;
    else {
        idiff = (rxinteger)node1->node_string_length - (rxinteger)node2->node_string_length;
        return idiff>0 ? 1 : (idiff<0 ? -1 : 0);
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
    int index;

    if (direction == in) {
        /* IN - TOP DOWN */
        payload->current_scope = node->scope;
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
                    ast_prun(node);
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
                        if (node->value_type == TP_FLOAT) {
                            rewrite_to_float_constant(node, payload,
                                                      child1->float_value +
                                                      child2->float_value);
                        }
                        else if (node->value_type == TP_DECIMAL) {
                            /* Decimal addition */
                            value* val1 = value_f();
                            value* val2 = value_f();
                            value* result = value_f();
                            Context* context = node->context;
                            decplugin* decplugin = context->decimal_plugin;
                            decplugin->decimalFromString(decplugin, val1, child1->decimal_value);
                            decplugin->decimalFromString(decplugin, val2, child2->decimal_value);
                            decplugin->decimalAdd(decplugin, result, val1, val2);
                            char* result_string = malloc(decplugin->getRequiredStringSize(decplugin) );
                            decplugin->decimalToString(decplugin, result, result_string);
                            rewrite_to_decimal_constant(node, payload, result_string);
                            free(result_string);
                            clear_value(val1);
                            free(val1);
                            clear_value(val2);
                            free(val2);
                            clear_value(result);
                            free(result);
                        }
                        else {
                            /* Must be integer */
                            rewrite_to_integer_constant(node, payload,
                                                        child1->int_value +
                                                        child2->int_value);
                        }
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
                        else {
                            rewrite_to_integer_constant(node, payload,
                                                        (rxinteger) pow(
                                                                (double)child1->int_value,
                                                                (double)child2->int_value));
                            if (!node->int_value) mknd_err(node, "OVERFLOW_UNDERFLOW");
                        }
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
                        if (node->value_type == TP_FLOAT) {
                            rewrite_to_float_constant(node, payload,
                                                          child1->float_value);
                        }
                        else if (node->value_type == TP_DECIMAL) {
                            rewrite_to_decimal_constant(node, payload,
                                                            child1->decimal_value);
                        }
                        else {
                            /* Must be integer */
                            rewrite_to_integer_constant(node, payload,
                                                            child1->int_value);
                        }
                        break;

                    case CONST_SYMBOL: /* Should not be being used in the AST at this stage - but for safety */
                    case FLOAT:
                    case DECIMAL:
                    case INTEGER:
                    case STRING:
                        node->node_type = CONSTANT;
                        string_to_type(node, node->target_type);
                        update_string(node);
                        payload->changed = 1;
                        break;

                    case CONSTANT:
                        if ( node->parent &&
                             ( node->parent->node_type == VAR_REFERENCE ||
                               node->parent->node_type == VAR_TARGET ||
                               node->parent->node_type == VAR_SYMBOL ) ) {
                            /* If the parent is a Variable then the node is an array subscript, and so it must be >=0 */
                            if (node->target_type == TP_INTEGER) { /* This 'must' be true */
                                index = ast_chdi(node);
                                if (node->int_value < node->parent->symbolNode->symbol->dim_base[index]) mknd_err(node, "OUT_OF_RANGE");

                                else if (node->parent->symbolNode->symbol->dim_elements[index]) {
                                    /* There is a max number of elements - so check it */
                                    if (node->int_value > node->parent->symbolNode->symbol->dim_base[index] +
                                                          node->parent->symbolNode->symbol->dim_elements[index] - 1)
                                        mknd_err(node, "OUT_OF_RANGE");
                                }
                            }
                        }
                        else if (node->parent && (node->parent->node_type == OP_ARG_VALUE || node->parent->node_type == OP_ARG_IX_EXISTS)) {
                            if (node->target_type == TP_INTEGER) { /* This 'must' be true */
                                if (node->int_value < 1)
                                    mknd_err(node, "OUT_OF_RANGE");
                            }
                        }
                        break;

                    default:;
                }
        }

        payload->current_scope = node->scope;
    }

    return result_normal;
}


/* Propagate constant symbols handler */
static void constant_symbols_in_scope(Symbol *symbol, void *pload) {
    /* TODO the logic here must be revisited once we have control flow analysis */
    Payload* payload = pload;
    ASTNode* n;
    ASTNode* m;
    ASTNode* value_node;
    size_t i;
    ValueType value_type;
    rxinteger int_value;
    int bool_value;
    double float_value;
    char *decimal_value;
    char *node_string;
    size_t node_string_length;

    if (symbol->symbol_type == CONSTANT_SYMBOL) return; /* already done */
    if (symbol->value_dims) return; /* Arrays are never constants */
    if (!sym_nond(symbol)) return; /* No nodes - weird - return */


    n = sym_trnd(symbol, 0)->node;
    if (n->parent->node_type == ASSIGN  && n->node_type == VAR_TARGET && n->sibling->node_type == CONSTANT) {
        /* OK this could be a constant symbol */
        value_node = n->sibling;
        /* Check to see if it is updated */
        for (i=1; i<sym_nond(symbol); i++) {
            if (sym_trnd(symbol, i)->writeUsage) return; /* Not a constant as it is updated */
        }
        value_type = value_node->value_type;
        int_value = value_node->int_value;
        bool_value = value_node->bool_value;
        float_value = value_node->float_value;
        decimal_value = value_node->decimal_value; /* Memory management is owned by the node */
        node_string = value_node->node_string;
        node_string_length = value_node->node_string_length;

        /* Prune the original assignment from the tree */
        ast_prun(n->parent);
    }

    else if (n->parent->node_type == DEFINE) {
        /* OK - an explicit declaration, this could be a constant symbol */

        /* After the "define" usage could be an "assign" - this is the value of the constant */
        if (sym_nond(symbol) < 2) m = 0;
        else m = sym_trnd(symbol,1)->node;
        if (m && m->parent->node_type == ASSIGN) {

            /* Check to see if it is updated - we check after the next node (i=2) because the first write is just
             * setting the initial value, but any writes after than mean it is not a constant */
            for (i=2; i<sym_nond(symbol); i++) {
                if (sym_trnd(symbol, i)->writeUsage) return; /* Not a constant as it is updated */
            }

            if (m->node_type == VAR_TARGET && m->sibling->node_type == CONSTANT) {
                value_node = m->sibling;
                value_type = value_node->value_type;
                int_value = value_node->int_value;
                bool_value = value_node->bool_value;
                float_value = value_node->float_value;
                decimal_value = value_node->decimal_value; /* Memory management is owned by the node */
                node_string = value_node->node_string;
                node_string_length = value_node->node_string_length;

                /* Remove the assign */
                ast_prun(m->parent);

                /* Remove the define */
                m = sym_trnd(symbol,0)->node->parent;
                sym_dnd(symbol, 0);
                ast_prun(m);
            }
            else return; /* The assign is not a constant - so neither if the symbol */
        }

        else {

            /* Check to see if the symbol is updated */
            for (i=1; i<sym_nond(symbol); i++) {
                if (sym_trnd(symbol, i)->writeUsage) return; /* Not a constant as it is updated */
            }

            /* Otherwise the value is zero */
            value_node = n->sibling;
            value_type = value_node->value_type;
            int_value = 0;
            bool_value = 0;
            float_value = 0.0;
            decimal_value = "0.0"; /* Memory management is owned by the node - in this case this is on the stack */
            /* if type is string it is blank, else 0 */
            if (value_type == TP_STRING) {
                node_string = "";
                node_string_length = 0;
            }
            else {
                node_string = "0";
                node_string_length = 1;
            }

            /* Remove the define */
            m = sym_trnd(symbol,0)->node->parent;
            ast_prun(m);
        }
    }

    else return;

    /* Set all the AST nodes to constants - copied from the value_node */
    symbol->symbol_type = CONSTANT_SYMBOL;
    payload->changed = 1;
    for (i=0; i<sym_nond(symbol); i++) {
        n = sym_trnd(symbol, i)->node;
        n->node_type = CONSTANT;
        n->value_type = value_type;
        n->int_value = int_value;
        n->float_value = float_value;
        /* If the value is a decimal then we need to copy the string into a new malloc'd string */
        if (decimal_value) {
            if (n->decimal_value) free(n->decimal_value); /* Free the old one */
            n->decimal_value = malloc(strlen(decimal_value) + 1);
            strcpy(n->decimal_value, decimal_value);
        }
        else {
            if (n->decimal_value) {
                free(n->decimal_value);
                n->decimal_value = 0;
            }
        }
        n->bool_value = bool_value;
        if (n->free_node_string) {
            free(n->node_string);
            n->free_node_string = 0;
        }
        n->node_string = node_string;
        n->node_string_length = node_string_length;

        /* Patch up to the right target type */
        string_to_type(n, n->target_type);
        update_string(n);
        payload->changed = 1;
    }
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
        payload->current_scope = node->scope;

        if (node->node_type == ARG) {
            if (!node->is_ref_arg) { /* Only if it is pass by value */

                /* Check if we are in a definition (external procedure) */

                /*  ARG > ARGS > PROC --- And then see in there is an INSTRUCTIONS Node, null means there isn't */
                if (ast_chld(node->parent->parent, INSTRUCTIONS, 0) == 0) return result_normal;

                /* Internal Procedure - do constant check */
                is_constant = 1;
                if (node->child->symbolNode) { /* If there is no symbol it is a varg (and this optimisation is irrelevant) */
                    symbol = node->child->symbolNode->symbol; /* The symbol is linked to the child node */
                    /* Check to see if the symbol is written to in the procedure */
                    for (i = 1; i < sym_nond(symbol); i++) {
                        if (sym_trnd(symbol, i)->writeUsage) {
                            is_constant = 0; /* Not a constant as it is updated */
                            break;
                        }
                    }
                    /* If it is readonly make the argument as const - this makes the emitter not bother to duplicate the register */
                    if (is_constant) node->is_const_arg = 1;
                }
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

    /* Constant Arguments converted to pass by reference */
    ast_wlkr(context->ast, opt2_walker, (void *) &payload);

}