# VM Memory Audit: Registers and Stack Frames

## 1. Life of a Register (Allocation -> Assignment -> Cleanup)

### Allocation & Initialization
- **Storage**: `Value` structures are allocated as part of larger blocks:
    - **Local Registers**: Allocated in a contiguous block within a `stack_frame`.
    - **Global Registers**: Allocated in a contiguous block within a `module_constant`.
    - **Work Registers**: Allocated on the VM stack or as individual variables in the `run()` loop (e.g., `work1`, `signal_value`).
- **Initialization**: Every `Value` is initialized via `value_init()`:
    - Sets `string_value` to an internal `small_string_buffer` (16 bytes).
    - Sets `string_buffer_length` to 16.
    - Sets `string_length` to 0.
    - Status flags are cleared.

### Assignment & Growth
- **Buffer Management**: `prep_string_buffer()` and `extend_string_buffer()` handle growth.
    - If a string exceeds the current `string_buffer_length`, a new buffer is `malloc`ed.
    - If the current `string_value` is NOT the `small_string_buffer`, the old buffer is `free`d before the new `malloc`.
- **Copy vs. Link**: 
    - **Assignment**: Most operations (`set_string`, `copy_value`, `set_value_string`) perform a **Deep Copy** (malloc + memcpy).
    - **Argument Passing**: Procedure calls (`CALL_FUNC`, etc.) **Link** registers. The callee's `locals` pointers are updated to point to the caller's `Value` objects. The frame's original `Value` objects are shadowed but preserved in `baselocals`.
    - **Return Values**: Values are **Deep Copied** back to the caller's return register using `copy_value` or `move_value`.

### Cleanup (The "Lazy Free" Pattern)
- **`value_zero()`**: Resets `string_length` to 0 but **does NOT free** the malloced buffer. This allows for buffer reuse within the same register life.
- **`clear_value()`**: Explicitly `free`s the malloced `string_value` (if not the small buffer) and other resources (decimal, binary, attributes).
- **Final Deallocation**: `Value` structures themselves are freed only when their container (Frame, Module, or `run` loop) is destroyed.

## 2. Life of a Frame (Cache -> Use -> Recycle)

### Allocation & Caching
- **`frame_f()`**: Retrieves a frame for a procedure.
    - First checks the procedure's `frame_free_list` (a pool of cached frames).
    - If empty, `malloc`s a new `stack_frame` and its associated `Value` registers.
- **`free_frame()`**: Returns a frame to the `frame_free_list`.
    - **Critical**: It does **NOT** call `clear_value` or `value_zero` on the registers. The buffers and data persist in the cached frame.

### Recycling & Stale State
- When a frame is reused by `frame_f()`:
    - The `locals` pointers are restored to point to `baselocals` (reversing any argument linking).
    - **Optimization Risk**: Registers are only zeroed if `SAFE_RECYCLED_STACKFRAMES` is defined (it is currently **NOT** defined in the standard build).
    - This means a reused frame contains **stale pointers and buffers** from its previous use.

## 3. Global State & Shared Values
- **Module Globals**: Persistent throughout the VM's life. Shared across all procedures within a module.
- **Work Registers**: `work1`, `work2`, `work3`, and `signal_value` are local to the `run()` interpreter loop and are `clear_value`'d and `free`'d upon VM exit.
- **Interrupt Objects**: An array of `Value` objects used for signals, also cleared at exit.
