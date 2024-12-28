//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // For POSIX systems (Linux/macOS)
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file
#include <math.h>
#ifdef _WIN32
    #include <windows.h>
    #define MATRIX_ALLOC(size) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size)
    #define MATRIX_FREE(ptr) HeapFree(GetProcessHeap(), 0, ptr)
    HANDLE hHeap = NULL;
#else
    #include <stdlib.h>
    #define MATRIX_ALLOC(size) calloc(1, size)
    #define MATRIX_FREE(ptr) free(ptr)
#endif

// Maximum number of matrices that can be allocated
#define MATRIX_MAX_COUNT 100

// Error codes
#define MATRIX_SUCCESS          0
#define MATRIX_INVALID_PARAM   -1
#define MATRIX_NO_SLOTS       -12
#define MATRIX_ALLOC_CB       -16
#define MATRIX_ALLOC_DATA     -20

// Numerical constants
#define MATRIX_EPSILON 1e-10

// Matrix validation return codes
#define MATRIX_VALID           0
#define MATRIX_NULL          -30
#define MATRIX_INVALID_INDEX -31
#define MATRIX_CORRUPT      -32

// Add these optimization macros at the top
#define MATRIX_BLOCK_SIZE 64  // Cache-friendly block size

struct Matrix {
    double* CBselfref;     // CB self reference
    char id[32];
    int rows;
    int cols;
    double * vector;  // Pointer to the matrix data
};
#define mat(mptr,row,col) mptr.vector[row * mptr.cols + col]
// Helper macro for minimum value
#define MIN(a,b) ((a) < (b) ? (a) : (b))


int matrixmax = MATRIX_MAX_COUNT;
void * allVectors[MATRIX_MAX_COUNT];

uintptr_t getmain(int bytes) {
    if (bytes <= 0) {
        return MATRIX_INVALID_PARAM;
    }
    
    #ifdef _WIN32
    if (hHeap == NULL) {
        hHeap = GetProcessHeap();
        if (hHeap == NULL) {
            return MATRIX_ALLOC_CB;
        }
    }
    #endif
    
    void *pMemory = MATRIX_ALLOC(bytes);
    if (pMemory == NULL) {
        return MATRIX_ALLOC_DATA;
    }
    
    return (uintptr_t)pMemory;
}

int validateMatrix(int matnum) {
    if (matnum < 0 || matnum >= matrixmax) {
        return MATRIX_INVALID_INDEX;
    }

    if (allVectors[matnum] == NULL) {
        return MATRIX_NULL;
    }

    struct Matrix *matrix = (struct Matrix *)allVectors[matnum];

    // Check for corruption
    if (matrix->CBselfref != (double *)allVectors[matnum] ||
        matrix->rows <= 0 || matrix->cols <= 0 ||
        matrix->vector == NULL) {
        return MATRIX_CORRUPT;
    }

    return MATRIX_VALID;
}

void printMatrix(struct Matrix* matrix) {
    int i, j;
    if (!matrix) {
        printf("Error: NULL matrix pointer\n");
        return;
    }
    printf("Matrix %s, dimension: %dx%d\n", matrix->id, matrix->cols, matrix->rows);
    for (i = 0; i < matrix->rows; ++i) {
        for (j = 0; j < matrix->cols; ++j) {
            printf("%10.6f ", matrix->vector[i * matrix->cols + j]);
        }
        printf("\n");
    }
}

// Function to calculate the mean of the vector
double calculate_mean(int matname, int col) {
    int i;
    struct Matrix matrix = *(struct Matrix *) allVectors[matname];
    if (matrix.rows <= 0) return 0.0;
    
    double sum = 0.0;
    for (i = 0; i < matrix.rows; i++) {
        sum += mat(matrix,i,col);
    }
    return sum / matrix.rows;
}

// Function to calculate the standard deviation of the vector
double calculate_stdev(int matname, int col, double mean) {
    int i;
    struct Matrix matrix = *(struct Matrix *) allVectors[matname];
    if (matrix.rows <= 1) return 0.0;  // Need at least 2 points for std dev
    
    double sum_squared_diff = 0.0;
    for (i = 0; i < matrix.rows; i++) {
        double diff = mat(matrix,i,col) - mean;
        sum_squared_diff += diff * diff;
    }
    return sqrt(sum_squared_diff / (matrix.rows - 1));  // Using n-1 for sample standard deviation
}

// Function to standardize the vector
void standardize_vector(int matname, int matnew, int col) {
    int i;
    double mean = calculate_mean(matname, col);
    double stdev = calculate_stdev(matname, col, mean);
    struct Matrix matrix= *(struct Matrix *) allVectors[matname];
    struct Matrix matstd= *(struct Matrix *) allVectors[matnew];

    // Standardize each element
    for (i = 0; i < matrix.rows; i++) {
        mat(matstd,i,col) = (mat(matrix,i,col)- mean) / stdev;
    }
 }


int matcreate(int rows, int cols, char * matid) {
    if (rows <= 0 || cols <= 0 || matid == NULL) {
        return MATRIX_INVALID_PARAM;
    }
    
    int matrixname;
    double * matCB;
    double * matVECTOR;
    
    // find empty matrix slot
    for (matrixname = 0; matrixname <= matrixmax; ++matrixname) {
        if (allVectors[matrixname] == 0) break;
    }
    if (matrixname > matrixmax) return MATRIX_NO_SLOTS;
    
    matCB = (double *) getmain(sizeof(struct Matrix));
    if ((intptr_t)matCB < 0) return MATRIX_ALLOC_CB;

    struct Matrix * MATRIX = (struct Matrix *) matCB;
    MATRIX->CBselfref = matCB;     // save self referring CB address
    MATRIX->rows = rows;   // rows of matrix
    MATRIX->cols = cols;   // cols of matrix

    // allocate vector containing the real matrix
    matVECTOR = (double *) getmain(rows * cols * sizeof(double));
    if ((intptr_t)matVECTOR < 0) {
        HeapFree(hHeap, 0, matCB);  // Clean up on failure
        return MATRIX_ALLOC_DATA;
    }
    
    MATRIX->vector = matVECTOR;
    allVectors[matrixname] = matCB;
    strncpy(MATRIX->id, matid, sizeof(MATRIX->id) - 1);
    MATRIX->id[sizeof(MATRIX->id) - 1] = '\0';  // Ensure null termination
    
    return matrixname;
}
PROCEDURE(mcreate) {
    int rows, cols, matnum;
    char * id = GETSTRING(ARG2);
    
    rows = GETINT(ARG0);   // rows of matrix
    cols = GETINT(ARG1);   // cols of matrix
    
    matnum = matcreate(rows, cols, id);
    
    // Return codes from matcreate:
    // -1:  Invalid input parameters
    // -12: No matrix slots available
    // -16: Unable to allocate heap memory for matrix structure
    // -20: Unable to allocate heap memory for matrix data
    RETURNINT(matnum);
}

PROCEDURE(mset) {
    int row, col, status;
    
    status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) {
        RETURNINT(status);  // Return validation error code
    }
    
    struct Matrix cmatrix = *(struct Matrix *) allVectors[GETINT(ARG0)];
    row = GETINT(ARG1) - 1;
    col = GETINT(ARG2) - 1;
    
    if (row < 0 || row >= cmatrix.rows || col < 0 || col >= cmatrix.cols) {
        RETURNINT(MATRIX_INVALID_INDEX);
    }
    
    mat(cmatrix,row,col) = GETFLOAT(ARG3);
    RETURNINT(MATRIX_SUCCESS);
}
PROCEDURE(mprint) {
    int status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) {
        printf("Matrix validation failed: %d\n", status);
        RETURNINT(status);
    }
    
    struct Matrix cmatrix = *(struct Matrix *) allVectors[GETINT(ARG0)];
    printMatrix((struct Matrix *) cmatrix.CBselfref);
    ENDPROC
}

// Optimized matrix multiplication with blocking
PROCEDURE(mmultiply) {
    int i, j, k, ii, jj, kk, matnum, status;
    double sum;
    
    status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);
    
    status = validateMatrix(GETINT(ARG1));
    if (status != MATRIX_VALID) RETURNINT(status);
    
    struct Matrix mptr1 = *(struct Matrix *) allVectors[GETINT(ARG0)];
    struct Matrix mptr2 = *(struct Matrix *) allVectors[GETINT(ARG1)];
    
    if (mptr1.cols != mptr2.rows) {
        RETURNINT(MATRIX_INVALID_PARAM);
    }
    
    matnum = matcreate(mptr1.rows, mptr2.cols, GETSTRING(ARG2));
    if (matnum < 0) RETURNINT(matnum);
    
    struct Matrix mptr3 = *(struct Matrix *) allVectors[matnum];
    
    // Initialize result matrix to zero
    for (i = 0; i < mptr1.rows; i++) {
        for (j = 0; j < mptr2.cols; j++) {
            mat(mptr3,i,j) = 0.0;
        }
    }
    
    // Block matrix multiplication
    for (ii = 0; ii < mptr1.rows; ii += MATRIX_BLOCK_SIZE) {
        for (jj = 0; jj < mptr2.cols; jj += MATRIX_BLOCK_SIZE) {
            for (kk = 0; kk < mptr1.cols; kk += MATRIX_BLOCK_SIZE) {
                // Process block
                for (i = ii; i < MIN(ii + MATRIX_BLOCK_SIZE, mptr1.rows); i++) {
                    for (j = jj; j < MIN(jj + MATRIX_BLOCK_SIZE, mptr2.cols); j++) {
                        sum = mat(mptr3,i,j);
                        for (k = kk; k < MIN(kk + MATRIX_BLOCK_SIZE, mptr1.cols); k++) {
                            sum += mat(mptr1,i,k) * mat(mptr2,k,j);
                        }
                        mat(mptr3,i,j) = sum;
                    }
                }
            }
        }
    }
    
    RETURNINT(matnum);
}

// Function to invert a matrix using Gaussian elimination
// int invertMatrix(int n, double matrix[n][n], double inverse[n][n])
PROCEDURE(minvert) {
    int i,j,k,n,matnum, status;
    
    status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);
    
    struct Matrix matrix = *(struct Matrix *) allVectors[GETINT(ARG0)];
    
    // Validate matrix is square
    if (matrix.rows != matrix.cols) {
        RETURNINT(MATRIX_INVALID_PARAM);
    }
    n = matrix.cols;
    
    matnum = matcreate(matrix.rows, matrix.cols, GETSTRING(ARG1));
    if (matnum < 0) {
        RETURNINT(-2);  // Failed to create result matrix
    }
    struct Matrix minv = *(struct Matrix *) allVectors[matnum];

    double augmented[n][2 * n];   // Create an augmented matrix [matrix | I]
    
    // Initialize augmented matrix
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            augmented[i][j] = mat(matrix,i,j);
        }
        for (j = n; j < 2 * n; j++) {
            augmented[i][j] = (i == j - n) ? 1.0 : 0.0;
        }
    }
    
    // Gaussian elimination with improved pivot handling
    for (i = 0; i < n; i++) {
        if (fabs(augmented[i][i]) < 1e-10) {  // Better numerical stability check
            // Search for a row to swap
            int swapRow = -1;
            for (k = i + 1; k < n; k++) {
                if (fabs(augmented[k][i]) > 1e-10) {
                    swapRow = k;
                    break;
                }
            }
            if (swapRow == -1) {
                RETURNINT(-3);  // Matrix is singular
            }
            // Swap rows
            for (j = 0; j < 2 * n; j++) {
                double temp = augmented[i][j];
                augmented[i][j] = augmented[swapRow][j];
                augmented[swapRow][j] = temp;
            }
        }
        
        // Rest of the inversion process remains the same
        double pivot = augmented[i][i];
        for (j = 0; j < 2 * n; j++) {
            augmented[i][j] /= pivot;
        }
        for (k = 0; k < n; k++) {
            if (k != i) {
                double factor = augmented[k][i];
                for (j = 0; j < 2 * n; j++) {
                    augmented[k][j] -= factor * augmented[i][j];
                }
            }
        }
    }

    // Extract the inverse matrix
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            mat(minv,i,j) = augmented[i][j + n];
        }
    }

    RETURNINT(matnum);
}

// Optimized matrix transpose with better cache usage
PROCEDURE(mtranspose) {
    int i, j, ii, jj, rows, cols, matnum, status;
    
    status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);
    
    struct Matrix matrix = *(struct Matrix *) allVectors[GETINT(ARG0)];
    
    matnum = matcreate(matrix.cols, matrix.rows, GETSTRING(ARG1));
    if (matnum < 0) RETURNINT(matnum);
    
    struct Matrix mtrans = *(struct Matrix *) allVectors[matnum];
    
    rows = matrix.rows;
    cols = matrix.cols;
    
    // Block transpose for better cache usage
    for (ii = 0; ii < rows; ii += MATRIX_BLOCK_SIZE) {
        for (jj = 0; jj < cols; jj += MATRIX_BLOCK_SIZE) {
            for (i = ii; i < MIN(ii + MATRIX_BLOCK_SIZE, rows); i++) {
                for (j = jj; j < MIN(jj + MATRIX_BLOCK_SIZE, cols); j++) {
                    mat(mtrans,j,i) = mat(matrix,i,j);
                }
            }
        }
    }
    
    RETURNINT(matnum);
}
PROCEDURE(mstandard) {
    int i, j, matnew, status;
    
    status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);
    
    struct Matrix matrix = *(struct Matrix *) allVectors[GETINT(ARG0)];
    
    if (matrix.rows <= 1) {
        RETURNINT(-1);  // Need at least 2 rows for standardization
    }
    
    matnew = matcreate(matrix.rows, matrix.cols, GETSTRING(ARG1));
    if (matnew < 0) {
        RETURNINT(-2);  // Matrix creation failed
    }
    
    struct Matrix matnew_matrix = *(struct Matrix *) allVectors[matnew];
    
    // Standardize each column
    for (j = 0; j < matrix.cols; j++) {
        double mean = calculate_mean(GETINT(ARG0), j);
        double stdev = calculate_stdev(GETINT(ARG0), j, mean);
        
        if (stdev < 1e-10) {  // Check for zero/near-zero standard deviation
            // Copy column as-is if stdev is too small
            for (i = 0; i < matrix.rows; i++) {
                mat(matnew_matrix, i, j) = 0.0;
            }
            continue;
        }
        
        // Standardize column
        for (i = 0; i < matrix.rows; i++) {
            mat(matnew_matrix, i, j) = (mat(matrix, i, j) - mean) / stdev;
        }
    }
    
    RETURNINT(matnew);
}
PROCEDURE(mprod) {
    int i, j, matprod, rows, cols, status;
    
    status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);
    
    struct Matrix matrix = *(struct Matrix *) allVectors[GETINT(ARG0)];
    
    matprod = matcreate(matrix.rows, matrix.cols, GETSTRING(ARG2));
    if (matprod < 0) {
        RETURNINT(-1);  // Matrix creation failed
    }
    
    struct Matrix matnew = *(struct Matrix *) allVectors[matprod];
    double scalar = GETFLOAT(ARG1);

    rows = matrix.rows;
    cols = matrix.cols;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            mat(matnew,i,j) = mat(matrix,i,j) * scalar;
        }
    }
    
    RETURNINT(matprod);
}
int freeMatrix(int matnum) {
    int status = validateMatrix(matnum);
    if (status != MATRIX_VALID) {
        return status;
    }
    
    struct Matrix *matrix = (struct Matrix *)allVectors[matnum];
    
    if (!MATRIX_FREE(matrix->vector)) {
        return -8;
    }
    
    if (!MATRIX_FREE(matrix->CBselfref)) {
        return -12;
    }
    
    allVectors[matnum] = NULL;
    return 0;
}
PROCEDURE(mfree) {
    int i,rc;
    if(GETINT(ARG0)>=0) {
        RETURNINT(freeMatrix(GETINT(ARG0)));
    } else {
        for (i = 0; i < matrixmax; i++) {
            if (allVectors[i] != NULL) {  // Only try to free non-NULL entries
                rc = freeMatrix(i);
                if (rc == 0) printf("Matrix freed %d\n",i);
            }
        }
    }
    ENDPROC
}

/* -------------------------------------------------------------------------------------
 * Functions provided to REXX:
 * mmultiply:  Multiply two matrices (m0 x m1 -> mid)
 * mprod:      Multiply matrix by scalar (m0 x prod -> mid)
 * minvert:    Invert a matrix (m0 -> mid)
 * mtranspose: Transpose a matrix (m0 -> mid)
 * mstandard:  Standardize matrix columns (m0 -> mid)
 * mcreate:    Create a new matrix (rows x cols -> id)
 * mset:       Set matrix element (m0[row,col] = value)
 * mprint:     Print matrix (m0)
 * mfree:      Free matrix memory (m0 or all if m0 < 0)
 * -------------------------------------------------------------------------------------
 */
LOADFUNCS
    ADDPROC(mmultiply, "matrix.mmultiply", "b",  ".int", "m0=.int, m1=.int,mid=.string");
    ADDPROC(mprod,     "matrix.mprod",     "b",  ".int", "m0=.int, prod=.float,mid=.string");
    ADDPROC(minvert,   "matrix.minvert",   "b",  ".int", "m0=.int, mid=.string");
    ADDPROC(mtranspose,"matrix.mtranspose","b",  ".int", "m0=.int, mid=.string");
    ADDPROC(mstandard, "matrix.mstandard", "b",  ".int", "m0=.int, mid=.string");
    ADDPROC(mcreate,   "matrix.mcreate",   "b",  ".int", "rows=.int,cols=.int,id=.string");
    ADDPROC(mset,      "matrix.mset",      "b",  ".int", "m0=.int, row=.int, col=.int,value=.float");
    ADDPROC(mprint,    "matrix.mprint",    "b",  ".int", "m0=.int");
    ADDPROC(mfree,     "matrix.mfree",     "b",  ".int", "m0=.int");
ENDLOADFUNCS