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
- `mfactor(m0, factors, rotate, scores, mid_load, [mid_scores])` - Advanced factor analysis
  - Parameters:
    - m0: Input data matrix
    - factors: Number of factors to extract
    - rotate: Rotation method
      - 0: None
      - 1: Varimax (orthogonal)
      - 2: Quartimax (orthogonal)
      - 3: Promax (oblique)
    - scores: Calculate factor scores (0/1)
    - mid_load: Result matrix identifier for loadings
    - mid_scores: Result matrix identifier for scores (if scores=1)
  
  - Example:
    ```
    data = matrix.mcreate(100, 5, "DataMatrix")
    /* ... fill data matrix ... */
    
    // Basic factor analysis
    loadings = matrix.mfactor(data, 2, 0, 0, "Unrotated")
    
    // With varimax rotation
    loadings = matrix.mfactor(data, 2, 1, 0, "Varimax")
    
    // With promax rotation and scores
    loadings = matrix.mfactor(data, 2, 3, 1, "Promax", "Scores")
    ```
  
  - Rotation Methods:
    - Varimax: Maximizes variance of squared loadings
    - Quartimax: Simplifies rows of loading matrix
    - Promax: Allows correlated factors
  
  - Factor Scores:
    - Regression method
    - Standardized data
    - One score per factor per observation
  
  - Diagnostics:
    - Communalities
    - Factor correlations (for oblique rotation)
    - Proportion of variance explained

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

### Factor Analysis Diagnostics and Interpretation

#### Basic Usage
```
data = matrix.mcreate(100, 5, "DataMatrix")
/* ... fill data matrix ... */

// Factor analysis with diagnostics and interpretation
loadings = matrix.mfactor(data, 2, 1, 0, "Loadings", "Diagnostics", "Interpretation")
```

#### Diagnostic Matrix (4 rows)
1. **Communalities** (Row 0)
   - Values range from 0 to 1
   - Represents proportion of variance explained for each variable
   - Interpretation:
     - < 0.3: Poor explanation
     - 0.3-0.5: Moderate explanation
     - > 0.5: Good explanation

2. **Eigenvalues** (Row 1)
   - Total variance explained by each factor
   - Kaiser criterion: Keep factors with eigenvalues > 1

3. **Proportion of Variance** (Row 2)
   - Percentage of total variance explained by each factor
   - Higher values indicate more important factors

4. **Cumulative Proportion** (Row 3)
   - Running total of variance explained
   - Target: > 60% for adequate solution

#### Interpretation Matrix
- Values indicate significant loadings:
  - 1: Strong positive relationship (> 0.4)
  - -1: Strong negative relationship (< -0.4)
  - 0: Weak or insignificant relationship

#### Adequacy Checks
The procedure automatically checks for:
- Low communalities (< 0.3)
- Insufficient total variance explained (< 60%)
- Warnings are printed if these conditions are found

#### Best Practices
1. **Sample Size**
   - Minimum: 5 cases per variable
   - Preferred: 10+ cases per variable

2. **Number of Factors**
   - Use Kaiser criterion (eigenvalues > 1)
   - Check scree plot (use eigenvalues)
   - Consider interpretability

3. **Rotation Selection**
   - Varimax: When factors should be uncorrelated
   - Promax: When factors might be correlated
   - Quartimax: When seeking general factors

4. **Loading Interpretation**
   - 0.4 is typical significance threshold
   - Consider practical significance
   - Look for simple structure

### Advanced Interpretation and Visualization

#### Scree Plot Data
- Two-row matrix:
  1. Factor numbers (1-based)
  2. Eigenvalues
- Use for creating scree plot
- Interpretation:
  - Look for "elbow" point
  - Factors before elbow are significant
  - Example plot command:
    ```
    plot matrix.get(scree,1,*) type=line
    ```

#### Detailed Interpretation Matrix
- Rows 1 to p: Variables
  - Columns 1 to m: Factor loadings
  - Column m+1: Communalities
  - Column m+2: Complexity (number of significant loadings)
- Last three rows: Factor statistics
  - Eigenvalues
  - Proportion of variance
  - Cumulative proportion

#### Enhanced Adequacy Checks
1. **Communality Check**
   - < 0.3: Poor explanation
   - 0.3-0.5: Moderate
   - > 0.5: Good

2. **Variance Explained**
   - Total > 60%: Adequate
   - Total > 80%: Strong

3. **Cross-Loading Check**
   - Variables should load strongly on only one factor
   - Cross-loading indicates complex structure

4. **Sample Size**
   - Minimum: 5 cases per variable
   - Optimal: 10+ cases per variable

5. **Multicollinearity**
   - Checks for extreme correlations (> 0.9)
   - May indicate redundant variables

#### Visualization Best Practices
1. **Scree Plot**
   - Plot eigenvalues vs. factor number
   - Look for natural break point
   - Consider factors before the break

2. **Loading Plot**
   - Plot loadings for pairs of factors
   - Look for clusters of variables
   - Example:
     ```
     plot matrix.get(loadings,*,1) matrix.get(loadings,*,2)
     ```

3. **Heat Map**
   - Visualize loading patterns
   - Use color intensity for loading strength
   - Helps identify structure

#### Interpretation Guidelines
1. **Simple Structure**
   - Each variable loads strongly on one factor
   - Low loadings on other factors
   - Fewer cross-loadings is better

2. **Factor Naming**
   - Based on strongly loading variables
   - Consider theoretical framework
   - Look for common themes

3. **Factor Retention**
   - Kaiser criterion (eigenvalues > 1)
   - Scree plot elbow
   - Theoretical considerations
   - Interpretability

4. **Solution Quality**
   - Check all adequacy measures
   - Consider theoretical fit
   - Validate with split sample if possible

### Matrix Plotting

Basic plotting functionality using gnuplot:

```
matrix.mplot(matrix, plot_type)
```

Parameters:
- matrix: Matrix to plot
- plot_type: String specifying plot type
  - "line": Line plot
  - "scatter": Scatter plot
  - "bar": Bar chart
  - "heatmap": Heatmap

Examples:
```
// Create and plot correlation matrix
corr = matrix.mcorr(data, "Correlations")
matrix.mplot(corr, "heatmap")

// Plot factor loadings
matrix.mplot(loadings, "bar")

// Scree plot
matrix.mplot(eigenvalues, "line")
```

Requirements:
- Gnuplot must be installed and accessible
- Temporary file access for data transfer