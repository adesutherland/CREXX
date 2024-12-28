# Matrix Operations Plugin for CREXX/PA

## Overview
This plugin provides matrix operations for the CREXX/PA system. It supports basic matrix operations including multiplication, transposition, inversion, and standardization.

## Functions

### Matrix Creation and Management
- **mcreate(rows, cols, id)** → matrix_number
  - Creates a new matrix with specified dimensions
  - Parameters:
    - rows: Number of rows (integer)
    - cols: Number of columns (integer)
    - id: Matrix identifier (string)
  - Returns: Matrix number or error code
  - Error codes:
    - -1: Invalid input parameters
    - -12: No matrix slots available
    - -16: Unable to allocate matrix structure
    - -20: Unable to allocate matrix data

- **mfree(matrix_number)** → status
  - Frees a matrix or all matrices if matrix_number < 0
  - Parameters:
    - matrix_number: Matrix to free, or -1 to free all
  - Returns: 0 on success, error code on failure

### Matrix Operations
- **mmultiply(m1, m2, id)** → matrix_number
  - Multiplies two matrices
  - Parameters:
    - m1: First matrix number
    - m2: Second matrix number
    - id: Result matrix identifier
  - Returns: New matrix number or error code

- **mprod(m, scalar, id)** → matrix_number
  - Multiplies matrix by scalar value
  - Parameters:
    - m: Matrix number
    - scalar: Multiplication factor (float)
    - id: Result matrix identifier
  - Returns: New matrix number or error code

- **minvert(m, id)** → matrix_number
  - Inverts a square matrix
  - Parameters:
    - m: Matrix number
    - id: Result matrix identifier
  - Returns: New matrix number or error code
  - Note: Matrix must be square and non-singular

- **mtranspose(m, id)** → matrix_number
  - Transposes a matrix
  - Parameters:
    - m: Matrix number
    - id: Result matrix identifier
  - Returns: New matrix number or error code

- **mstandard(m, id)** → matrix_number
  - Standardizes matrix columns (z-score transformation)
  - Parameters:
    - m: Matrix number
    - id: Result matrix identifier
  - Returns: New matrix number or error code

### Matrix Element Access
- **mset(m, row, col, value)** → status
  - Sets a matrix element value
  - Parameters:
    - m: Matrix number
    - row: Row number (1-based)
    - col: Column number (1-based)
    - value: New value (float)
  - Returns: 0 on success, error code on failure

- **mprint(m)** → status
  - Prints matrix contents
  - Parameters:
    - m: Matrix number
  - Returns: 0 on success, error code on failure

## Error Codes
- MATRIX_SUCCESS (0): Operation successful
- MATRIX_INVALID_PARAM (-1): Invalid parameters
- MATRIX_NO_SLOTS (-12): No available matrix slots
- MATRIX_ALLOC_CB (-16): Control block allocation failed
- MATRIX_ALLOC_DATA (-20): Data allocation failed
- MATRIX_NULL (-30): Matrix is NULL
- MATRIX_INVALID_INDEX (-31): Invalid matrix index
- MATRIX_CORRUPT (-32): Matrix structure is corrupt

## Example Usage

/ Create and initialize a matrix /
m1 = mcreate(3, 3, "Test Matrix")
call mset m1, 1, 1, 1.0
call mset m1, 2, 2, 1.0
call mset m1, 3, 3, 1.0
/ Print the matrix /
call mprint m1
/ Create transpose /
m2 = mtranspose(m1, "Transposed")
call mprint m2
/ Multiply by scalar /
m3 = mprod(m1, 2.0, "Doubled")
call mprint m3
/ Free resources /
call mfree m1
call mfree m2
call mfree m3

## Implementation Notes
- Matrix indices are 1-based in REXX calls
- All matrices are stored in double precision
- Maximum number of matrices: 100
- Memory is managed through Windows HeapAlloc
- Matrix operations validate inputs before processing

## Error Handling
All functions return error codes for invalid operations. Check return values for:
- Invalid matrix numbers
- Memory allocation failures
- Invalid dimensions
- Corrupt matrix structures

