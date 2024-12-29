# Matrix Operations Plugin for CREXX/PA

## Overview
This plugin provides comprehensive matrix operations for the CREXX/PA system, including linear algebra and statistical functions.

## Memory Management
- Cross-platform heap storage using MATRIX_ALLOC/MATRIX_FREE macros
- Windows: HeapAlloc/HeapFree from process heap
- Unix/Linux/macOS: malloc/free from C standard library
- Maximum matrices: 100
- Automatic cleanup on matrix free or plugin unload

## Functions

### Basic Operations
- `mmultiply(m0, m1, mid)` - Matrix multiplication (m0 × m1 → mid)
- `mprod(m0, scalar, mid)` - Scalar multiplication (m0 × scalar → mid)
- `mtranspose(m0, mid)` - Matrix transpose (m0 → mid)
- `minvert(m0, mid)` - Matrix inversion (m0 → mid)

### Matrix Creation and Management
- `mcreate(rows, cols, id)` - Create new matrix
- `mset(m0, row, col, value)` - Set matrix element
- `mprint(m0)` - Print matrix
- `mfree(m0)` - Free matrix (if m0 < 0, frees all)

### Linear Algebra
- `mdet(m0)` - Calculate determinant
- `mlu(m0, L, U)` - LU decomposition
- `mqr(m0, Q, R)` - QR decomposition
- `meigen(m0, vec)` - Calculate dominant eigenvalue/vector
- `mrank(m0)` - Calculate matrix rank

### Statistical Operations
- `mcov(m0, mid)` - Covariance matrix
- `mcorr(m0, mid)` - Correlation matrix
- `mmean(m0, axis, mid)` - Row/column means (axis: 0=rows, 1=cols)
- `mstandard(m0, mid)` - Standardize matrix columns

## Performance Features
- Cache-friendly blocking for matrix operations
- Block size optimization (MATRIX_BLOCK_SIZE = 64)
- Loop unrolling for better instruction pipelining
- Efficient memory access patterns
- Numerical stability checks

## Error Handling
- Input validation
- Memory allocation checks
- Numerical stability checks
- Detailed error codes

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
```
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
```
```
rexx
/ Create and multiply two matrices /
m1 = matrix.mcreate(3, 3, "Matrix1")
m2 = matrix.mcreate(3, 3, "Matrix2")
matrix.mset(m1, 1, 1, 2.0) / Set element at row 1, col 1 /
result = matrix.mmultiply(m1, m2, "Result")
matrix.mprint(result)
matrix.mfree(result)
```

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

## Return Codes
- 0: Success
- -1: Invalid parameter
- -2: Memory allocation failure
- -3: Singular matrix
- -4: Dimension mismatch
- -8: Memory free error

## Notes
- All indices are 1-based for REXX compatibility
- Matrices are stored in row-major order
- Double precision floating point used throughout

## Detailed Error Codes

### Operation Status Codes
- MATRIX_SUCCESS (0): Operation completed successfully
- MATRIX_INVALID_PARAM (-1): Invalid input parameters
  - Matrix dimensions don't match
  - Invalid matrix number
  - NULL matrix identifier
  - Invalid row/column indices

### Memory Related Errors
- MATRIX_NO_SLOTS (-12): No available matrix slots
  - Maximum number of matrices (100) reached
  - Need to free some matrices before creating new ones
- MATRIX_ALLOC_CB (-16): Control block allocation failed
  - System memory allocation error
  - Not enough memory for matrix structure
- MATRIX_ALLOC_DATA (-20): Data allocation failed
  - Not enough memory for matrix data
  - System memory allocation error

### Matrix Validation Errors
- MATRIX_NULL (-30): Matrix is NULL
  - Attempting to operate on a NULL matrix
  - Matrix may have been freed
- MATRIX_INVALID_INDEX (-31): Invalid matrix index
  - Matrix number out of range (0-99)
  - Negative matrix number
- MATRIX_CORRUPT (-32): Matrix structure is corrupt
  - Invalid internal pointers
  - Memory corruption detected

### Operation-Specific Errors
- MATRIX_SINGULAR (-40): Matrix is singular
  - Determinant is zero
  - Matrix cannot be inverted
- MATRIX_DIM_MISMATCH (-41): Matrix dimensions mismatch
  - Incompatible dimensions for operation
  - Example: multiplication of incompatible matrices
- MATRIX_NOT_SQUARE (-42): Matrix must be square
  - Operation requires square matrix
  - Example: LU decomposition
- MATRIX_NO_CONVERGENCE (-43): Algorithm did not converge
  - Maximum iterations reached
  - Example: eigenvalue calculation

### Memory Management Errors
- MATRIX_FREE_ERROR (-8): Error freeing matrix memory
  - System memory deallocation error
  - Invalid memory block
- MATRIX_FREE_CB_ERROR (-12): Error freeing control block
  - System memory deallocation error
  - Invalid control block pointer


### Linear Algebra Examples
rexx
/ LU Decomposition /
m = matrix.mcreate(2, 2, "Original")
matrix.mset(m, 1, 1, 4.0)
matrix.mset(m, 1, 2, 3.0)
matrix.mset(m, 2, 1, 6.0)
matrix.mset(m, 2, 2, 3.0)
L = matrix.mlu(m, "L", "U") / Returns L, U is L+1 /
matrix.mprint(L)
matrix.mprint(L+1)
/ Matrix Rank /
rank = matrix.mrank(m)
say "Matrix rank:" rank
/ Eigenvalue /
ev = matrix.meigen(m, "eigenvector")
matrix.mprint(ev) / First element is eigenvalue, rest is eigenvector /

### Statistical Analysis
rexx:matrix.md
/ Create data matrix /
data = matrix.mcreate(4, 3, "DataSet")
/ Row 1 /
matrix.mset(data, 1, 1, 1.2)
matrix.mset(data, 1, 2, 2.3)
matrix.mset(data, 1, 3, 3.4)
/ Row 2 /
matrix.mset(data, 2, 1, 2.3)
matrix.mset(data, 2, 2, 3.4)
matrix.mset(data, 2, 3, 4.5)
/ Row 3 /
matrix.mset(data, 3, 1, 3.4)
matrix.mset(data, 3, 2, 4.5)
matrix.mset(data, 3, 3, 5.6)
/ Row 4 /
matrix.mset(data, 4, 1, 4.5)
matrix.mset(data, 4, 2, 5.6)
matrix.mset(data, 4, 3, 6.7)
/ Calculate statistics /
means = matrix.mmean(data, 1, "ColumnMeans") / Column means /
covar = matrix.mcov(data, "CovMatrix") / Covariance matrix /
corr = matrix.mcorr(data, "CorrMatrix") / Correlation matrix /
matrix.mprint(means)
matrix.mprint(covar)
matrix.mprint(corr)

### Advanced Statistical Analysis
- `mfactor(m0, factors, mid)` - Factor analysis
  - Parameters:
    - m0: Input data matrix
    - factors: Number of factors to extract
    - mid: Result matrix identifier for factor loadings
  - Returns: Matrix number containing factor loadings or error code
  - Example:
    ```
    data = matrix.mcreate(100, 5, "DataMatrix")  /* 100 observations, 5 variables */
    /* ... fill data matrix ... */
    loadings = matrix.mfactor(data, 2, "Factors")  /* Extract 2 factors */
    matrix.mprint(loadings)  /* Show factor loadings */
    ```

### Statistical Functions
- `mcolstats(m0, mid)` - Comprehensive column statistics
  - Returns matrix with 5 rows:
    1. Means
    2. Standard deviations
    3. Medians
    4. Skewness
    5. Kurtosis
  - Example:
    ```
    data = matrix.mcreate(100, 4, "Data")  /* 100 observations, 4 variables */
    /* ... fill data ... */
    stats = matrix.mcolstats(data, "Statistics")
    matrix.mprint(stats)
    ```

### Performance Optimizations
1. Cache-friendly blocking
   - Matrix operations use block sizes optimized for L1 cache
   - Reduces cache misses and improves performance
   
2. OpenMP Parallelization
   - Automatic parallelization for large matrices
   - Configurable threshold for parallel execution
   
3. Memory Access Patterns
   - Column-major operations optimized for matrix structure
   - Minimizes cache thrashing
   
4. Numerical Stability
   - Robust handling of small numbers
   - Stable statistical computations
   
### Statistical Measures
- **Mean**: Arithmetic average
- **Standard Deviation**: Sample standard deviation (n-1 denominator)
- **Median**: Middle value (interpolated for even sample sizes)
- **Skewness**: Measure of distribution asymmetry
- **Kurtosis**: Measure of tail weight (excess kurtosis, normal = 0)