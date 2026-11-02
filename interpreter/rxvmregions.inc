/* Cold logical-region departure support. The immutable indexes are prepared
 * by the loader. No region state is pushed at entry or tested by dispatch. */

typedef struct rxvm_region_departure {
    const rxvm_inline_region *source;
    const rxvm_inline_region *destination;
    size_t source_address;
    numeric_context numeric;
    int numeric_changed;
} rxvm_region_departure;

static const rxvm_inline_region *rxvm_region_at(
        const stack_frame *frame, size_t address) {
    const module *mod = frame->procedure->binarySpace->module;
    const rxvm_inline_region *found = NULL;
    size_t index;
    for (index = 0u; index < mod->inline_range_count; ++index) {
        const rxvm_inline_range *range = &mod->inline_ranges[index];
        if (range->definition->start > address) break;
        if (address < range->definition->end &&
            rxvm_private_binding_owner(range->region->procedure) ==
                    rxvm_private_binding_owner(frame->procedure) &&
            (!found || found->depth < range->region->depth))
            found = range->region;
    }
    return found;
}

/* Return PCs identify the cell after a CALL, possibly a region's endpoint.
 * Recover and validate its canonical instruction, never use diagnostic origin
 * overrides as executable ownership. This scan is signal-path work only. */
static int rxvm_region_callsite(const stack_frame *child, size_t *address) {
    const stack_frame *parent = child->parent;
    const module *mod;
    size_t site, next, continuation;
    if (!parent || !parent->procedure || !parent->procedure->binarySpace) return 0;
    mod = parent->procedure->binarySpace->module;
    if (child->signal_parent_address != SIZE_MAX) {
        if (!runtime_procedure_contains(mod, parent->procedure, child->signal_parent_address)) return 0;
        *address = child->signal_parent_address;
        return 1;
    }
    if (child->invocation) {
        int64_t module_number, origin;
        rxvm_invoke_origin(child->invocation, &module_number, &origin);
        if (origin < 0 || module_number != (int64_t)mod->module_number) return 0;
        for (site = parent->procedure->start; site < mod->segment.inst_size; site = next) {
            size_t width = (size_t)mod->segment.binary[site].instruction.no_ops + 1u;
            if (width > mod->segment.inst_size - site) return 0;
            next = site + width;
            if (site == (size_t)origin) {
                if (!runtime_procedure_contains(mod, parent->procedure, site)) return 0;
                *address = site;
                return 1;
            }
            if (site > (size_t)origin) return 0;
        }
        return 0;
    }
    {
        uintptr_t base = (uintptr_t)mod->execution_image;
        uintptr_t returned = (uintptr_t)child->return_pc;
        size_t bytes;
        if (!base || returned <= base) return 0;
        bytes = (size_t)(returned - base);
        if (bytes % sizeof(bin_code) || bytes / sizeof(bin_code) > mod->segment.inst_size) return 0;
        continuation = bytes / sizeof(bin_code);
    }
    for (site = parent->procedure->start; site < continuation; site = next) {
        size_t width = (size_t)mod->segment.binary[site].instruction.no_ops + 1u;
        if (width > mod->segment.inst_size - site) return 0;
        next = site + width;
        if (next == continuation) {
            /* return_pc is captured by the real VM call owner. Verify its
             * exact canonical instruction boundary, without importing the
             * assembler's analysis implementation into the executor. */
            if (!runtime_procedure_contains(mod, parent->procedure, site)) return 0;
            *address = site;
            return 1;
        }
        if (next > continuation) return 0;
    }
    return 0;
}

static int rxvm_region_slot(const stack_frame *frame, uint32_t bank,
                            uint32_t index, size_t *slot) {
    size_t globals = (size_t)frame->procedure->binarySpace->globals;
    size_t locals = (size_t)frame->procedure->locals;
    switch (bank) {
        case RXBIN_INLINE_BANK_R:
            if ((size_t)index >= locals) return 0;
            *slot = index;
            break;
        case RXBIN_INLINE_BANK_G:
            if ((size_t)index >= globals) return 0;
            *slot = locals + index;
            break;
        case RXBIN_INLINE_BANK_A:
            if ((size_t)index > frame->number_args) return 0;
            *slot = locals + globals + index;
            break;
        default: return 0;
    }
    return *slot < frame->number_locals && frame->locals[*slot] && frame->baselocals[*slot];
}

static int rxvm_region_numeric(numeric_context *numeric, uint32_t field,
                               rxinteger value) {
    if (value < 0 || value > INT_MAX) return 0;
    switch (field) {
        case RXOP_CONTEXT_DIGITS:
            if (!value) return 0;
            numeric->digits = (int)value;
            return 1;
        case RXOP_CONTEXT_FUZZ: numeric->fuzz = (int)value; return 1;
        case RXOP_CONTEXT_FORM:
            if (value != NUMERIC_FORM_SCIENTIFIC && value != NUMERIC_FORM_ENGINEERING) return 0;
            numeric->form = (numeric_form)value;
            return 1;
        case RXOP_CONTEXT_CASE:
            if (value != CASE_LOWER && value != CASE_UPPER) return 0;
            numeric->casetype = (case_type)value;
            return 1;
        case RXOP_CONTEXT_STANDARD:
            if (value != NUMERIC_STANDARD_COMMON && value != NUMERIC_STANDARD_CLASSIC) return 0;
            numeric->standard = (numeric_standard)value;
            return 1;
        default: return 0;
    }
}

static int rxvm_region_departure_prepare(
        stack_frame *current, stack_frame *target, size_t actual_module,
        size_t actual_address, size_t destination, rxvm_region_departure *departure) {
    const module *mod;
    const stack_frame *child;
    const rxvm_inline_region *region;
    size_t index;
    memset(departure, 0, sizeof(*departure));
    if (!target || !target->procedure || !target->procedure->binarySpace) return 0;
    mod = target->procedure->binarySpace->module;
    if (!mod || !runtime_procedure_contains(mod, target->procedure, destination)) return 0;
    if (!mod->inline_region_count) return 1;
    departure->numeric = target->num_context;
    if (current == target) {
        if (actual_module != mod->module_number ||
            !runtime_procedure_contains(mod, target->procedure, actual_address)) return 0;
        departure->source_address = actual_address;
    } else {
        for (child = current; child && child->parent != target; child = child->parent) {}
        if (!child || !rxvm_region_callsite(child, &departure->source_address)) return 0;
    }
    departure->source = rxvm_region_at(target, departure->source_address);
    departure->destination = rxvm_region_at(target, destination);
    region = departure->source;
    while (region && region != departure->destination) region = region->parent;
    if (region != departure->destination) return 0; /* inactive sibling/descendant */
    for (region = departure->source; region != departure->destination; region = region->parent) {
        for (index = region->first_action; index < region->first_action + region->action_count; ++index) {
            const meta_inline_exit_constant *action = mod->inline_exits[index].definition;
            size_t first, second;
            rxinteger value;
            if (departure->source_address < action->start || departure->source_address >= action->end)
                continue;
            if (action->kind == RXBIN_INLINE_ACTION_NUMERIC) {
                value = action->literal;
                if (action->source == RXBIN_INLINE_SOURCE_OWNED_BASE) {
                    const struct value *saved;
                    if (action->index1 >= (uint32_t)rxvm_owned_local_count(target->procedure)) return 0;
                    saved = target->baselocals[action->index1];
                    if (!saved) return 0;
                    value = saved->int_value;
                }
                if (!rxvm_region_numeric(&departure->numeric, action->field, value)) return 0;
                departure->numeric_changed = 1;
            } else if (action->kind == RXBIN_INLINE_ACTION_ENDLIFE) {
                if (action->index1 >= (uint32_t)rxvm_owned_local_count(target->procedure) ||
                    !target->baselocals[action->index1]) return 0;
            } else {
                if (!rxvm_region_slot(target, action->bank1, action->index1, &first)) return 0;
                if (action->kind == RXBIN_INLINE_ACTION_SWAP || action->kind == RXBIN_INLINE_ACTION_LINK) {
                    if (!rxvm_region_slot(target, action->bank2, action->index2, &second)) return 0;
                } else if (action->kind != RXBIN_INLINE_ACTION_UNLINK) return 0;
            }
        }
    }
    /* The RXAS setters publish independent field bounds. In particular FUZZ
     * may be >= DIGITS; a source-language restriction is not a VM exit rule. */
    return 1;
}

/* All sources and bounds have been checked before real-frame or policy
 * retirement. ENDLIFE and pointer actions neither allocate nor call Rexx. */
static void rxvm_region_departure_apply(stack_frame *frame,
                                       const rxvm_region_departure *departure) {
    const module *mod = frame->procedure->binarySpace->module;
    const rxvm_inline_region *region;
    size_t index;
    for (region = departure->source; region != departure->destination; region = region->parent) {
        for (index = region->first_action; index < region->first_action + region->action_count; ++index) {
            const meta_inline_exit_constant *action = mod->inline_exits[index].definition;
            size_t first = 0u, second = 0u;
            if (departure->source_address < action->start || departure->source_address >= action->end)
                continue;
            if (action->kind == RXBIN_INLINE_ACTION_ENDLIFE) {
                release_value_reference_lifetime(frame->baselocals[action->index1]);
            } else if (action->kind != RXBIN_INLINE_ACTION_NUMERIC) {
                (void)rxvm_region_slot(frame, action->bank1, action->index1, &first);
                if (action->kind == RXBIN_INLINE_ACTION_UNLINK)
                    frame->locals[first] = frame->baselocals[first];
                else {
                    (void)rxvm_region_slot(frame, action->bank2, action->index2, &second);
                    if (action->kind == RXBIN_INLINE_ACTION_LINK)
                        frame->locals[first] = frame->locals[second];
                    else {
                        struct value *saved = frame->locals[first];
                        uint32_t identity = frame->local_identity[first];
                        frame->locals[first] = frame->locals[second];
                        frame->locals[second] = saved;
                        frame->local_identity[first] = frame->local_identity[second];
                        frame->local_identity[second] = identity;
                    }
                }
            }
        }
    }
    if (departure->numeric_changed) {
        frame->num_context = departure->numeric;
        rxvm_restore_frame_numeric_context(frame);
    }
}
