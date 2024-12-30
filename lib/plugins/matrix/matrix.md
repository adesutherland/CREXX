# Matrix Library Reference

## Quick Reference

### Most Common Operations
```c
// Create matrix
id = mcreate(rows, cols, "name")

// Basic operations
madd(m1, m2, "result")    // Addition
mmult(m1, m2, "result")   // Multiplication
mtrans(m1, "result")      // Transpose

// Statistical analysis
mcorr(data, "correlation")     // Correlation matrix
mfactor(data, n, rot, scores)  // Factor analysis

// Visualization
masciiplot(data, "hist")       // ASCII histogram
mplot(data, "line")           // Gnuplot line plot
```

### Common Parameters
- `rows`, `cols`: Matrix dimensions
- `rot`: Rotation type (0=none, 1=varimax, 2=quartimax, 3=promax)
- `scores`: Calculate factor scores (0/1)

## Detailed Documentation

### Matrix Operations

#### Creation and Initialization
```c
// Create new matrix
id = mcreate(rows, cols, "name")

// Initialize matrix
mset(id, row, col, value)    // Set single element

```

#### Access and Information
```c
value = mget(id, row, col)   // Get element
rows = mrows(id)             // Get row count
cols = mcols(id)             // Get column count
name = mname(id)             // Get matrix name
```

#### Memory Management
```c
mfree(id)                    // Free matrix

### Statistical Functions

#### Basic Statistics
```c
mmean(data, "means")         // Column means
mstdev(data, "stddevs")      // Standard deviations
```

#### Correlation and Covariance
```c
mcorr(data, "correlation")   // Correlation matrix
mcov(data, "covariance")     // Covariance matrix
```

#### Distribution Statistics
```c
// Box plot statistics
calculate_box_stats(data, n, &min, &q1, &median, &q3, &max)
```

### Factor Analysis

#### Main Function
```c
mfactor(m0, factors, rotate, scores, 'matrix-id-text-load','matrix-id-text-scores','matrix-id-text-diagnose')
```
mfactor does a factor analysis, whith an optional rotation. As a result 3 matrices will be created. They are consecutive to the factor-loadings number.   
#### Parameters
- `m0`: Input data matrix
- `factors`: Number of factors to extract
- `rotate`: Rotation method
  - 0: None
  - 1: Varimax (max iterations = 100)
  - 2: Quartimax (max iterations = 100)
  - 3: Promax (kappa = 4)
- `scores`: Calculate factor scores (0/1)
- `matrix-id-text-load`: factor-loadings matrix name
- `matrix-id-text-scores`: scores matrix name
- `matrix-id-text-diagnose`: diagnostics matrix name

#### Adequacy Checks
```c
// Returned in diagnostics matrix
1. KMO measure (> 0.6 acceptable)
2. Bartlett's test
3. Communalities (> 0.3 acceptable)
4. Total variance explained (> 60% good)
5. Sample size check (> 5 * variables)
```

#### Rotation Details
```c
// Convergence parameters
ROTATE_MAX_ITER = 100
ROTATE_CONVERGE = 0.000001

// Promax parameters
PROMAX_POWER = 4
```

### Visualization

#### Gnuplot Integration
```c
// Path configuration
#define GNUPLOT_DEFAULT_PATH "C:\\Program Files\\gnuplot\\bin\\gnuplot.exe"

// Plot dimensions
#define PLOT_WIDTH  60
#define PLOT_HEIGHT 20
```

#### Plot Types and Parameters

##### Standard Plots (`mplot`)
```c
mplot(data, plot_type)

// Plot types:
"line"    - Line plot
"scatter" - Scatter plot
"bar"     - Bar chart
"hist"    - Histogram
```

##### Factor Analysis Plots (`mfaplot`)
```c
mfaplot(matrix, plot_type [, scores_matrix])

// Plot types:
"scree"    - Scree plot with elbow detection
"loadings" - Factor loadings plot
"biplot"   - Combined loadings and scores
```

##### ASCII Plots (`masciiplot`)
```c
masciiplot(matrix, plot_type)

// Plot types:
"hist"    - Histogram (60x20 characters)
"bar"     - Bar chart
"line"    - Line plot
"heat"    - Heat map
"scatter" - Scatter plot (60x20 characters)
"box"     - Box plot
```

#### Return Codes
```c
 0: Success
-1: Invalid matrix
-2: Gnuplot not found
-3: Temporary file error
-4: Invalid plot type
-5: Insufficient data
```

### Error Handling

#### Matrix Validation
```c
MATRIX_VALID       0
MATRIX_INVALID    -1
MATRIX_NOT_FOUND  -2
MATRIX_BAD_SIZE   -3
```

#### Common Errors
```c
// Memory errors
MATRIX_NO_MEMORY  -10
MATRIX_BAD_ALLOC  -11

// Operation errors
MATRIX_BAD_OP     -20
MATRIX_SINGULAR   -21
MATRIX_NO_CONV    -22  // No convergence
```

### Best Practices

#### Factor Analysis
1. Sample size: 5-10 cases per variable
2. Check communalities (> 0.3)
3. Examine factor loadings (> 0.4)
4. Consider theoretical meaning

#### Visualization
1. Choose appropriate plot type
2. Check for scaling issues
3. Consider data density
4. Use ASCII plots for quick checks
5. Use Gnuplot for publication quality

#### Memory Management
- Matrices are stored in row-major order
- Automatic garbage collection on program exit
- Manual memory management with `mfree()`
- Temporary matrices are auto-cleaned

#### Performance
- BLAS-like optimizations for large matrices
- In-place operations when possible
- Efficient memory reuse
- Parallel processing for large operations

#### Limitations
- Maximum matrix size: system memory dependent
- Maximum name length: 32 characters
- Maximum factors: min(rows, cols)
- Maximum plot dimensions: 60x20 (ASCII)

### Examples

#### Factor Analysis Workflow
```c
// 1. Prepare data
data = mcreate(100, 5, "MyData")
// ... load data ...

// 2. Check correlations
corr = mcorr(data, "Correlations")
masciiplot(corr, "heat")
/*
Heat Map:
1 | #.+*#
2 | .#*+.
3 | +*#+*
4 | *+#.*
5 | #.+*.
*/

// 3. Run factor analysis
loadings = mfactor(data, 2, 1, 1, "Loadings", "Scores", "Diagnostics")

// 4. Check scree plot
masciiplot(eigenvalues, "line")
/*
5.0 |   *
4.0 |    *
3.0 |     *--*--*
2.0 |
1.0 |
    ---------------
*/

// 5. Examine loadings
masciiplot(loadings, "bar")
/*
Var1 |##### 0.85
Var2 |### 0.45
Var3 |###### 0.92
*/

// 6. Plot factor scores
masciiplot(scores, "scatter")
/*
2.0 |   * *  *
1.0 |  *   *
0.0 | *  *  *
   --------------
*/
```

#### Time Series Analysis
```c
// 1. Create time series data
ts = mcreate(100, 1, "TimeSeries")
// ... load data ...

// 2. Plot trend
masciiplot(ts, "line")
/*
10.0 |    *--*
 8.0 |   *    *
 6.0 |  *      *
     -------------
*/

// 3. Plot both series
mplot(ts, "line")      // Using gnuplot
```

#### Correlation Analysis
```c
// 1. Create correlation matrix
corr = mcorr(data, "Correlations")

// 2. Visualize patterns
masciiplot(corr, "heat")
/*
Heat Map:
1 | #.+*#
2 | .#*+.
3 | +*#+*
*/

```

#### Distribution Analysis
```c
// 1. Basic histogram
masciiplot(data, "hist")
/*
    *
   ***
  *****
 *******
---------
*/

// 2. Box plot for each variable
masciiplot(data, "box")
/*
Var1: |---[####|####]---|
Var2: |--[###|###]--|
Var3: |----[#####|#]---|
*/

// 3. Compare distributions
stats = mstats(data, "Statistics")
/*
      Mean   SD    Min   Max
Var1  5.2    1.1   2.1   8.3
Var2  4.8    0.9   2.5   7.2
*/
```

#### Matrix Operations
```c
// 1. Matrix multiplication with verification
A = mcreate(3, 2, "A")
B = mcreate(2, 3, "B")
// ... fill matrices ...

// Check dimensions
if(mcols(A) == mrows(B)) {
    C = mmult(A, B, "C")
    mprint(C)
} else {
    printf("Invalid dimensions for multiplication\n")
}`

## Operation-Specific Error Codes

### Matrix Creation and Manipulation
```c
-10: Invalid row count
-11: Invalid column count
-12: Invalid matrix name
-13: Matrix already exists
-14: Index out of bounds (mset)
```

### Statistical Operations
```c
-30: Insufficient data
-31: Zero variance
-32: Invalid axis parameter
-33: Non-positive definite matrix
-34: Convergence failure
```

### Plotting
```c
-50: Invalid plot type
-51: Gnuplot not found
-52: Plot file creation error
-53: Empty matrix
-54: Plot dimensions error
```

## Error Handling Examples

### Matrix Creation
```c
id = mcreate(rows, cols, "matrix")
if (id < 0) {
    switch(id) {
        case -10: print("Invalid row count")
        case -11: print("Invalid column count")
        case -12: print("Invalid matrix name")
        case -13: print("Matrix already exists")
        case -5:  print("Memory allocation failed")
    }
}
```

### Matrix Operations
```c
result = mmultiply(m1, m2, "result")
if (result < 0) {
    switch(result) {
        case -2:  print("Invalid matrix ID")
        case -3:  print("Matrix not found")
        case -20: print("Incompatible dimensions")
        case -5:  print("Memory allocation failed")
    }
}
```

### Statistical Operations
```c
result = mcorr(m0, "correlation")
if (result < 0) {
    switch(result) {
        case -30: print("Insufficient data")
        case -31: print("Zero variance")
        case -33: print("Non-positive definite matrix")
    }
}
```

### Factor Analysis
```c
result = mfactor(m0, factors, rotation, scores, "loadings")
if (result < 0) {
    switch(result) {
        case -40: print("Too few variables")
        case -41: print("Too few observations")
        case -42: print("Invalid number of factors")
        case -44: print("Rotation did not converge")
    }
}
```

### Plotting
```c
result = mplot(m0, "line")
if (result < 0) {
    switch(result) {
        case -50: print("Invalid plot type")
        case -51: print("Gnuplot not found")
        case -52: print("Plot file creation error")
        case -53: print("Empty matrix")
    }
}
```

## Best Practices

1. **Always Check Return Values**
   ```c
   result = operation(...)
   if (result < 0) {
       // Handle error
   }
   ```

2. **Memory Management**
   ```c
   id = mcreate(...)
   if (id >= 0) {
       // Use matrix
       mfree(id)  // Clean up
   }
   ```

3. **Validation Before Operations**
   ```c
   if (mrank(m0) < min_rank) {
       // Handle insufficient rank
   }
   ```

4. **Error Recovery**
   ```c
   if (result < 0) {
       mfree(temp_matrix)  // Clean up temporary matrices
       return result             // Propagate error
   }
   ```

## Common Error Patterns

1. **Dimension Mismatch**
   - Matrix multiplication
   - Matrix addition/subtraction
   - Element-wise operations

2. **Numerical Issues**
   - Singular matrices
   - Zero variance
   - Convergence failures

3. **Resource Issues**
   - Memory allocation
   - File operations
   - External program access (Gnuplot)

4. **Invalid Parameters**
   - Matrix indices
   - Factor counts
   - Rotation methods

## Complete Example
```c
// Create and populate a data matrix
data_id = mcreate(100, 5, "Data")
// ... set values ...

// Standardize the data
std_id = mstandard(data_id, "Standardized")

// Calculate correlation matrix
corr_id = mcorr(std_id, "Correlations")
masciiplot(corr_id, "heat")

// Perform factor analysis
fa_id = mfactor(std_id, 2, 1, 1, "Loadings")
mfaplot(fa_id, "scree")
```

#### mset(m0, row, col, value)
**Description:**  
Sets a specific value in the 

**Parameters:**
- `m0`: Integer, matrix ID.
- `row`: Integer, row index (0-based).
- `col`: Integer, column index (0-based).
- `value`: Float, value to set.

**Returns:** 
- Integer: 0 for successful completion, else error.

**Example:**
```c
mset(id, 1, 2, 3.14);
```

#### mprint(m0)
**Description:**  
Prints the contents of the 

**Parameters:**
- `m0`: Integer, matrix ID.

**Returns:** 
- Integer: 0 for successful completion, else error.

**Example:**
```c
mprint(id);
```

#### mfree(m0)
**Description:**  
Frees the memory allocated for the matrix number presented. If -1 used all matrices will be freed.

**Parameters:**
- `m0`: Integer, matrix ID.

**Returns:** 
- Integer: 0 for successful completion, else error.

**Example:**
```c
mfree(id);
```

## Matrix Arithmetic

### Matrix Operations

#### mmultiply(m0, m1, 'matrix-identification-text')
**Description:**  
Multiplies two matrices.

**Parameters:**
- `m0`: Integer, first matrix ID.
- `m1`: Integer, second matrix ID.
- `matrix-identification-text`: String, result matrix identifier.

**Returns:** 
- Integer: Result matrix ID or error code.

**Example:**
```c
c_id = mmultiply(a_id, b_id, "C");
```

#### mprod(m0, prod, 'matrix-identification-text')
**Description:**  
Performs scalar multiplication on a 

**Parameters:**
- `m0`: Integer, matrix ID.
- `prod`: Float, scalar value.
- `matrix-identification-text`: String, result matrix identifier.

**Returns:** 
- Integer: Result matrix ID or error code.

**Example:**
```c
scaled_id = mprod(id, 2.5, "Scaled");
```

#### minvert(m0, 'matrix-identification-text')
**Description:**  
Inverts a 

**Parameters:**
- `m0`: Integer, matrix ID.
- `matrix-identification-text`: String, result matrix identifier.

**Returns:** 
- Integer: Result matrix ID or error code.

**Example:**
```c
inv_id = minvert(id, "Inverse");
```

#### mtranspose(m0, 'matrix-identification-text')
**Description:**  
Transposes a 

**Parameters:**
- `m0`: Integer, matrix ID.
- `matrix-identification-text`: String, result matrix identifier.

**Returns:** 
- Integer: Result matrix ID or error code.

**Example:**
```c
trans_id = mtranspose(id, "Transposed");
```

## Matrix Properties

### Matrix Characteristics

#### mdet(m0)
**Description:**  
Calculates the determinant of a 

**Parameters:**
- `m0`: Integer, matrix ID.

**Returns:** 
- Float: Determinant value.

**Example:**
```c
det = mdet(id);
```

#### mrank(m0)
**Description:**  
Calculates the rank of a 

**Parameters:**
- `m0`: Integer, matrix ID.

**Returns:** 
- Integer: Rank of the matrix or error code.

**Example:**
```c
rank = mrank(id);
```

#### mlu(m0, L, U)
**Description:**  
Performs LU decomposition on a 

**Parameters:**
- `m0`: Integer, matrix ID.
- `L`: String, lower triangular matrix identifier.
- `U`: String, upper triangular matrix identifier.

**Returns:** 
- Integer: 0 for successful completion, else error.

**Example:**
```c
status = mlu(id, "L_matrix", "U_matrix");
```

## Statistical Operations

### Statistical Functions

#### mstandard(m0, 'matrix-identification-text')
**Description:**  
Standardizes a matrix (z-scores).

**Parameters:**
- `m0`: Integer, matrix ID.
- `matrix-identification-text`: String, standardized matrix identifier.

**Returns:** 
- Integer: Result matrix ID or error code.

**Example:**
```c
std_id = mstandard(id, "Standardized");
```

#### mcov(m0, 'matrix-identification-text')
**Description:**  
Calculates the covariance 

**Parameters:**
- `m0`: Integer, matrix ID.
- `matrix-identification-text`: String, covariance matrix identifier.

**Returns:** 
- Integer: Result matrix ID or error code.

**Example:**
```c
cov_id = mcov(id, "Covariance");
```

#### mcorr(m0, 'matrix-identification-text')
**Description:**  
Calculates the correlation 

**Parameters:**
- `m0`: Integer, matrix ID.
- `matrix-identification-text`: String, correlation matrix identifier.

**Returns:** 
- Integer: Result matrix ID or error code.

**Example:**
```c
corr_id = mcorr(id, "Correlation");
```

#### mmean(m0, axis)
**Description:**  
Calculates the mean of the matrix along the specified column.

**Parameters:**
- `m0`: Integer, matrix ID.
- `axis`: Integer, 0 for rows, 1 for columns.

**Returns:** 
- Float: Mean value.

**Example:**
```c
col_mean = mmean(id, 1);
```

#### mstdev(m0, axis)
**Description:**  
Calculates the standard deviation of the matrix along the specified column.

**Parameters:**
- `m0`: Integer, matrix ID.
- `axis`: Integer, 0 for rows, 1 for columns.

**Returns:** 
- Float: Standard deviation value.

**Example:**
```c
row_sd = mstdev(id, 0);
```

## Factor Analysis

### Factor Analysis Functions

#### mfactor(m0, factors, rotation, scores, 'matrix-identification-text')
**Description:**  
Performs factor analysis on the 

**Parameters:**
- `m0`: Integer, matrix ID.
- `factors`: Integer, number of factors to extract.
- `rotation`: Integer, rotation method (0=none, 1=varimax).
- `scores`: Integer, calculate scores (0/1).
- `matrix-identification-text`: String, loadings matrix identifier.

**Returns:** 
- Integer: Result matrix ID or error code.

**Example:**
```c
fa_id = mfactor(id, 3, 1, 1, "Loadings");
```

## Visualization

### Plotting Functions

#### mplot(m0, plot_type)
**Description:**  
Creates a plot for the specified 

**Parameters:**
- `m0`: Integer, matrix ID.
- `plot_type`: String, type of plot.

**Returns:** 
- Integer: 0 for successful completion, else error.

**Example:**
```c
mplot(id, "line");
```

#### mfaplot(m0, plot_type)
**Description:**  
Creates a factor analysis plot for the specified 

**Parameters:**
- `m0`: Integer, matrix ID.
- `plot_type`: String, type of factor analysis plot.

**Returns:** 
- Integer: 0 for successful completion, else error.

**Example:**
```c
mfaplot(id, "scree");
```

#### masciiplot(m0, plot_type)
**Description:**  
Creates an ASCII-based plot for the specified 

**Parameters:**
- `m0`: Integer, matrix ID.
- `plot_type`: String, type of ASCII plot.

**Returns:** 
- Integer: 0 for successful completion, else error.

**Example:**
```c
masciiplot(id, "hist");
```

## Error Codes

### General Error Codes
```
 0: Success
-1: General error
-2: Invalid matrix ID
-3: Matrix not found
-4: Invalid dimensions
-5: Memory allocation error
```

### Operation-Specific Error Codes

#### Matrix Creation and Manipulation
```
-10: Invalid row count
-11: Invalid column count
-12: Invalid matrix name
-13: Matrix already exists
-14: Index out of bounds (mset)
```

#### Matrix Arithmetic
```
-20: Incompatible dimensions for multiplication
-21: Singular matrix (for inversion)
-22: Non-square matrix (when square required)
-23: Division by zero
-24: Overflow error
```

#### Statistical Operations
```
-30: Insufficient data
-31: Zero variance
-32: Invalid axis parameter
-33: Non-positive definite matrix
-34: Convergence failure
```

#### Factor Analysis
```
-40: Too few variables
-41: Too few observations
-42: Invalid number of factors
-43: Invalid rotation method
-44: Rotation did not converge
-45: Insufficient rank
```

#### Plotting
```
-50: Invalid plot type
-51: Gnuplot not found
-52: Plot file creation error
-53: Empty matrix
-54: Plot dimensions error
```

### Output of Regression
The result matrix will typically include:
- Coefficients for each independent variable.
- Intercept.
- R-squared value.
- Standard error of the coefficients.
- p-values for hypothesis testing.

### Error Codes for Regression
- **-60**: Invalid dependent variable ID.
- **-61**: Invalid independent variables ID.
- **-62**: Dimension mismatch between dependent and independent variables.
- **-63**: Insufficient data for regression analysis.

## Conclusion
