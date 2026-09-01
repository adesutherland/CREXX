/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2026 the cREXX contributors
 */

/* Gate F channel RXAS handlers.  Every provider call is synchronous with
 * respect to the controller instruction, so inputs can be consumed before
 * output registers are replaced.  This also preserves the documented
 * input/output aliasing semantics; the assembler rejects only duplicate
 * output registers. */

RXVM_HANDLER(CHANOPEN_REG_REG_REG_REG_REG, VM_ADVANCE(5);
    DEBUG("TRACE - CHANOPEN R%lu,R%lu,R%lu,R%lu,R%lu\n",
          REG_IDX(1), REG_IDX(2), REG_IDX(3), REG_IDX(4), REG_IDX(5));
    {
        int64_t channel = 0;
        rxvm_channel_status status = rxvm_channel_open(
                context, (int64_t)op3R->int_value,
                (int64_t)op4R->int_value,
                REG_OP(5)->binary_value, REG_OP(5)->binary_length, &channel);
        set_int(op2R, (rxinteger)channel);
        set_int(op1R, (rxinteger)status);
    }
    DISPATCH;
)

RXVM_HANDLER(CHANSTART_REG_REG_REG_REG_REG, VM_ADVANCE(5);
    DEBUG("TRACE - CHANSTART R%lu,R%lu,R%lu,R%lu,R%lu\n",
          REG_IDX(1), REG_IDX(2), REG_IDX(3), REG_IDX(4), REG_IDX(5));
    {
        int64_t ticket = 0;
        rxvm_channel_status status = rxvm_channel_start(
                context, (int64_t)op3R->int_value,
                op4R->binary_value, op4R->binary_length,
                (int64_t)REG_OP(5)->int_value, &ticket);
        set_int(op2R, (rxinteger)ticket);
        set_int(op1R, (rxinteger)status);
    }
    DISPATCH;
)

RXVM_HANDLER(CHANWAIT_REG_REG_REG_REG, VM_ADVANCE(4);
    DEBUG("TRACE - CHANWAIT R%lu,R%lu,R%lu,R%lu\n",
          REG_IDX(1), REG_IDX(2), REG_IDX(3), REG_IDX(4));
    {
        rxvm_channel_binary completion = {0, 0, 0};
        rxvm_channel_status status = rxvm_channel_wait(
                context, (int64_t)op3R->int_value,
                (int64_t)op4R->int_value, &completion);
        if (status == RXVM_CHANNEL_OK) {
            (void)set_buffer_binary(op2R, (char *)completion.data,
                                    completion.length, completion.capacity);
            completion.data = 0;
            completion.length = 0u;
            completion.capacity = 0u;
        } else {
            (void)set_binary(op2R, 0, 0);
        }
        rxvm_channel_binary_free(&completion);
        set_int(op1R, (rxinteger)status);
    }
    DISPATCH;
)

RXVM_HANDLER(CHANCANCEL_REG_REG_REG_REG, VM_ADVANCE(4);
    DEBUG("TRACE - CHANCANCEL R%lu,R%lu,R%lu,R%lu\n",
          REG_IDX(1), REG_IDX(2), REG_IDX(3), REG_IDX(4));
    {
        rxvm_channel_status status = rxvm_channel_cancel(
                context, (int64_t)op2R->int_value,
                (int64_t)op3R->int_value,
                op4R->binary_value, op4R->binary_length);
        set_int(op1R, (rxinteger)status);
    }
    DISPATCH;
)

RXVM_HANDLER(CHANCLOSE_REG_REG_REG, VM_ADVANCE(3);
    DEBUG("TRACE - CHANCLOSE R%lu,R%lu,R%lu\n",
          REG_IDX(1), REG_IDX(2), REG_IDX(3));
    {
        rxvm_channel_status status = rxvm_channel_close(
                context, (int64_t)op2R->int_value,
                (int64_t)op3R->int_value);
        set_int(op1R, (rxinteger)status);
    }
    DISPATCH;
)
