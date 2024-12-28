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
#define matp(mptr,row,col) mptr->vector[row * mptr->cols + col]
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

void printMatrix(int matname) {
    int i, j, status;
    status = validateMatrix(matname);
    struct Matrix matrix = *(struct Matrix *) allVectors[matname];
    if (status != MATRIX_VALID) {
        printf("Error: Matrix validation error %d for %d\n",MATRIX_VALID,matname);
        return;
    }
    printf("Matrix %d: %s, dimension: %dx%d\n",matname, matrix.id, matrix.cols, matrix.rows);
    for (i = 0; i < matrix.rows; ++i) {
        for (j = 0; j < matrix.cols; ++j) {
            printf("%10.6f ", matrix.vector[i * matrix.cols + j]);
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
        printf("Matrix validation for %d failed: %d\n",GETINT(ARG0),status);
        RETURNINT(status);
    }
    printMatrix(GETINT(ARG0));
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

// Calculate determinant using LU decomposition
double calculate_determinant(struct Matrix* matrix) {
    int i,j,k, n = matrix->rows;
    if (n != matrix->cols) return 0.0;
    
    double det = 1.0;
    double* temp = (double*)MATRIX_ALLOC(n * n * sizeof(double));
    if (!temp) return 0.0;
    
    // Copy matrix to temp
    for (i = 0; i < n * n; i++) {
        temp[i] = matrix->vector[i];
    }
    
    // Gaussian elimination
    for (i = 0; i < n; i++) {
        // Find pivot
        double pivot = temp[i * n + i];
        if (fabs(pivot) < MATRIX_EPSILON) {
            MATRIX_FREE(temp);
            return 0.0;  // Singular matrix
        }
        
        det *= pivot;
        
        // Eliminate column
        for (j = i + 1; j < n; j++) {
            double factor = temp[j * n + i] / pivot;
            for (k = i; k < n; k++) {
                temp[j * n + k] -= factor * temp[i * n + k];
            }
        }
    }
    
    MATRIX_FREE(temp);
    return det;
}

PROCEDURE(mdet) {
    int status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) {
        RETURNFLOAT(0.0);
    }
    
    struct Matrix matrix = *(struct Matrix *) allVectors[GETINT(ARG0)];
    double det = calculate_determinant(&matrix);
    RETURNFLOAT(det);
}

// LU Decomposition implementation
int lu_decomposition(struct Matrix* matrix, struct Matrix* L, struct Matrix* U) {
    int n = matrix->rows;
    int i, j, k, p;
    
    if (n != matrix->cols) return MATRIX_INVALID_PARAM;
    
    // Initialize L and U
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            if (i == j) {
                matp(L,i,j) = 1.0;  // Diagonal of L is 1
            } else {
                matp(L,i,j) = 0.0;
            }
            matp(U,i,j) = 0.0;
        }
    }
    
    // Copy first row of U
    for (j = 0; j < n; j++) {
        matp(U,0,j) = matp(matrix,0,j);
    }
    
    // Calculate first column of L
    for (i = 1; i < n; i++) {
        if (fabs(matp(U,0,0)) < MATRIX_EPSILON) return -1;  // Singular matrix
        matp(L,i,0) = matp(matrix,i,0) / matp(U,0,0);
    }
    
    // Calculate rest of L and U
    for (k = 1; k < n; k++) {
        // Calculate row k of U
        for (j = k; j < n; j++) {
            double sum = 0.0;
            for (p = 0; p < k; p++) {
                sum += matp(L,k,p) * matp(U,p,j);
            }
            matp(U,k,j) = matp(matrix,k,j) - sum;
        }
        
        // Calculate column k of L
        for (i = k + 1; i < n; i++) {
            double sum = 0.0;
            for (p = 0; p < k; p++) {
                sum += matp(L,i,p) * matp(U,p,k);
            }
            if (fabs(matp(U,k,k)) < MATRIX_EPSILON) return -1;  // Singular matrix
            matp(L,i,k) = (matp(matrix,i,k) - sum) / matp(U,k,k);
        }
    }
    
    return MATRIX_SUCCESS;
}

PROCEDURE(mlu) {
    int status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);
    
    struct Matrix matrix = *(struct Matrix *) allVectors[GETINT(ARG0)];
    
    // Create L and U matrices
    int L_num = matcreate(matrix.rows, matrix.cols, GETSTRING(ARG1));
    if (L_num < 0) RETURNINT(L_num);
    
    int U_num = matcreate(matrix.rows, matrix.cols, GETSTRING(ARG2));
    if (U_num < 0) {
        freeMatrix(L_num);
        RETURNINT(U_num);
    }
    
    struct Matrix L = *(struct Matrix *) allVectors[L_num];
    struct Matrix U = *(struct Matrix *) allVectors[U_num];
    
    status = lu_decomposition(&matrix, &L, &U);
    if (status != MATRIX_SUCCESS) {
        freeMatrix(L_num);
        freeMatrix(U_num);
        RETURNINT(status);
    }
    
    // Return L matrix number (U matrix number is L_num + 1)
    RETURNINT(L_num);
}

// QR decomposition using Gram-Schmidt process
int qr_decomposition(struct Matrix* matrix, struct Matrix* Q, struct Matrix* R) {
    int m = matrix->rows;
    int n = matrix->cols;
    int i, j, k;
    double norm, dot;
    
    // Initialize Q and R
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            matp(Q,i,j) = matp(matrix,i,j);
            matp(R,i,j) = 0.0;
        }
    }
    
    // Gram-Schmidt process
    for (j = 0; j < n; j++) {
        // Calculate R[j,j]
        norm = 0.0;
        for (i = 0; i < m; i++) {
            norm += matp(Q,i,j) * matp(Q,i,j);
        }
        norm = sqrt(norm);
        
        if (norm < MATRIX_EPSILON) {
            return -1;  // Linearly dependent columns
        }
        
        matp(R,j,j) = norm;
        
        // Normalize column j of Q
        for (i = 0; i < m; i++) {
            matp(Q,i,j) /= norm;
        }
        
        // Calculate R[j,k] and update Q[:,k]
        for (k = j + 1; k < n; k++) {
            dot = 0.0;
            for (i = 0; i < m; i++) {
                dot += matp(Q,i,j) * matp(Q,i,k);
            }
            matp(R,j,k) = dot;
            
            for (i = 0; i < m; i++) {
                matp(Q,i,k) -= dot * matp(Q,i,j);
            }
        }
    }
    
    return MATRIX_SUCCESS;
}

// Power method for dominant eigenvalue/eigenvector
int power_method(struct Matrix* matrix, double* eigenvalue, double* eigenvector) {
    int n = matrix->rows;
    int i, j, iter;
    double norm, prev_eigenvalue;
    double* temp;
    
    if (n != matrix->cols) return MATRIX_INVALID_PARAM;
    
    // Initialize random vector
    for (i = 0; i < n; i++) {
        eigenvector[i] = (double)rand() / RAND_MAX;
    }
    
    // Normalize initial vector
    norm = 0.0;
    for (i = 0; i < n; i++) {
        norm += eigenvector[i] * eigenvector[i];
    }
    norm = sqrt(norm);
    for (i = 0; i < n; i++) {
        eigenvector[i] /= norm;
    }
    
    // Power iteration
    prev_eigenvalue = 0.0;
    temp = (double*)MATRIX_ALLOC(n * sizeof(double));
    if (!temp) return MATRIX_ALLOC_DATA;
    
    for (iter = 0; iter < 100; iter++) {  // Max 100 iterations
        // Matrix-vector multiplication
        for (i = 0; i < n; i++) {
            temp[i] = 0.0;
            for (j = 0; j < n; j++) {
                temp[i] += matp(matrix,i,j) * eigenvector[j];
            }
        }
        
        // Calculate eigenvalue (Rayleigh quotient)
        *eigenvalue = 0.0;
        for (i = 0; i < n; i++) {
            *eigenvalue += temp[i] * eigenvector[i];
        }
        
        // Check convergence
        if (fabs(*eigenvalue - prev_eigenvalue) < MATRIX_EPSILON) {
            MATRIX_FREE(temp);
            return MATRIX_SUCCESS;
        }
        prev_eigenvalue = *eigenvalue;
        
        // Normalize
        norm = 0.0;
        for (i = 0; i < n; i++) {
            norm += temp[i] * temp[i];
        }
        norm = sqrt(norm);
        for (i = 0; i < n; i++) {
            eigenvector[i] = temp[i] / norm;
        }
    }
    
    MATRIX_FREE(temp);
    return MATRIX_SUCCESS;
}

// Calculate matrix rank using QR decomposition
int calculate_rank(struct Matrix* matrix) {
    int m = matrix->rows;
    int n = matrix->cols;
    int i, rank = 0;
    int Q_num, R_num;
    struct Matrix *Q, *R;
    
    // Create temporary matrices for QR decomposition
    Q_num = matcreate(m, n, "Q_temp");
    if (Q_num < 0) return -1;
    
    R_num = matcreate(n, n, "R_temp");
    if (R_num < 0) {
        freeMatrix(Q_num);
        return -1;
    }
    
    Q = (struct Matrix*)allVectors[Q_num];
    R = (struct Matrix*)allVectors[R_num];
    
    // Perform QR decomposition
    if (qr_decomposition(matrix, Q, R) != MATRIX_SUCCESS) {
        freeMatrix(Q_num);
        freeMatrix(R_num);
        return -1;
    }
    
    // Count non-zero diagonal elements in R
    for (i = 0; i < MIN(m,n); i++) {
        if (fabs(matp(R,i,i)) > MATRIX_EPSILON) {
            rank++;
        }
    }
    
    // Clean up temporary matrices
    freeMatrix(Q_num);
    freeMatrix(R_num);
    
    return rank;
}

PROCEDURE(mrank) {
    int status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);
    
    struct Matrix matrix = *(struct Matrix *) allVectors[GETINT(ARG0)];
    int rank = calculate_rank(&matrix);
    
    RETURNINT(rank);
}

// Calculate covariance between two columns
double calculate_covariance(struct Matrix* matrix, int col1, int col2) {
    int i;
    double mean1 = 0.0, mean2 = 0.0, covar = 0.0;
    
    // Calculate means
    for (i = 0; i < matrix->rows; i++) {
        mean1 += matp(matrix,i,col1);
        mean2 += matp(matrix,i,col2);
    }
    mean1 /= matrix->rows;
    mean2 /= matrix->rows;
    
    // Calculate covariance
    for (i = 0; i < matrix->rows; i++) {
        covar += (matp(matrix,i,col1) - mean1) * (matp(matrix,i,col2) - mean2);
    }
    return covar / (matrix->rows - 1);  // Using n-1 for sample covariance
}

// Create covariance matrix
PROCEDURE(mcov) {
    int i, j, matnum, status;
    
    status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);
    
    struct Matrix matrix = *(struct Matrix *) allVectors[GETINT(ARG0)];
    
    // Create square matrix for covariance
    matnum = matcreate(matrix.cols, matrix.cols, GETSTRING(ARG1));
    if (matnum < 0) RETURNINT(matnum);
    
    struct Matrix covar = *(struct Matrix *) allVectors[matnum];
    
    // Calculate covariance matrix
    for (i = 0; i < matrix.cols; i++) {
        for (j = i; j < matrix.cols; j++) {
            double cov = calculate_covariance(&matrix, i, j);
            mat(covar,i,j) = cov;
            mat(covar,j,i) = cov;  // Covariance matrix is symmetric
        }
    }
    RETURNINT(matnum);
}

// Calculate correlation matrix
PROCEDURE(mcorr) {
    int i, j, matnum, status;
    double *stdevs;
    
    status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);
    
    struct Matrix matrix = *(struct Matrix *) allVectors[GETINT(ARG0)];
    
    // Create square matrix for correlation
    matnum = matcreate(matrix.cols, matrix.cols, GETSTRING(ARG1));
    if (matnum < 0) RETURNINT(matnum);
    
    struct Matrix corr = *(struct Matrix *) allVectors[matnum];
    
    // Calculate standard deviations for each column
    stdevs = (double*)MATRIX_ALLOC(matrix.cols * sizeof(double));
    if (!stdevs) {
        freeMatrix(matnum);
        RETURNINT(MATRIX_ALLOC_DATA);
    }
    
    for (i = 0; i < matrix.cols; i++) {
        double mean = calculate_mean(GETINT(ARG0), i);
        stdevs[i] = calculate_stdev(GETINT(ARG0), i, mean);
    }
    
    // Calculate correlation matrix
    for (i = 0; i < matrix.cols; i++) {
        for (j = i; j < matrix.cols; j++) {
            if (stdevs[i] < MATRIX_EPSILON || stdevs[j] < MATRIX_EPSILON) {
                mat(corr,i,j) = (i == j) ? 1.0 : 0.0;
            } else {
                double cov = calculate_covariance(&matrix, i, j);
                double corr_val = cov / (stdevs[i] * stdevs[j]);
                mat(corr,i,j) = corr_val;
                mat(corr,j,i) = corr_val;  // Correlation matrix is symmetric
            }
        }
    }
    
    MATRIX_FREE(stdevs);
    RETURNINT(matnum);
}

// Calculate row/column means
PROCEDURE(mmean) {
    int i, j, matnum, status;
    int axis = GETINT(ARG1);  // 0 for row means, 1 for column means
    
    status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);
    
    struct Matrix matrix = *(struct Matrix *) allVectors[GETINT(ARG0)];
    
    if (axis == 0) {
        matnum = matcreate(matrix.rows, 1, GETSTRING(ARG2));
    } else {
        matnum = matcreate(1, matrix.cols, GETSTRING(ARG2));
    }
    if (matnum < 0) RETURNINT(matnum);
    
    struct Matrix means = *(struct Matrix *) allVectors[matnum];
    
    if (axis == 0) {  // Row means
        for (i = 0; i < matrix.rows; i++) {
            double sum = 0.0;
            for (j = 0; j < matrix.cols; j++) {
                sum += mat(matrix,i,j);
            }
            mat(means,i,0) = sum / matrix.cols;
        }
    } else {  // Column means
        for (j = 0; j < matrix.cols; j++) {
            double sum = 0.0;
            for (i = 0; i < matrix.rows; i++) {
                sum += mat(matrix,i,j);
            }
            mat(means,0,j) = sum / matrix.rows;
        }
    }
    
    RETURNINT(matnum);
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
    ADDPROC(mdet,      "matrix.mdet",      "b",  ".int", "m0=.int");
    ADDPROC(mlu,       "matrix.mlu",       "b",  ".int", "m0=.int, L=.string, U=.string");
    ADDPROC(mrank,     "matrix.mrank",     "b",  ".int", "m0=.int");
    ADDPROC(mcov,      "matrix.mcov",      "b",  ".int", "m0=.int, mid=.string");
    ADDPROC(mcorr,     "matrix.mcorr",     "b",  ".int", "m0=.int, mid=.string");
    ADDPROC(mmean,     "matrix.mmean",     "b",  ".int", "m0=.int, axis=.int, mid=.string");
ENDLOADFUNCS