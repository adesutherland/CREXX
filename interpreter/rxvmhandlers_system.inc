/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2026 the cREXX contributors
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

/* System RXAS handler definitions; body text is compiled by RXVM_HANDLER. */
RXVM_HANDLER(FREADCDPT_REG_REG, VM_ADVANCE(2);
        DEBUG("TRACE - FREADCDPT R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        {
            RXVM_UTF8_ONLY(
            int codepoint;
            int first_byte;
            int next_byte;
            int invalid = 0;
            size_t chars = 0;
            string_cache_reset(op1R);
            prep_string_buffer(op1R, 4);

            /* Read the first byte - determines length */
            first_byte = fgetc((FILE *)op2R->int_value);
            if (first_byte == EOF) {
                op1R->int_value = -1;
                op1R->string_length = 0;
                op1R->string_chars = 0;
                string_cache_reset(op1R);
                mark_utf8_valid_count(op1R);
            } else {
                op1R->string_value[0] = (char)first_byte;

                /* Read the rest of the code point */
                if ((unsigned char)op1R->string_value[0] < 128) {
                    op1R->string_length = 1;
                } else if ((unsigned char)op1R->string_value[0] < 224) {
                    op1R->string_length = 2;
                } else if ((unsigned char)op1R->string_value[0] < 240) {
                    op1R->string_length = 3;
                } else {
                    op1R->string_length = 4;
                }

                if (op1R->string_length > 1) {
                    next_byte = fgetc((FILE *)op2R->int_value);
                    if (next_byte == EOF) invalid = 1;
                    else op1R->string_value[1] = (char)next_byte;
                }
                if (op1R->string_length > 2) {
                    next_byte = fgetc((FILE *)op2R->int_value);
                    if (next_byte == EOF) invalid = 1;
                    else op1R->string_value[2] = (char)next_byte;
                }
                if (op1R->string_length > 3) {
                    next_byte = fgetc((FILE *)op2R->int_value);
                    if (next_byte == EOF) invalid = 1;
                    else op1R->string_value[3] = (char)next_byte;
                }

                if (invalid || validate_utf8_bytes(op1R->string_value, op1R->string_length, &chars) != 0) {
                    SET_SIGNAL_MSG(RXSIGNAL_UNICODE_ERROR, "Invalid UTF-8 codepoint in text file read");
                } else {
                    utf8codepoint(op1R->string_value, &codepoint);
                    op1R->int_value = codepoint;
                    rxvm_value_set_string_chars_known(op1R, chars);
                    string_cache_reset(op1R);
                    mark_utf8_valid_count(op1R);
                }
            }
            )
            RXVM_BYTE_ONLY(
            {
                int byte = fgetc( (FILE*)op2R->int_value );
                if (byte == EOF) {
                    op1R->int_value = -1;
                    op1R->string_length = 0;
                } else {
                    prep_string_buffer(op1R, 1);
                    op1R->int_value = (unsigned char)byte;
                    op1R->string_value[0] = op1R->int_value;
                    op1R->string_length = 1;
                }
            }
            string_cache_reset(op1R);
            clear_vm_private_flags(op1R);
            )
        }
        DISPATCH;

    /* fwrite - fwrite to op1 file*(int) from op2(string) */

)

RXVM_HANDLER(FWRITE_REG_REG, VM_ADVANCE(2);
        DEBUG("TRACE - FWRITE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        fwrite(op2R->string_value, op2R->string_length, 1, (FILE*)op1R->int_value);
        DISPATCH;

    /* fwriteb - fwrite to op1 file*(int) from op2(binary) */

)

RXVM_HANDLER(FWRITEB_REG_REG, VM_ADVANCE(2);
    DEBUG("TRACE - FWRITEB R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
    fwrite(op2R->binary_value, op2R->binary_length, 1, (FILE*)op1R->int_value);
    DISPATCH;

    /* fwritebyte - write byte to op1 file*(int) op2 source(int) */

)

RXVM_HANDLER(FWRITEBYTE_REG_REG, VM_ADVANCE(2);
        DEBUG("TRACE - FWRITEBYTE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        fputc(op2R->int_value, (FILE*)op1R->int_value);
        DISPATCH;

    /* fwritecdpt - write codepoint to op1 file*(int) op2 source(int) */

)

RXVM_HANDLER(FWRITECDPT_REG_REG, VM_ADVANCE(2);
        DEBUG("TRACE - FWRITECDPT R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        {
            char codepoint[4];
            size_t length_of_codepoint;
            RXVM_UTF8_ONLY(
            char* end_of_codepoint;

            end_of_codepoint = utf8catcodepoint(codepoint, op2R->int_value, 4);
            length_of_codepoint = end_of_codepoint - codepoint;
            )
            RXVM_BYTE_ONLY(
            length_of_codepoint = 1;
            codepoint[0] = op2R->int_value;
            )
            fwrite(codepoint, length_of_codepoint, 1, (FILE*)op1R->int_value);
        }
        DISPATCH;

    /* fclearerr - clearerr op1 file*(int) */

)

RXVM_HANDLER(FCLEARERR_REG, VM_ADVANCE(1);
        DEBUG("TRACE - FCLEARERR R%lu\n", REG_IDX(1));
        clearerr((FILE*)op1R->int_value);
        DISPATCH;

    /* feof - op1 rc(int) = feof op2 file*(int) */

)

RXVM_HANDLER(FEOF_REG_REG, VM_ADVANCE(2);
        DEBUG("TRACE - FEOF R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        op1R->int_value = feof((FILE*)op2R->int_value);
        DISPATCH;

    /* ferror - op1 rc(int) = ferror op2 file*(int) */

)

RXVM_HANDLER(FERROR_REG_REG, VM_ADVANCE(2);
        DEBUG("TRACE - FERROR R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        op1R->int_value = ferror((FILE*)op2R->int_value);
        DISPATCH;

    /* ichkrng - if op1<op2 | op1>op3 signal OUT_OF_RANGE */

)

RXVM_HANDLER(ICHKRNG_REG_INT_INT, VM_ADVANCE(3);
        DEBUG("TRACE - ICHKRNG R%lu,%d,%d\n", REG_IDX(1), (int)op2I, (int)op3I);
        if (op1R->int_value < op2I || op1R->int_value > op3I) SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE)
        DISPATCH;

    /* ichkrng - if op1<op2 | op1>op3 signal OUT_OF_RANGE */

)

RXVM_HANDLER(ICHKRNG_REG_INT_REG, VM_ADVANCE(3);
        DEBUG("TRACE - ICHKRNG R%lu,%d,R%lu\n", REG_IDX(1), (int)op2I, REG_IDX(3));
        if (op1R->int_value < op2I || op1R->int_value > op3R->int_value) SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE)
        DISPATCH;

    /* ichkrng - if op1<op2 | op1>op3 signal OUT_OF_RANGE */

)

RXVM_HANDLER(ICHKRNG_REG_REG_REG, VM_ADVANCE(3);
        DEBUG("TRACE - ICHKRNG R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
        if (op1R->int_value < op2R->int_value || op1R->int_value > op3R->int_value) SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE)
        DISPATCH;

    /* ichkrng - if op1<op2 | op1>op3 signal OUT_OF_RANGE */

)

RXVM_HANDLER(ICHKRNG_INT_INT_REG, VM_ADVANCE(3);
        DEBUG("TRACE - ICHKRNG %d,%d,R%lu\n", (int)op1I, (int)op2I, REG_IDX(3));
        if (op1I < op2I || op1I > op3R->int_value) SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE)
        DISPATCH;

    /* ichkrng - if op1<op2 | op1>op3 signal OUT_OF_RANGE */

)

RXVM_HANDLER(ICHKRNG_INT_REG_REG, VM_ADVANCE(3);
        DEBUG("TRACE - ICHKRNG %d,R%lu,R%lu\n", (int)op1I, REG_IDX(2), REG_IDX(3));
        if (op1I < op2R->int_value || op1I > op3R->int_value) SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE)
        DISPATCH;

    /* getenv - get environment variable, op1=env[op2] */

)

RXVM_HANDLER(GETENV_REG_REG, VM_ADVANCE(2);
        DEBUG("TRACE - GETENV R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        {
            char *value;
            int should_free = getEnvVal(&value, op2R->string_value, op2R->string_length);
            set_null_string(op1R, value);
            if (should_free) free(value);
        }
        DISPATCH;

    /* getenv - get environment variable, op1=env[op2] */

)

RXVM_HANDLER(GETENV_REG_STRING, VM_ADVANCE(2);
        DEBUG("TRACE - GETENV R%lu,\"%.*s\"\n", REG_IDX(1),
              (int)(CONSTSTRING_OP(2))->string_len, (CONSTSTRING_OP(2))->string);
        {
            char *value;
            int should_free = getEnvVal(&value, (CONSTSTRING_OP(2))->string, (CONSTSTRING_OP(2))->string_len);
            set_null_string(op1R, value);
            if (should_free) free(value);
        }
        DISPATCH;


    /* ------------------------------------------------------------------------------------
 *  TRIMR_REG_REG_REG  Trim right with char                          pej updated Jan 2026
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(TRIMR_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - TRIMR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
                if (op1R != op2R) set_value_string(op1R, op2R);
                if (op3R->string_length > 0) {
                    char trim_char = op3R->string_value[0];
                    size_t i = op1R->string_length;
                    while (i > 0 && op1R->string_value[i - 1] == trim_char) {
                        i--;
                    }
                    rxvm_value_set_string_length_known(op1R, i);
                }
                null_terminate_string_buffer(op1R);
                finish_string_write(op1R, op1R->string_length);
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  TRIML_REG_REG_REG  Trim left with char                           pej updated Jan 2026
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(TRIML_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - TRIML R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
                size_t i = 0;
                if (op1R != op2R) set_value_string(op1R, op2R);
                if (op3R->string_length > 0) {
                    char trim_char = op3R->string_value[0];
                    while (i < op1R->string_length && op1R->string_value[i] == trim_char) i++;
                }

                if (i >= op1R->string_length) {
                    op1R->string_length = 0;
                } else {
                    rxvm_value_set_string_length_known(
                            op1R, op1R->string_length - i);
                    if (i > 0) memmove(op1R->string_value, op1R->string_value + i, op1R->string_length);
                }
                null_terminate_string_buffer(op1R);
                finish_string_write(op1R, op1R->string_length);
            }
            DISPATCH;

/* ------------------------------------------------------------------------------------
 *  TRUNC_REG_REG_REG  Truncate string                              pej updated Jan 2026
 *  -----------------------------------------------------------------------------------
 */

)

RXVM_HANDLER(TRUNC_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - TRUNC R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
                rxinteger len = op3R->int_value;
                if (op1R != op2R) set_value_string(op1R, op2R);
                if (len < 0) len = 0;
                if (len < op1R->string_length) {
                    rxvm_value_set_string_length_known(
                            op1R, (size_t)len);
                    null_terminate_string_buffer(op1R);
                    finish_string_write(op1R, op1R->string_length);
                }
            }
            DISPATCH;


)

#define RXVM_UNKNOWN_INSTRUCTION_BODY()                                       \
    do {                                                                       \
        SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);                              \
        DISPATCH;                                                              \
    } while (0)

RXVM_HANDLER(IUNKNOWN,
            RXVM_UNKNOWN_INSTRUCTION_BODY();
)

RXVM_HANDLER(INULL,
            RXVM_UNKNOWN_INSTRUCTION_BODY();


)

#undef RXVM_UNKNOWN_INSTRUCTION_BODY

RXVM_HANDLER(EXIT,
            DEBUG("TRACE - EXIT");
            rc = 0;
            RXVM_HANDLER_FINISH();


)

RXVM_HANDLER(EXIT_INT,
            DEBUG("TRACE - EXIT %d\n", (int)op1I);
            rc = op1I;
            RXVM_HANDLER_FINISH();


)

RXVM_HANDLER(EXIT_REG,
            DEBUG("TRACE - EXIT R%lu\n", REG_IDX(1));
            rc = op1RI;
            RXVM_HANDLER_FINISH();


)

RXVM_HANDLER(SETOBJTYPE_REG_STRING, VM_ADVANCE(2);
            {
                RxGraph *graph = runtime_module_graph(current_module);
                RxGraphId type = (RxGraphId)(pc + 2)->index;
                const RxGraphTypeRef *type_ref = rx_graph_type_ref(graph, type);
                DEBUG("TRACE - SETOBJTYPE R%lu,TYPE#%u(%s)\n",
                      REG_IDX(1), type, type_ref ? type_ref->name : "?");
                op1R->object_type = type_ref;
            }
            clear_value_uninitialized_object(op1R);
            DISPATCH;


)

RXVM_HANDLER(SETOBJUNINIT_REG_STRING, VM_ADVANCE(2);
            {
                RxGraph *graph = runtime_module_graph(current_module);
                RxGraphId type = (RxGraphId)(pc + 2)->index;
                const RxGraphTypeRef *type_ref = rx_graph_type_ref(graph, type);
                DEBUG("TRACE - SETOBJUNINIT R%lu,TYPE#%u(%s)\n",
                      REG_IDX(1), type, type_ref ? type_ref->name : "?");
                value_zero(op1R);
                op1R->object_type = type_ref;
            }
            mark_value_uninitialized_object(op1R);
            DISPATCH;


)

RXVM_HANDLER(ASSERTINITIALIZED_REG, VM_ADVANCE(1);
            DEBUG("TRACE - ASSERTINITIALIZED R%lu\n", REG_IDX(1));
            if (value_is_uninitialized_object(op1R)) {
                char *error_message = build_runtime_uninitialized_object_error(op1R);
                if (error_message) {
                    SET_SIGNAL_MSG(RXSIGNAL_OBJECT_NOT_INITIALIZED, error_message);
                    rxvm_internal_free(error_message);
                } else {
                    SET_SIGNAL(RXSIGNAL_OBJECT_NOT_INITIALIZED);
                }
                DISPATCH;
            }
            DISPATCH;


)

RXVM_HANDLER(ISINITIALIZED_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - ISINITIALIZED R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            value_zero(op1R);
            set_int(op1R, value_is_uninitialized_object(op2R) ? 0 : 1);
            DISPATCH;


)

RXVM_HANDLER(SRCMETHODSEL_REG_REG_STRING, VM_ADVANCE(3);
            {
                proc_runtime *called_function;
                const char *class_name;
                size_t class_name_length;
                RxGraph *graph;
                RxMemberId member;
                char *descriptor;
                rxvm_dynamic_site_cache *site_cache;

                graph = runtime_module_graph(current_module);
                member = (RxMemberId)(pc + 3)->index;
                descriptor = 0;
                site_cache = rxvm_dynamic_cache_for_site(
                    current_module, VM_CANONICAL_INDEX(pc));
                DEBUG("TRACE - SRCMETHODSEL R%lu,R%lu,MEMBER#%u\n",
                      REG_IDX(1), REG_IDX(2), member);
                RXVM_INSTRUMENTATION_DYNAMIC(
                        RXVM_PROFILE_DYNAMIC_METHOD_SELECTION,
                        RXVM_PROFILE_DYNAMIC_ATTEMPT);

                if (value_is_uninitialized_object(op2R)) {
                    char *error_message = build_runtime_uninitialized_object_error(op2R);
                    if (error_message) {
                        SET_SIGNAL_MSG(RXSIGNAL_OBJECT_NOT_INITIALIZED, error_message);
                        rxvm_internal_free(error_message);
                    } else {
                        SET_SIGNAL(RXSIGNAL_OBJECT_NOT_INITIALIZED);
                    }
                    RXVM_INSTRUMENTATION_DYNAMIC(
                            RXVM_PROFILE_DYNAMIC_METHOD_SELECTION,
                            RXVM_PROFILE_DYNAMIC_FAILURE);
                    DISPATCH;
                }

                class_name = runtime_value_type_name(op2R,
                                                     &class_name_length);
                called_function = 0;

                if (!class_name || !class_name_length) {
                    descriptor = rx_graph_operand_text(graph,
                                                       OP_SRCMETHODSEL_REG_REG_STRING,
                                                       2u,
                                                       member);
                    if (descriptor) {
                        SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, descriptor);
                        free(descriptor);
                    }
                    else {
                        SET_SIGNAL(RXSIGNAL_FAILURE);
                    }
                    RXVM_INSTRUMENTATION_DYNAMIC(
                            RXVM_PROFILE_DYNAMIC_METHOD_SELECTION,
                            RXVM_PROFILE_DYNAMIC_FAILURE);
                    DISPATCH;
                }

                called_function = op2R->object_type &&
                                  op2R->object_type->graph == graph
                    ? resolve_runtime_graph_method_cached(
                          context,
                          current_module->graph_binding,
                          site_cache,
                          op2R,
                          member)
                    : 0;
                if (!called_function) {
                    descriptor = rx_graph_operand_text(graph,
                                                       OP_SRCMETHODSEL_REG_REG_STRING,
                                                       2u,
                                                       member);
                    if (descriptor) {
                        called_function = resolve_runtime_method(context,
                                                                 class_name,
                                                                 class_name_length,
                                                                 descriptor,
                                                                 strlen(descriptor));
                    }
                }
                if (!called_function) {
                    if (!descriptor) {
                        SET_SIGNAL(RXSIGNAL_FAILURE);
                        RXVM_INSTRUMENTATION_DYNAMIC(
                                RXVM_PROFILE_DYNAMIC_METHOD_SELECTION,
                                RXVM_PROFILE_DYNAMIC_FAILURE);
                        DISPATCH;
                    }
                    SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, descriptor);
                    free(descriptor);
                    RXVM_INSTRUMENTATION_DYNAMIC(
                            RXVM_PROFILE_DYNAMIC_METHOD_SELECTION,
                            RXVM_PROFILE_DYNAMIC_FAILURE);
                    DISPATCH;
                }
                free(descriptor);

                value_zero(op1R);
                op1R->int_value = (rxinteger) called_function;
                RXVM_INSTRUMENTATION_DYNAMIC(
                        RXVM_PROFILE_DYNAMIC_METHOD_SELECTION,
                        RXVM_PROFILE_DYNAMIC_SUCCESS);
            }
            DISPATCH;


)

RXVM_HANDLER(SRCFPROCSEL_REG_STRING_REG, VM_ADVANCE(3);
            {
                proc_runtime *called_function;
                char *error_message;
                char *descriptor;
                RxGraph *graph;
                RxFactoryId factory;
                int graph_result;
                rxvm_dynamic_site_cache *site_cache;
                const rxvm_graph_factory_binding *factory_binding;

                graph = runtime_module_graph(current_module);
                factory = (RxFactoryId)(pc + 2)->index;
                site_cache = rxvm_dynamic_cache_for_site(
                    current_module, VM_CANONICAL_INDEX(pc));
                DEBUG("TRACE - SRCFPROCSEL R%lu,FACTORY#%u,R%lu\n",
                      REG_IDX(1), factory, REG_IDX(3));
                RXVM_INSTRUMENTATION_DYNAMIC(
                        RXVM_PROFILE_DYNAMIC_FACTORY_SELECTION,
                        RXVM_PROFILE_DYNAMIC_ATTEMPT);

                called_function = 0;
                error_message = 0;
                descriptor = 0;
                factory_binding = 0;

                if (site_cache &&
                    site_cache->generation != context->semantic_generation) {
                    memset(&site_cache->value, 0, sizeof(site_cache->value));
                    site_cache->generation = context->semantic_generation;
                }
                if (site_cache) {
                    if (site_cache->value.factory.state ==
                        RXVM_FACTORY_CACHE_UNKNOWN) {
                        factory_binding = rxvm_bound_graph_factory(
                            current_module->graph_binding, factory);
                        site_cache->value.factory.binding = factory_binding;
                        if (factory_binding && factory_binding->direct_target) {
                            site_cache->value.factory.target =
                                factory_binding->direct_target;
                            site_cache->value.factory.state =
                                RXVM_FACTORY_CACHE_DIRECT;
                        } else {
                            site_cache->value.factory.state =
                                RXVM_FACTORY_CACHE_GENERAL;
                        }
                    } else {
                        factory_binding = site_cache->value.factory.binding;
                    }
                    if (site_cache->value.factory.state ==
                        RXVM_FACTORY_CACHE_DIRECT) {
                        called_function = site_cache->value.factory.target;
                    }
                } else {
                    factory_binding = rxvm_bound_graph_factory(
                        current_module->graph_binding, factory);
                    if (factory_binding) {
                        called_function = factory_binding->direct_target;
                    }
                }
                if (called_function) {
                    graph_result = 1;
                } else {
                    graph_result = resolve_runtime_graph_factory(
                        context,
                        factory_binding,
                        (rxinteger) op3R->int_value,
                        (&(op3R)) + 1,
                        &called_function,
                        &error_message);
                }
                if (graph_result < 0) {
                    descriptor = rx_graph_operand_text(graph,
                                                       OP_SRCFPROCSEL_REG_STRING_REG,
                                                       1u,
                                                       factory);
                    if (descriptor &&
                        resolve_runtime_factory(context,
                                                descriptor,
                                                strlen(descriptor),
                                                (rxinteger) op3R->int_value,
                                                (&(op3R)) + 1,
                                                &called_function,
                                                &error_message)) {
                        graph_result = 1;
                    } else {
                        graph_result = 0;
                    }
                }
                free(descriptor);
                if (!graph_result) {
                    if (error_message) {
                        SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, error_message);
                        rxvm_internal_free(error_message);
                    } else {
                        SET_SIGNAL(RXSIGNAL_FAILURE);
                    }
                    RXVM_INSTRUMENTATION_DYNAMIC(
                            RXVM_PROFILE_DYNAMIC_FACTORY_SELECTION,
                            RXVM_PROFILE_DYNAMIC_FAILURE);
                    DISPATCH;
                }

                value_zero(op1R);
                op1R->int_value = (rxinteger) called_function;
                RXVM_INSTRUMENTATION_DYNAMIC(
                        RXVM_PROFILE_DYNAMIC_FACTORY_SELECTION,
                        RXVM_PROFILE_DYNAMIC_SUCCESS);
            }
            DISPATCH;


)

RXVM_HANDLER(TYPEOF_REG_REG, VM_ADVANCE(2);
            {
                char *type_name;
                const char *internal_type_name;
                size_t internal_type_name_length;

                DEBUG("TRACE - TYPEOF R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));

                internal_type_name = runtime_value_type_name(
                    op2R, &internal_type_name_length);
                if (internal_type_name && internal_type_name_length > 0) {
                    type_name = runtime_internal_type_to_source_name(
                        internal_type_name, internal_type_name_length);
                } else {
                    type_name = dup_runtime_name(".object", sizeof(".object") - 1u);
                }

                if (!type_name) {
                    SET_SIGNAL(RXSIGNAL_FAILURE);
                    DISPATCH;
                }

                value_zero(op1R);
                set_null_string(op1R, type_name);
                rxvm_internal_free(type_name);
            }
            DISPATCH;


)

RXVM_HANDLER(ISTYPE_REG_REG_STRING, VM_ADVANCE(3);
            {
                RxGraph *graph = runtime_module_graph(current_module);
                RxGraphId type = (RxGraphId)(pc + 3)->index;
                DEBUG("TRACE - ISTYPE R%lu,R%lu,TYPE#%u\n",
                      REG_IDX(1), REG_IDX(2), type);
                value_zero(op1R);
                set_int(op1R, runtime_value_matches_graph_type(context,
                                                               op2R,
                                                               graph,
                                                               type));
            }
            DISPATCH;


)

RXVM_HANDLER(ASSERTTYPE_REG_STRING, VM_ADVANCE(2);
            {
                RxGraph *graph = runtime_module_graph(current_module);
                RxGraphId type = (RxGraphId)(pc + 2)->index;
                const char *type_name = rx_graph_type_name(graph, type);
                DEBUG("TRACE - ASSERTTYPE R%lu,TYPE#%u(%s)\n",
                      REG_IDX(1), type, type_name ? type_name : "?");
                if (!runtime_value_matches_graph_type(context, op1R, graph, type)) {
                char *error_message = build_runtime_cast_error(op1R,
                                                               type_name ? type_name : "?",
                                                               type_name ? strlen(type_name) : 1u);
                if (error_message) {
                    SET_SIGNAL_MSG(RXSIGNAL_CONVERSION_ERROR, error_message);
                    rxvm_internal_free(error_message);
                } else {
                    SET_SIGNAL(RXSIGNAL_CONVERSION_ERROR);
                }
                DISPATCH;
                }
            }
            DISPATCH;


)

RXVM_HANDLER(SOCKNEW_REG, VM_ADVANCE(1);
            DEBUG("TRACE - SOCKNEW R%lu\n", REG_IDX(1));
            set_int(op1R, rxvm_socket_new(context));
            DISPATCH;


)

RXVM_HANDLER(SOCKCLOSE_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - SOCKCLOSE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            set_int(op1R, rxvm_socket_close(context, op2R->int_value));
            DISPATCH;


)

RXVM_HANDLER(SOCKCONNECT_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SOCKCONNECT R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            rxvm_socket_connect(context, op1R->int_value, op2R, op3R->int_value);
            DISPATCH;


)

RXVM_HANDLER(SOCKBIND_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SOCKBIND R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            rxvm_socket_bind(context, op1R->int_value, op2R, op3R->int_value);
            DISPATCH;


)

RXVM_HANDLER(SOCKLISTEN_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SOCKLISTEN R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            set_int(op1R, rxvm_socket_listen(context, op2R->int_value, op3R->int_value));
            DISPATCH;


)

RXVM_HANDLER(SOCKACCEPT_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - SOCKACCEPT R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            set_int(op1R, rxvm_socket_accept(context, op2R->int_value));
            DISPATCH;


)

RXVM_HANDLER(SOCKSHUTDOWN_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SOCKSHUTDOWN R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            set_int(op1R, rxvm_socket_shutdown(context, op2R->int_value, op3R->int_value));
            DISPATCH;


)

RXVM_HANDLER(SOCKSEND_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SOCKSEND R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            set_int(op1R, rxvm_socket_send_string(context, op2R->int_value, op3R));
            DISPATCH;


)

RXVM_HANDLER(SOCKSENDB_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SOCKSENDB R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            set_int(op1R, rxvm_socket_send_binary(context, op2R->int_value, op3R));
            DISPATCH;


)

RXVM_HANDLER(SOCKRECV_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SOCKRECV R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            rxvm_socket_recv_string(context, op1R, op2R->int_value, op3R->int_value);
            DISPATCH;


)

RXVM_HANDLER(SOCKRECVB_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SOCKRECVB R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            rxvm_socket_recv_binary(context, op1R, op2R->int_value, op3R->int_value);
            DISPATCH;


)

RXVM_HANDLER(SOCKPENDING_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - SOCKPENDING R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            set_int(op1R, rxvm_socket_pending(context, op2R->int_value));
            DISPATCH;


)

RXVM_HANDLER(SOCKTIMEOUT_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SOCKTIMEOUT R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            set_int(op1R, rxvm_socket_timeout(context, op2R->int_value, op3R->int_value));
            DISPATCH;


)

RXVM_HANDLER(SOCKBLOCKING_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SOCKBLOCKING R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            set_int(op1R, rxvm_socket_blocking(context, op2R->int_value, op3R->int_value));
            DISPATCH;


)

RXVM_HANDLER(SOCKNODELAY_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SOCKNODELAY R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            set_int(op1R, rxvm_socket_nodelay(context, op2R->int_value, op3R->int_value));
            DISPATCH;


)

RXVM_HANDLER(SOCKKEEPALIVE_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SOCKKEEPALIVE R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            set_int(op1R, rxvm_socket_keepalive(context, op2R->int_value, op3R->int_value));
            DISPATCH;


)

RXVM_HANDLER(SOCKPEER_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - SOCKPEER R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            rxvm_socket_peer(context, op1R, op2R->int_value);
            DISPATCH;


)

RXVM_HANDLER(SOCKLOCAL_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - SOCKLOCAL R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            rxvm_socket_local(context, op1R, op2R->int_value);
            DISPATCH;


)

RXVM_HANDLER(SOCKSTATUS_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - SOCKSTATUS R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            set_int(op1R, rxvm_socket_status(context, op2R->int_value));
            DISPATCH;


)

RXVM_HANDLER(SOCKERROR_REG_REG, VM_ADVANCE(2);
            DEBUG("TRACE - SOCKERROR R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            rxvm_socket_error(context, op1R, op2R->int_value);
            DISPATCH;


)

RXVM_HANDLER(SOCKSTARTTLS_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SOCKSTARTTLS R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            set_int(op1R, rxvm_socket_starttls(context, op2R->int_value, op3R));
            DISPATCH;


)

RXVM_HANDLER(SOCKCONNECTTLS_REG_REG_REG, VM_ADVANCE(3);
            DEBUG("TRACE - SOCKCONNECTTLS R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            rxvm_socket_connect_tls(context, op1R->int_value, op2R, op3R->int_value);
            DISPATCH;


)

RXVM_HANDLER(CNOP_REG_REG_REG_REG_REG_REG_REG_REG_REG, VM_ADVANCE(9);
            DEBUG("TRACE - CNOP R%lu,R%lu,R%lu,R%lu,R%lu,R%lu,R%lu,R%lu,R%lu\n",
                  REG_IDX(1), REG_IDX(2), REG_IDX(3), REG_IDX(4), REG_IDX(5),
                  REG_IDX(6), REG_IDX(7), REG_IDX(8), REG_IDX(9));
            DISPATCH;

)

RXVM_HANDLER(RESERVED_088,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_089,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_090,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_091,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_092,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_093,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_094,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_095,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_096,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_097,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_098,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_099,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_151,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_154,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_155,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_156,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_188,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_189,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_263,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)



RXVM_HANDLER(SWAPN_REG_REG_REG_REG, VM_ADVANCE(4);
            RXVM_SWAP_PAIR(1, 2);
            RXVM_SWAP_PAIR(3, 4);
            DISPATCH;


)

RXVM_HANDLER(SWAPN_REG_REG_REG_REG_REG_REG, VM_ADVANCE(6);
            RXVM_SWAP_PAIR(1, 2);
            RXVM_SWAP_PAIR(3, 4);
            RXVM_SWAP_PAIR(5, 6);
            DISPATCH;


)

RXVM_HANDLER(SWAPN_REG_REG_REG_REG_REG_REG_REG_REG, VM_ADVANCE(8);
            RXVM_SWAP_PAIR(1, 2);
            RXVM_SWAP_PAIR(3, 4);
            RXVM_SWAP_PAIR(5, 6);
            RXVM_SWAP_PAIR(7, 8);
            DISPATCH;


)

RXVM_HANDLER(SETTPSWAP_REG_INT_REG, VM_ADVANCE(3);
            REG_OP(1)->status.all_type_flags =
                    RXFLAGS_PUBLIC_WRITE(REG_OP(1)->status.all_type_flags,
                                         (uint32_t)INT_OP(2));
            RXVM_SWAP_PAIR(1, 3);
            DISPATCH;


)

RXVM_HANDLER(LOADSETTP2_REG_INT_REG_INT, VM_ADVANCE(4);
            set_int(REG_OP(1), INT_OP(2));
            REG_OP(3)->status.all_type_flags =
                    RXFLAGS_PUBLIC_WRITE(REG_OP(3)->status.all_type_flags,
                                         (uint32_t)INT_OP(4));
            DISPATCH;


)

RXVM_HANDLER(LOADSETTPSWAP_REG_INT_REG_INT_REG, VM_ADVANCE(5);
            set_int(REG_OP(1), INT_OP(2));
            REG_OP(3)->status.all_type_flags =
                    RXFLAGS_PUBLIC_WRITE(REG_OP(3)->status.all_type_flags,
                                         (uint32_t)INT_OP(4));
            RXVM_SWAP_PAIR(3, 5);
            DISPATCH;


)

RXVM_HANDLER(SWAPSETTP_REG_REG_REG_INT, VM_ADVANCE(4);
            RXVM_SWAP_PAIR(1, 2);
            REG_OP(3)->status.all_type_flags =
                    RXFLAGS_PUBLIC_WRITE(REG_OP(3)->status.all_type_flags,
                                         (uint32_t)INT_OP(4));
            DISPATCH;


)

RXVM_HANDLER(SWAPSETTPSWAP_REG_REG_REG_INT_REG, VM_ADVANCE(5);
            RXVM_SWAP_PAIR(1, 2);
            REG_OP(3)->status.all_type_flags =
                    RXFLAGS_PUBLIC_WRITE(REG_OP(3)->status.all_type_flags,
                                         (uint32_t)INT_OP(4));
            RXVM_SWAP_PAIR(3, 5);
            DISPATCH;


)

RXVM_HANDLER(RESERVED_272,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)



RXVM_HANDLER(SETTPSWAPSETTPSWAP_REG_INT_REG_REG_REG, VM_ADVANCE(5);
            REG_OP(1)->status.all_type_flags =
                    RXFLAGS_PUBLIC_WRITE(REG_OP(1)->status.all_type_flags,
                                         (uint32_t)INT_OP(2));
            RXVM_SWAP_PAIR(1, 3);
            REG_OP(4)->status.all_type_flags =
                    RXFLAGS_PUBLIC_WRITE(REG_OP(4)->status.all_type_flags,
                                         (uint32_t)INT_OP(2));
            RXVM_SWAP_PAIR(4, 5);
            DISPATCH;


)

RXVM_HANDLER(NULLN_REG_REG, VM_ADVANCE(2);
            value_zero(REG_OP(1));
            value_zero(REG_OP(2));
            DISPATCH;


)

RXVM_HANDLER(NULLN_REG_REG_REG, VM_ADVANCE(3);
            value_zero(REG_OP(1));
            value_zero(REG_OP(2));
            value_zero(REG_OP(3));
            DISPATCH;


)

RXVM_HANDLER(NULLN_REG_REG_REG_REG, VM_ADVANCE(4);
            value_zero(REG_OP(1));
            value_zero(REG_OP(2));
            value_zero(REG_OP(3));
            value_zero(REG_OP(4));
            DISPATCH;


)

RXVM_HANDLER(RESERVED_277,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_278,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_279,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)



RXVM_HANDLER(SWAPCALL_REG_FUNC_REG_REG_REG, VM_ADVANCE(5);
            RXVM_SWAP_PAIR(4, 5);
            RXVM_MAPPED_CALL_BODY(REG_OP(1), PROC_OP(2),
                                  REG_OP(3)->int_value, REG_IDX(3) + 1);
            DISPATCH;


)

RXVM_HANDLER(SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG, VM_ADVANCE(6);
            REG_OP(4)->status.all_type_flags =
                    RXFLAGS_PUBLIC_WRITE(REG_OP(4)->status.all_type_flags,
                                         (uint32_t)INT_OP(5));
            RXVM_SWAP_PAIR(4, 6);
            RXVM_MAPPED_CALL_BODY(REG_OP(1), PROC_OP(2),
                                  REG_OP(3)->int_value, REG_IDX(3) + 1);
            DISPATCH;


)

RXVM_HANDLER(SETTPCALL_REG_FUNC_REG_REG_INT, VM_ADVANCE(5);
            REG_OP(4)->status.all_type_flags =
                    RXFLAGS_PUBLIC_WRITE(REG_OP(4)->status.all_type_flags,
                                         (uint32_t)INT_OP(5));
            RXVM_MAPPED_CALL_BODY(REG_OP(1), PROC_OP(2),
                                  REG_OP(3)->int_value, REG_IDX(3) + 1);
            DISPATCH;


)

RXVM_HANDLER(UNLINKN_REG_REG, VM_ADVANCE(2);
            REG_OP(1) = current_frame->baselocals[REG_IDX(1)];
            REG_OP(2) = current_frame->baselocals[REG_IDX(2)];
            DISPATCH;


)

RXVM_HANDLER(ISETUNLINK_REG_REG, VM_ADVANCE(2);
            REG_OP(1)->int_value = REG_OP(2)->int_value;
            REG_OP(1) = current_frame->baselocals[REG_IDX(1)];
            DISPATCH;


)

RXVM_HANDLER(IGETUNLINK_REG_REG, VM_ADVANCE(2);
            {
                rxinteger saved_value = REG_OP(2)->int_value;
                REG_OP(2) = current_frame->baselocals[REG_IDX(2)];
                REG_OP(1)->int_value = saved_value;
            }
            DISPATCH;


)

RXVM_HANDLER(ILOADSETUNLINK_REG_INT, VM_ADVANCE(2);
            set_int(REG_OP(1), INT_OP(2));
            REG_OP(1) = current_frame->baselocals[REG_IDX(1)];
            DISPATCH;


)

RXVM_HANDLER(ISETUNLINKN_REG_REG_REG, VM_ADVANCE(3);
            REG_OP(1)->int_value = REG_OP(2)->int_value;
            REG_OP(1) = current_frame->baselocals[REG_IDX(1)];
            REG_OP(3) = current_frame->baselocals[REG_IDX(3)];
            DISPATCH;


)

RXVM_HANDLER(UNLINKBR_REG_ID,
            REG_OP(1) = current_frame->baselocals[REG_IDX(1)];
            VM_SELECT_INDEX((pc + 2)->index, RXVM_TRANSITION_BRANCH);
            DISPATCH;


)

RXVM_HANDLER(ILOADSETUNLINKN_REG_INT_REG, VM_ADVANCE(3);
            set_int(REG_OP(1), INT_OP(2));
            REG_OP(1) = current_frame->baselocals[REG_IDX(1)];
            REG_OP(3) = current_frame->baselocals[REG_IDX(3)];
            DISPATCH;


)

RXVM_HANDLER(RESERVED_290,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)



RXVM_HANDLER(SETLINKATTR1_REG_REG_INT_REG, VM_ADVANCE(4);
            set_num_attributes(REG_OP(2), INT_OP(3));
            if (REG_OP(4)->int_value < 1 ||
                REG_OP(4)->int_value > REG_OP(2)->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            REG_OP(1) = REG_OP(2)->attributes[REG_OP(4)->int_value - 1];
            DISPATCH;


)

RXVM_HANDLER(SETLINKATTR1_REG_REG_INT_REG_INT, VM_ADVANCE(5);
            {
                rxinteger index;
                set_num_attributes(REG_OP(2), INT_OP(3));
                if (!rxinteger_checked_add(REG_OP(4)->int_value, INT_OP(5), &index)) {
                    SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
                    DISPATCH;
                }
                if (index < 1 || index > REG_OP(2)->num_attributes) {
                    SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                    DISPATCH;
                }
                REG_OP(1) = REG_OP(2)->attributes[index - 1];
            }
            DISPATCH;


)

RXVM_HANDLER(MINLINKATTR1_REG_REG_INT, VM_ADVANCE(3);
            if (INT_OP(3) > REG_OP(2)->num_attributes) {
                set_num_attributes(REG_OP(2), INT_OP(3));
            }
            if (INT_OP(3) < 1 || INT_OP(3) > REG_OP(2)->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            REG_OP(1) = REG_OP(2)->attributes[INT_OP(3) - 1];
            DISPATCH;


)

RXVM_HANDLER(MINLINKATTR1_REG_REG_REG_INT, VM_ADVANCE(4);
            {
                rxinteger index;
                if (!rxinteger_checked_add(REG_OP(3)->int_value, INT_OP(4), &index)) {
                    SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
                    DISPATCH;
                }
                if (index > REG_OP(2)->num_attributes) {
                    set_num_attributes(REG_OP(2), index);
                }
                if (index < 1 || index > REG_OP(2)->num_attributes) {
                    SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                    DISPATCH;
                }
                REG_OP(1) = REG_OP(2)->attributes[index - 1];
            }
            DISPATCH;


)

RXVM_HANDLER(ITOF_REG_REG, VM_ADVANCE(2);
            REG_OP(1)->int_value = REG_OP(2)->int_value;
            REG_OP(1)->float_value = (double)REG_OP(2)->int_value;
            DISPATCH;


)

RXVM_HANDLER(STOI_REG_REG, VM_ADVANCE(2);
            if (string2integer(&REG_OP(1)->int_value,
                               REG_OP(2)->string_value,
                               REG_OP(2)->string_length)) {
                SET_SIGNAL(RXSIGNAL_CONVERSION_ERROR);
            }
            DISPATCH;


)

RXVM_HANDLER(FDIVSUB_REG_REG_REG_FLOAT, VM_ADVANCE(4);
            REG_OP(3)->float_value = REG_OP(2)->float_value / REG_OP(3)->float_value;
            REG_OP(1)->float_value = REG_OP(3)->float_value - FLOAT_OP(4);
            DISPATCH;


)

RXVM_HANDLER(FMULTICOPY_REG_FLOAT_REG_REG, VM_ADVANCE(4);
            REG_OP(1)->float_value *= FLOAT_OP(2);
            REG_OP(3)->int_value = REG_OP(4)->int_value;
            DISPATCH;


)

RXVM_HANDLER(RESERVED_299,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


#define RXVM_FIXED_CALL_BODY(ARITY_)                                           \
        do {                                                                    \
            proc_runtime *explicit_called__ = PROC_OP(2);                      \
            value *explicit_result__ = REG_OP(1);                              \
            value *explicit_args__[4];                                         \
            size_t explicit_i__;                                               \
            size_t explicit_j__;                                               \
            if ((ARITY_) > 0) explicit_args__[0] = REG_OP(3);                  \
            if ((ARITY_) > 1) explicit_args__[1] = REG_OP(4);                  \
            if ((ARITY_) > 2) explicit_args__[2] = REG_OP(5);                  \
            if ((ARITY_) > 3) explicit_args__[3] = REG_OP(6);                  \
            if (explicit_called__->start == SIZE_MAX) {                        \
                RXVM_INSTRUMENTATION_CALL(                                      \
                        RXVM_PROFILE_CALL_DIRECT_BYTECODE, explicit_called__,  \
                        (ARITY_), RXVM_PROFILE_FRAME_NONE_FAILED,               \
                        RXVM_PROFILE_CALL_UNRESOLVED, current_frame,            \
                        current_module->module_number, VM_CANONICAL_INDEX(pc),  \
                        0, 0);                                                  \
                SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, explicit_called__->name); \
                DISPATCH;                                                       \
            }                                                                   \
            if (!explicit_called__->binarySpace) {                             \
                RXVM_INSTRUMENTATION_CALL(                                      \
                        RXVM_PROFILE_CALL_DIRECT_NATIVE, explicit_called__,    \
                        (ARITY_), RXVM_PROFILE_FRAME_NONE_FAILED,               \
                        RXVM_PROFILE_CALL_INVALID, current_frame,               \
                        current_module->module_number, VM_CANONICAL_INDEX(pc),  \
                        0, 0);                                                  \
                SET_SIGNAL_MSG(RXSIGNAL_NOT_IMPLEMENTED,                       \
                               "fixed direct calls require a bytecode target"); \
                DISPATCH;                                                       \
            }                                                                   \
            temp_frame = frame_f(explicit_called__, (ARITY_), current_frame,   \
                                 next_pc, explicit_result__);                   \
            if (!temp_frame) {                                                  \
                RXVM_INSTRUMENTATION_CALL(                                      \
                        RXVM_PROFILE_CALL_DIRECT_BYTECODE, explicit_called__,  \
                        (ARITY_), RXVM_PROFILE_FRAME_NONE_FAILED,               \
                        RXVM_PROFILE_CALL_FRAME_FAILED, current_frame,          \
                        current_module->module_number, VM_CANONICAL_INDEX(pc),  \
                        0, 0);                                                  \
                SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Unable to allocate stack frame"); \
                DISPATCH;                                                       \
            }                                                                   \
            RXVM_INSTRUMENTATION_CALL(                                          \
                    RXVM_PROFILE_CALL_DIRECT_BYTECODE, explicit_called__,      \
                    temp_frame->number_args,                                   \
                    RXVM_PROFILE_FRAME_LAST_ACTIVATION,                        \
                    RXVM_PROFILE_CALL_SUCCESS, current_frame,                  \
                    current_module->module_number, VM_CANONICAL_INDEX(pc),      \
                    0, 0);                                                      \
            VM_ACTIVATE_FRAME(temp_frame, RXVM_TRANSITION_CALL);                \
            VM_SELECT_INDEX(explicit_called__->start, RXVM_TRANSITION_CALL);    \
            explicit_j__ = current_frame->procedure->binarySpace->globals +    \
                           current_frame->procedure->locals + 1;                \
            for (explicit_i__ = 0; explicit_i__ < (size_t)(ARITY_);            \
                 explicit_i__++, explicit_j__++) {                             \
                current_frame->baselocals[explicit_j__] =                      \
                        explicit_args__[explicit_i__];                         \
                current_locals[explicit_j__] =                                 \
                        current_frame->baselocals[explicit_j__];               \
            }                                                                   \
        } while (0)


RXVM_HANDLER(CALL1_REG_FUNC_REG, VM_ADVANCE(3);
            RXVM_FIXED_CALL_BODY(1);
            DISPATCH;


)

RXVM_HANDLER(CALL2_REG_FUNC_REG_REG, VM_ADVANCE(4);
            RXVM_FIXED_CALL_BODY(2);
            DISPATCH;


)

RXVM_HANDLER(CALL3_REG_FUNC_REG_REG_REG, VM_ADVANCE(5);
            RXVM_FIXED_CALL_BODY(3);
            DISPATCH;


)

RXVM_HANDLER(CALL4_REG_FUNC_REG_REG_REG_REG, VM_ADVANCE(6);
            RXVM_FIXED_CALL_BODY(4);
            DISPATCH;


)
#undef RXVM_FIXED_CALL_BODY

RXVM_HANDLER(PARSEWORDS3_REG_REG_REG_REG, VM_ADVANCE(4);
            {
                rxvm_parse_span parse_spans__[3];
                value *parse_source__ = REG_OP(4);
                char *parse_snapshot__ = 0;
                const char *parse_bytes__ = parse_source__->string_value;
                rxvm_parse_words3_spans(parse_source__, parse_spans__);
                if (op1R == parse_source__ || op2R == parse_source__ ||
                    op3R == parse_source__) {
                    parse_snapshot__ = rxvm_internal_alloc(
                        parse_source__->string_length > 0 ?
                        parse_source__->string_length : 1);
                    if (!parse_snapshot__) {
                        SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
                        DISPATCH;
                    }
                    if (parse_source__->string_length > 0) {
                        memcpy(parse_snapshot__, parse_source__->string_value,
                               parse_source__->string_length);
                    }
                    parse_bytes__ = parse_snapshot__;
                }
                rxvm_parse_set_span_bytes(op1R, parse_bytes__, &parse_spans__[0]);
                rxvm_parse_set_span_bytes(op2R, parse_bytes__, &parse_spans__[1]);
                rxvm_parse_set_span_bytes(op3R, parse_bytes__, &parse_spans__[2]);
                rxvm_internal_free(parse_snapshot__);
            }
            DISPATCH;


)

RXVM_HANDLER(ISETATTR1_REG_INT_REG, VM_ADVANCE(3);
            if (INT_OP(2) < 1 || INT_OP(2) > REG_OP(1)->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            REG_OP(1)->attributes[INT_OP(2) - 1]->int_value = REG_OP(3)->int_value;
            DISPATCH;


)

RXVM_HANDLER(RESERVED_407,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)



RXVM_HANDLER(PARSEPOS2_REG_REG_REG_INT, VM_ADVANCE(4);
            {
                rxvm_parse_span parse_spans__[2];
                value *parse_source__ = op3R;
                char *parse_snapshot__ = 0;
                const char *parse_bytes__ = parse_source__->string_value;
                rxvm_parse_position2_spans(parse_source__, INT_OP(4), parse_spans__);
                if (op1R == parse_source__ || op2R == parse_source__) {
                    parse_snapshot__ = rxvm_internal_alloc(
                        parse_source__->string_length > 0 ?
                        parse_source__->string_length : 1);
                    if (!parse_snapshot__) {
                        SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
                        DISPATCH;
                    }
                    if (parse_source__->string_length > 0) {
                        memcpy(parse_snapshot__, parse_source__->string_value,
                               parse_source__->string_length);
                    }
                    parse_bytes__ = parse_snapshot__;
                }
                rxvm_parse_set_span_bytes(op1R, parse_bytes__, &parse_spans__[0]);
                rxvm_parse_set_span_bytes(op2R, parse_bytes__, &parse_spans__[1]);
                rxvm_internal_free(parse_snapshot__);
            }
            DISPATCH;


)

RXVM_HANDLER(PARSEWORDS3D_REG_REG_REG_REG, VM_ADVANCE(4);
            {
                rxvm_parse_span parse_spans__[3];
                value *parse_source__ = REG_OP(4);
                char *parse_snapshot__ = 0;
                const char *parse_bytes__ = parse_source__->string_value;
                rxvm_parse_words3_drop_spans(parse_source__, parse_spans__);
                if (op1R == parse_source__ || op2R == parse_source__ ||
                    op3R == parse_source__) {
                    parse_snapshot__ = rxvm_internal_alloc(
                        parse_source__->string_length > 0 ?
                        parse_source__->string_length : 1);
                    if (!parse_snapshot__) {
                        SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
                        DISPATCH;
                    }
                    if (parse_source__->string_length > 0) {
                        memcpy(parse_snapshot__, parse_source__->string_value,
                               parse_source__->string_length);
                    }
                    parse_bytes__ = parse_snapshot__;
                }
                rxvm_parse_set_span_bytes(op1R, parse_bytes__, &parse_spans__[0]);
                rxvm_parse_set_span_bytes(op2R, parse_bytes__, &parse_spans__[1]);
                rxvm_parse_set_span_bytes(op3R, parse_bytes__, &parse_spans__[2]);
                rxvm_internal_free(parse_snapshot__);
            }
            DISPATCH;

)

RXVM_HANDLER(PARSEPLAN_REG_REG_STRING, VM_ADVANCE(3);
            {
                int parse_result__ = rxvm_parse_plan_execute(op1R, op2R, op3S);
                if (parse_result__ == RXVM_PARSE_PLAN_CONVERSION) {
                    SET_SIGNAL_MSG(RXSIGNAL_CONVERSION_ERROR,
                                   "Dynamic PARSE position is not a nonnegative native integer");
                } else if (parse_result__ != RXVM_PARSE_PLAN_OK) {
                    SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS,
                                   "Invalid compact compiled PARSE descriptor");
                }
            }
            DISPATCH;

)

RXVM_HANDLER(RESERVED_411,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)



RXVM_HANDLER(LINKSETATTRSLINKADD_REG_REG_INT_INT_REG_REG_INT, VM_ADVANCE(7);
            {
                rxinteger index;
                if (INT_OP(3) < 1 || INT_OP(3) > REG_OP(2)->num_attributes) {
                    SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                    DISPATCH;
                }
                REG_OP(1) = REG_OP(2)->attributes[INT_OP(3) - 1];
                set_num_attributes(REG_OP(1), INT_OP(4));
                if (!rxinteger_checked_add(REG_OP(6)->int_value, INT_OP(7), &index)) {
                    SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
                    DISPATCH;
                }
                if (index < 1 || index > REG_OP(1)->num_attributes) {
                    SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                    DISPATCH;
                }
                REG_OP(5) = REG_OP(1)->attributes[index - 1];
            }
            DISPATCH;


)

RXVM_HANDLER(RESERVED_413,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_414,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)



RXVM_HANDLER(SETLINKILOAD_REG_REG_INT_REG_REG_INT, VM_ADVANCE(6);
            set_num_attributes(REG_OP(2), INT_OP(3));
            if (REG_OP(4)->int_value < 1 ||
                REG_OP(4)->int_value > REG_OP(2)->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH;
            }
            REG_OP(1) = REG_OP(2)->attributes[REG_OP(4)->int_value - 1];
            set_int(REG_OP(5), INT_OP(6));
            DISPATCH;


)

RXVM_HANDLER(RESERVED_416,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_417,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_418,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_419,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_420,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_421,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)



RXVM_HANDLER(ILOADSETUNLINKN_REG_REG_INT_REG, VM_ADVANCE(4);
            set_int(REG_OP(1), INT_OP(3));
            REG_OP(2)->int_value = REG_OP(1)->int_value;
            REG_OP(2) = current_frame->baselocals[REG_IDX(2)];
            REG_OP(4) = current_frame->baselocals[REG_IDX(4)];
            DISPATCH;


)

RXVM_HANDLER(RESERVED_423,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_424,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_425,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_426,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_427,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_428,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_429,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_430,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_431,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_432,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_433,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_434,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_435,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_436,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_437,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_438,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_439,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_440,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_441,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_442,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_443,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_444,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_445,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_446,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_447,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_448,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)


RXVM_HANDLER(RESERVED_449,
    SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
    DISPATCH;
)
