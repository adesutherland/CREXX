---

# Matrix Operations Library

---

## Introduction
The **Matrix Operations Library** is a comprehensive and versatile toolkit rooted in the principles of **Linear Algebra** and enriched with robust **statistical capabilities**. Linear Algebra forms the foundation for understanding and manipulating matrices and vector spaces, while the statistical components extend its application to data analysis and interpretation. This library is designed to facilitate efficient matrix computations, statistical analysis, and data visualization.

With its ability to handle both fundamental operations, such as matrix multiplication and inversion, and advanced statistical tasks, like covariance and factor analysis, the library bridges the gap between mathematical theory and practical data-driven applications.

## Overview
### Summary
This library provides:
- **Basic Matrix Functions**: Includes creation, multiplication, inversion, transposition, and arithmetic operations.
- **Matrix Characteristics**: Tools for calculating determinants, ranks, and decompositions (e.g., LU, QR, and SVD).
- **Statistical Analysis**: Functions for covariance, correlation, mean, standard deviation, and advanced metrics like skewness and kurtosis.
- **Factor Analysis**: Includes support for rotation methods (Varimax, Promax) and factor score calculations.
- **Visualization Tools**: Enables matrix data visualization through line plots, scatterplots, and ASCII-based heatmaps.
- **Error Handling**: Comprehensive mechanisms to handle common matrix errors, ensuring robust and fail-safe operations.
- **Performance Optimization**: Features for parallel processing, memory optimization, and numerical stability.

The library is structured with best practices for memory management, error recovery, and performance tuning, making it suitable for both small-scale and high-performance computing environments. With its extensibility for advanced features like sparse matrices and system limits, the **Matrix Operations Library** delivers a powerful and flexible solution for handling complex matrix workflows.

---


## Basic Matrix Functions

### Matrix Creation

### Creating a New Matrix
- **Purpose**: Creates a new matrix with specified dimensions.
- **Parameters**:
    - `rows`: Number of rows (int)
    - `cols`: Number of columns (int)
    - `id`: Matrix identifier (string)
- **Returns**: Matrix handle (int)
```c
id = mcreate(3, 3, "myMatrix");
```

### Matrix Multiplication (`mmult`,'description')
- **Purpose**: Multiplies two matrices: \( C = A * B \)
- **Parameters**:
    - `m0`: First input matrix (int)
    - `m1`: Second input matrix (int)
    - `mid`: Result matrix identifier (string)
- **Returns**: Matrix handle (int)

### Scalar Multiplication (`mprod`,'description')
- **Purpose**: Multiplies a matrix by a scalar: \( B = A * scalar \)
- **Parameters**:
    - `m0`: Input matrix (int)
    - `prod`: Scalar value (float)
    - `mid`: Result matrix identifier (string)
- **Returns**: Matrix handle (int)


### Matrix Inversion (`minvert`,'description')
- **Purpose**: Computes the inverse of a matrix: \( B = A^{-1} \)
- **Parameters**:
    - `m0`: Input matrix (int)
    - `mid`: Result matrix identifier (string)
- **Returns**: Matrix handle (int)

### Matrix Transposition (`mtranspose`,'description')
- **Purpose**: Computes the transpose of a matrix: \( B = A^T \)
- **Parameters**:
    - `m0`: Input matrix (int)
    - `mid`: Result matrix identifier (string)
- **Returns**: Matrix handle (int)

### Matrix Arithmetic Examples
```c
madd(m1, m2, "result"); // Addition
mmult(m1, m2, "result"); // Multiplication
mtrans(m1, "result"); // Transpose
```

### Matrix Characteristics
### `mdet`
- **Purpose**: Calculate matrix determinant
- **Parameters**:
  - m0: Input matrix (.int)
- **Returns**: Success status (.int)

### `mrank`
- **Purpose**: Calculate matrix rank
- **Parameters**:
  - m0: Input matrix (.int)
- **Returns**: Matrix rank (.int)
### `mlu`
- **LU Decomposition**: `mlu(m0, L, U)` — Performs LU decomposition, returning lower (`L`) and upper (`U`) triangular matrices.
- m0: Input matrix (.int)
### `mprint`
- **Purpose**: Print matrix to output
- **Parameters**:
  - m0: Matrix to print (.int,"alternative-heading") - if empty Matrix heading defined with MCREATE is used
- **Returns**: Success status (.int)

---

## Statistical Functions

### Basic Statistical Operations

#### Covariance (`mcov`,'description')
Covariance is a statistical measure that quantifies the extent to which two variables change together. It provides insight into the direction of the linear relationship between variables:

Positive Covariance: Indicates that as one variable increases, the other tends to increase as well.
Negative Covariance: Suggests that as one variable increases, the other tends to decrease.

- **Purpose**: Computes the covariance matrix.
- **Returns**: Covariance matrix handle (int)
```c
cov_id = mcov(id, "Covariance");
```

#### Correlation (`mcorr`,'description')
Correlation is a statistical measure that quantifies the strength and direction of the linear relationship between two variables. Unlike covariance, correlation is standardized, providing values within the range
− −1 to 1

- 1: Perfect positive correlation (variables increase together).
- -1: Perfect negative correlation (one variable increases as the other decreases).
- 0: No linear relationship between the variables.


- **Purpose**: Computes the correlation matrix.
- **Returns**: Correlation matrix handle (int)
```c
corr_id = mcorr(id, "Correlation");
```

### Mean and Standard Deviation
Mean and Standard Deviation (StdDev) are fundamental statistical measures used to describe and analyze datasets:

**Mean**:

Represents the average value of a dataset.
Calculated by summing all values and dividing by the number of observations.
Reflects the central tendency of the data.

**Standard Deviation**:

Measures the dispersion or spread of data around the mean.
A smaller standard deviation indicates that data points are closer to the mean, while a larger standard deviation signifies greater variability.

### `mmean(m0, row)`
- **Purpose**: Calculate mean along specified columns
- **Parameters**:
  - m0: Input matrix (.int)
  - column number
- **Returns**: Mean value (.float)
 
### `mstdev(m0, row)`
- **Purpose**: Calculates standard deviation along specified columns
- **Parameters**:
  - m0: Input matrix (.int)
  - column number
- **Returns**: standard deviation value (.float)

```c
mean = mmean(id, 1); // Row 1 mean
stddev = mstdev(id, 2); // Row 2 standard deviation
```
### Matrix Helper Functions
### `colstats`
- **Purpose**: General statistical analysis
- **Parameters**:
  - m0: Input matrix (.int)
  - mode: row number 
  - **Returns**: Vector - Matrix(4,1)  with input matrix details
```
    row 1: StdDev
    row 2: median
    row 3: skewness
    row 4: kurtosis 
```
 
### Factor Analysis
Factor Analysis is a statistical method designed to uncover the underlying structure in a dataset by reducing observed variables into a smaller set of latent variables, called factors. These factors represent shared variance among the observed variables and are used to simplify data interpretation while retaining essential information. The technique is widely applied in psychology, social sciences, finance, and other fields for tasks such as dimensionality reduction and identifying relationships among variables.
Identify and group correlated variables into factors.
Reduce dimensionality while preserving meaningful patterns in the data.
### Factor Analysis (`mfactor`)
- **Purpose**: Performs factor analysis, including options for rotation (e.g., Varimax, Promax).
- **Parameters**:
    - `m0`: Input matrix (int)
    - `factors`: Number of factors (int)
    - `mid`: Result matrix description
    - `parms`: Additional options (string, e.g., rotation type)
- **Returns**: Matrix handle (int)
```c
loadings = mfactor(mfdata, 2, "Factor Analysis Varimax","Rotate=VARIMAX");
loadings2 = mfactor(mfdata, 1, "Factor Analysis Varimax","Rotate=Varimax,SCORE,diag=3")
loadings3 = mfactor(mfdata, 2, "Factor Analysis unrotated","Rotate=none,SCORES,diag=3")
loadings4 = mfactor(mfdata, 2, "Factor Analysis Quartimax","diag=3")
```

#### Factor Analysis Options
1. **ROTATE=type**: Type of rotation (e.g., `PROMAX`, `VARIMAX`, `NONE`).
2. **SCORE**: Enable factor score calculation.
3. **DIAG=value**: Diagnostic level.

### Advanced Matrix Operations
- **Specialized Decompositions**
  - Cholesky decomposition
  - QR decomposition
  - Singular Value Decomposition (SVD)
- **Matrix Functions**
  - Matrix exponential
  - Matrix logarithm
  - Matrix power
---

## Visualization

### Plotting Functions

#### `mplot`
- **Purpose**: Creates various plots like line, scatter, or bar charts.
- **Parameters**:
    - `m0`: Matrix ID (int)
    - `plot_type`: Plot type (string)
```c
mplot(id, "line"); // Line plot
```

#### `mfaplot`
- **Purpose**: Plots factor analysis results (e.g., scree plot, loadings).
- **Parameters**:
    - `m0`: Matrix ID (int)
    - `plot_type`: Plot type (string, e.g., `scree`, `loadings`)
```c
mfaplot(loadings, "scree"); // Scree plot
```

#### `masciiplot`
- **Purpose**: Creates ASCII-based plots (e.g., heat maps, histograms).
- **Parameters**:
    - `m0`: Matrix ID (int)
    - `plot_type`: Plot type (string)
```c
masciiplot(id, "heat"); // Heat map
```

---

## Error Handling

### Common Error Codes
- **General Errors**: `-1` for general errors, `-2` for invalid matrix ID.
- **Matrix Errors**: `-20` for dimension mismatch, `-21` for singular matrix.
- **Plotting Errors**: `-50` for invalid plot type, `-51` for missing Gnuplot.

### Example of Error Handling
```c
result = mmult(m1, m2, "result");
if (result < 0) {
    switch(result) {
        case -20: printf("Dimension mismatch\n"); break;
        case -5: printf("Memory allocation failed\n"); break;
    }
}
```

---

## Best Practices

1. **Always Check Return Values**: Ensure that matrix operations and functions return success before proceeding.
2. **Memory Management**: Clean up matrices with `mfree()` to prevent memory leaks.
3. **Error Recovery**: Implement proper error handling to manage issues such as dimension mismatches or singular matrices.

---

## Complete Example

```c
// Create a data matrix
data_id = mcreate(100, 5, "MyData");

// Calculate correlation matrix
corr_id = mcorr(data_id, "Correlations");

// Plot the correlation matrix as a heat map
masciiplot(corr_id, "heat");

// Perform factor analysis with 2 factors and Varimax rotation
loadings = mfactor(data_id, 2, "Factor Analysis Varimax", "Rotate=VARIMAX");

// Plot the factor analysis results
mfaplot(loadings, "scree");
```

## Extensions

### Matrix Types
- **Dense Matrix**: Standard row-major storage
- **Sparse Matrix**: Compressed storage for sparse data
- **Symmetric Matrix**: Optimized storage for symmetric data

### Error Categories
1. **Success (0)**
   - Successful operation completion

2. **Memory Management (-1 to -9)**
   - Memory allocation/deallocation
   - Handle management

3. **Dimension Related (-10 to -19)**
   - Size compatibility
   - Index bounds

4. **Calculation Related (-20 to -29)**
   - Numerical issues
   - Convergence problems

5. **Parameter Related (-30 to -39)**
   - Invalid inputs
   - Option parsing

6. **File Operations (-40 to -49)**
   - I/O errors
   - Format issues

7. **System Limits (-50 to -59)**
   - Resource constraints
   - System limitations
### Advanced Features
1. **Parallel Processing**
   - OpenMP support for large matrices
   - Thread-safe operations
   - Parallel algorithm implementations

2. **Memory Optimization**
   - Smart memory allocation
   - Cache-friendly algorithms
   - Memory pool for small matrices
   

3. **Error Handling**
   ```c
   #define MATRIX_SUCCESS       0
   #define MATRIX_MALLOC_ERROR -1
   #define MATRIX_DIM_ERROR   -2
   #define MATRIX_SING_ERROR  -3
   ```

### Performance Considerations
1. **Algorithm Selection**
   - Size-based optimization
   - Automatic method switching
   - Resource usage monitoring

2. **Cache Optimization**
   - Block operations
   - Stride optimization
   - Memory alignment

3. **Numerical Stability**
   - Condition number checking
   - Scaling strategies
   - Precision management

### Matrix Validation
- **Input Validation**
  - Dimension checks
  - Value range validation
  - Type compatibility
- **Result Validation**
  - Numerical stability checks
  - Convergence verification
  - Error bounds calculation

[Add to Statistical Functions]


#### `matrix.stats`
- **Purpose**: General statistical analysis
- **Parameters**:
  - m0: Input matrix (.int)
  - mode: Analysis mode (.string) - ROW or COL
- **Returns**: returns matrix dimension either column or row number 

[Add to Visualization]

#### `matrix.mplot`
- **Purpose**: Create matrix plot
- **Parameters**:
  - m0: Input matrix (.int)
  - plot_type: Type of plot (.string)
- **Returns**: Success status (.int)

#### `masciiplot`
- **Purpose**: Create ASCII visualization
- **Parameters**:
  - m0: Input matrix (.int)
  - plot_type: Plot type (.string)
- **Returns**: Success status (.int)