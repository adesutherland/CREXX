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

// Rotation types
#define ROTATE_NONE     0
#define ROTATE_VARIMAX  1
#define ROTATE_QUARTIMAX 2
#define ROTATE_PROMAX   3

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

// Centralized statistical functions
double calculate_column_mean(struct Matrix* matrix, int col) {
    int i;
    double sum = 0.0;
    
    if (matrix->rows <= 0) return 0.0;
    
    for (i = 0; i < matrix->rows; i++) {
        sum += matp(matrix,i,col);
    }
    return sum / matrix->rows;
}

double calculate_column_stddev(struct Matrix* matrix, int col, double mean) {
    int i;
    double sum_squared_diff = 0.0;
    
    if (matrix->rows <= 1) return 0.0;  // Need at least 2 points for std dev
    
    for (i = 0; i < matrix->rows; i++) {
        double diff = matp(matrix,i,col) - mean;
        sum_squared_diff += diff * diff;
    }
    return sqrt(sum_squared_diff / (matrix->rows - 1));  // Using n-1 for sample standard deviation
}

// Standardize a single column
void standardize_column(struct Matrix* matrix, struct Matrix* result, int col) {
    int i;
    double mean = calculate_column_mean(matrix, col);
    double stddev = calculate_column_stddev(matrix, col, mean);
    
    if (stddev < MATRIX_EPSILON) stddev = 1.0;  // Handle constant columns
    
    for (i = 0; i < matrix->rows; i++) {
        matp(result,i,col) = (matp(matrix,i,col) - mean) / stddev;
    }
}

int matcreate(int rows, int cols, int slotsneeded, char * matid) {
    if (rows <= 0 || cols <= 0 || matid == NULL) {
        return MATRIX_INVALID_PARAM;
    }

    int i,found, matrixname;
    double *matCB;
    double *matVECTOR;
 // find empty matrix slot
    for (matrixname = 0; matrixname < matrixmax - (slotsneeded - 1); ++matrixname) {
        found = 1;
        for (i = 0; i < slotsneeded; ++i) {
            if (allVectors[matrixname + i] != 0) {
                found = 0;
                break;
            }
        }
        if (found) {
            break; // Found n consecutive slots
        }
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
    
    matnum = matcreate(rows, cols, 1,id);
    
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
    
    matnum = matcreate(mptr1.rows, mptr2.cols, 1,GETSTRING(ARG2));
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
    
    matnum = matcreate(matrix.rows, matrix.cols, 1, GETSTRING(ARG1));
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
    
    matnum = matcreate(matrix.cols, matrix.rows, 1,GETSTRING(ARG1));
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
    
    struct Matrix* matrix = (struct Matrix*)allVectors[GETINT(ARG0)];

    if (matrix->rows <= 1) {
        RETURNINT(-1);  // Need at least 2 rows for standardization
    }
    
    matnew = matcreate(matrix->rows, matrix->cols, 1,GETSTRING(ARG1));
    if (matnew < 0) {
        RETURNINT(-2);  // Matrix creation failed
    }

    struct Matrix* matnew_matrix = (struct Matrix*)allVectors[matnew];
    
    // Standardize each column
    for (j = 0; j < matrix->cols; j++) {
        standardize_column(matrix, matnew_matrix, j);
    }
    
    RETURNINT(matnew);
}
PROCEDURE(mprod) {
    int i, j, matprod, rows, cols, status;
    
    status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);
    
    struct Matrix matrix = *(struct Matrix *) allVectors[GETINT(ARG0)];
    
    matprod = matcreate(matrix.rows, matrix.cols, 1, GETSTRING(ARG2));
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
    int L_num = matcreate(matrix.rows, matrix.cols, 2,GETSTRING(ARG1));
    if (L_num < 0) RETURNINT(L_num);
    
    int U_num = matcreate(matrix.rows, matrix.cols, 1, GETSTRING(ARG2));
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
    Q_num = matcreate(m, n, 1,"Q_temp");
    if (Q_num < 0) return -1;
    
    R_num = matcreate(n, n,1, "R_temp");
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
    
    struct Matrix* matrix = (struct Matrix*)allVectors[GETINT(ARG0)];
    
    // Create square matrix for covariance
    matnum = matcreate(matrix->cols, matrix->cols,1, GETSTRING(ARG1));
    if (matnum < 0) RETURNINT(matnum);
    
    struct Matrix* covar = (struct Matrix*)allVectors[matnum];
    
    // Calculate covariance matrix
    for (i = 0; i < matrix->cols; i++) {
        for (j = i; j < matrix->cols; j++) {
            double cov = calculate_covariance(matrix, i, j);
            matp(covar,i,j) = cov;
            matp(covar,j,i) = cov;  // Covariance matrix is symmetric
        }
    }
    RETURNINT(matnum);
}

// Calculate correlation matrix
PROCEDURE(mcorr) {
    int i, j, matnum, status;
    double *means, *stdevs;
    
    status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);
    
    struct Matrix* matrix = (struct Matrix*)allVectors[GETINT(ARG0)];
    
    // Create arrays for means and standard deviations
    means = (double*)MATRIX_ALLOC(matrix->cols * sizeof(double));
    stdevs = (double*)MATRIX_ALLOC(matrix->cols * sizeof(double));
    if (!means || !stdevs) {
        if (means) MATRIX_FREE(means);
        if (stdevs) MATRIX_FREE(stdevs);
        RETURNINT(MATRIX_ALLOC_DATA);
    }
    
    // Calculate means and standard deviations
    for (i = 0; i < matrix->cols; i++) {
        means[i] = calculate_column_mean(matrix, i);
        stdevs[i] = calculate_column_stddev(matrix, i, means[i]);
    }
    
    // Create square matrix for correlation
    matnum = matcreate(matrix->cols, matrix->cols, 1,GETSTRING(ARG1));
    if (matnum < 0) RETURNINT(matnum);
    
    struct Matrix corr = *(struct Matrix *) allVectors[matnum];
    
    // Calculate correlation matrix
    for (i = 0; i < matrix->cols; i++) {
        for (j = i; j < matrix->cols; j++) {
            if (stdevs[i] < MATRIX_EPSILON || stdevs[j] < MATRIX_EPSILON) {
                mat(corr,i,j) = (i == j) ? 1.0 : 0.0;
            } else {
                double cov = calculate_covariance(matrix, i, j);
                double corr_val = cov / (stdevs[i] * stdevs[j]);
                mat(corr,i,j) = corr_val;
                mat(corr,j,i) = corr_val;  // Correlation matrix is symmetric
            }
        }
    }
    
    MATRIX_FREE(means);
    MATRIX_FREE(stdevs);
    RETURNINT(matnum);
}

// Calculate row/column means
PROCEDURE(mmean) {
    int i, j, matnum, status;
    int axis = GETINT(ARG1);  // 0 for row means, 1 for column means
    
    status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);
    
    struct Matrix* matrix = (struct Matrix*)allVectors[GETINT(ARG0)];
    
    if (axis == 0) {
        matnum = matcreate(matrix->rows, 1,1, GETSTRING(ARG2));
    } else {
        matnum = matcreate(1, matrix->cols,1, GETSTRING(ARG2));
    }
    if (matnum < 0) RETURNINT(matnum);
    
    struct Matrix* means = (struct Matrix*)allVectors[matnum];
    
    if (axis == 0) {  // Row means
        for (i = 0; i < matrix->rows; i++) {
            double sum = 0.0;
            for (j = 0; j < matrix->cols; j++) {
                sum += matp(matrix,i,j);
            }
            matp(means,i,0) = sum / matrix->cols;
        }
    } else {  // Column means
        for (j = 0; j < matrix->cols; j++) {
            matp(means,0,j) = calculate_column_mean(matrix, j);
        }
    }
    
    RETURNINT(matnum);
}

// Factor Analysis Implementation
int factor_analysis(struct Matrix* data, struct Matrix* loadings, int factors) {
    int i, j, k, iter;
    int rows = data->rows;
    int cols = data->cols;
    double eigenval;
    
    if (factors > cols) return MATRIX_INVALID_PARAM;
    
    // Step 1: Standardize the data
    int std_num = matcreate(rows, cols, 1,"std_data");
    if (std_num < 0) return std_num;
    struct Matrix* std_data = (struct Matrix*)allVectors[std_num];
    
    // Standardize each column
    for (j = 0; j < cols; j++) {
        standardize_column(data, std_data, j);
    }
    
    // Step 2: Compute correlation matrix
    int corr_num = matcreate(cols, cols, 1, "correlation");
    if (corr_num < 0) {
        freeMatrix(std_num);
        return corr_num;
    }
    struct Matrix* corr = (struct Matrix*)allVectors[corr_num];
    
    for (i = 0; i < cols; i++) {
        for (j = i; j < cols; j++) {
            double sum = 0.0;
            for (k = 0; k < rows; k++) {
                sum += matp(std_data,k,i) * matp(std_data,k,j);
            }
            matp(corr,i,j) = sum / (rows - 1);
            matp(corr,j,i) = matp(corr,i,j);
        }
    }
    
    // Step 3: Extract factors using power method
    for (k = 0; k < factors; k++) {
        double* eigenvector = (double*)MATRIX_ALLOC(cols * sizeof(double));
        if (!eigenvector) {
            freeMatrix(std_num);
            freeMatrix(corr_num);
            return MATRIX_ALLOC_DATA;
        }
        
        // Initialize eigenvector
        for (i = 0; i < cols; i++) {
            eigenvector[i] = 1.0 / sqrt(cols);
        }
        
        // Power iteration
        for (iter = 0; iter < 100; iter++) {
            double norm = 0.0;
            double* new_vector = (double*)MATRIX_ALLOC(cols * sizeof(double));
            if (!new_vector) {
                MATRIX_FREE(eigenvector);
                freeMatrix(std_num);
                freeMatrix(corr_num);
                return MATRIX_ALLOC_DATA;
            }
            
            // Matrix-vector multiplication
            for (i = 0; i < cols; i++) {
                new_vector[i] = 0.0;
                for (j = 0; j < cols; j++) {
                    new_vector[i] += matp(corr,i,j) * eigenvector[j];
                }
                norm += new_vector[i] * new_vector[i];
            }
            
            norm = sqrt(norm);
            
            // Update eigenvector
            for (i = 0; i < cols; i++) {
                eigenvector[i] = new_vector[i] / norm;
            }
            
            MATRIX_FREE(new_vector);
        }
        
        // Store factor loadings
        for (i = 0; i < cols; i++) {
            matp(loadings,i,k) = eigenvector[i];
        }
        
        // Deflate correlation matrix
        eigenval = 0.0;
        for (i = 0; i < cols; i++) {
            for (j = 0; j < cols; j++) {
                eigenval += eigenvector[i] * matp(corr,i,j) * eigenvector[j];
            }
        }
        
        for (i = 0; i < cols; i++) {
            for (j = 0; j < cols; j++) {
                matp(corr,i,j) -= eigenval * eigenvector[i] * eigenvector[j];
            }
        }
        
        MATRIX_FREE(eigenvector);
    }
    
    freeMatrix(std_num);
    freeMatrix(corr_num);
    return MATRIX_SUCCESS;
}

// Varimax rotation for factor analysis
int varimax_rotation(struct Matrix* loadings, int max_iter) {
    int p = loadings->rows;    // Number of variables
    int m = loadings->cols;    // Number of factors
    int i, j, k, iter;
    double* h2 = NULL;         // Communalities
    double* u = NULL;          // Temporary storage
    double* v = NULL;          // Temporary storage
    double* A = NULL;          // Temporary storage
    double* B = NULL;          // Temporary storage
    double d = 0.0;
    
    // Allocate memory
    h2 = (double*)MATRIX_ALLOC(p * sizeof(double));
    u = (double*)MATRIX_ALLOC(p * sizeof(double));
    v = (double*)MATRIX_ALLOC(p * sizeof(double));
    A = (double*)MATRIX_ALLOC(p * sizeof(double));
    B = (double*)MATRIX_ALLOC(p * sizeof(double));
    
    if (!h2 || !u || !v || !A || !B) {
        if (h2) MATRIX_FREE(h2);
        if (u) MATRIX_FREE(u);
        if (v) MATRIX_FREE(v);
        if (A) MATRIX_FREE(A);
        if (B) MATRIX_FREE(B);
        return MATRIX_ALLOC_DATA;
    }
    
    // Calculate initial communalities
    for (i = 0; i < p; i++) {
        h2[i] = 0.0;
        for (j = 0; j < m; j++) {
            h2[i] += matp(loadings,i,j) * matp(loadings,i,j);
        }
        if (h2[i] < MATRIX_EPSILON) h2[i] = 1.0;
    }
    
    // Iterative rotation
    for (iter = 0; iter < max_iter; iter++) {
        double old_d = d;
        d = 0.0;
        
        // Rotate pairs of factors
        for (j = 0; j < m - 1; j++) {
            for (k = j + 1; k < m; k++) {
                double numerator = 0.0, denominator = 0.0;
                double s = 0.0, c = 0.0, t = 0.0;
                
                // Calculate rotation angle
                for (i = 0; i < p; i++) {
                    double x = matp(loadings,i,j);
                    double y = matp(loadings,i,k);
                    double u2 = x * x / h2[i];
                    double v2 = y * y / h2[i];
                    double uv = x * y / h2[i];
                    
                    numerator += 2.0 * uv;
                    denominator += u2 - v2;
                }
                
                // Calculate rotation
                if (fabs(denominator) > MATRIX_EPSILON) {
                    t = numerator / denominator;
                    s = 1.0 / sqrt(1.0 + t * t);
                    c = s * t;
                } else {
                    c = s = M_SQRT1_2;  // 1/√2
                }
                
                // Apply rotation
                for (i = 0; i < p; i++) {
                    double x = matp(loadings,i,j);
                    double y = matp(loadings,i,k);
                    matp(loadings,i,j) = x * c + y * s;
                    matp(loadings,i,k) = -x * s + y * c;
                }
                
                d += fabs(numerator);
            }
        }
        
        // Check convergence
        if (fabs(d - old_d) < MATRIX_EPSILON) break;
    }
    
    // Free memory
    MATRIX_FREE(h2);
    MATRIX_FREE(u);
    MATRIX_FREE(v);
    MATRIX_FREE(A);
    MATRIX_FREE(B);
    
    return MATRIX_SUCCESS;
}

// Factor rotation with multiple methods
int factor_rotation(struct Matrix* loadings, int method, int max_iter) {
    switch(method) {
        case ROTATE_VARIMAX:
            return varimax_rotation(loadings, max_iter);
        case ROTATE_QUARTIMAX:
            return quartimax_rotation(loadings, max_iter);
        case ROTATE_PROMAX:
            return promax_rotation(loadings, max_iter);
        default:
            return MATRIX_SUCCESS;  // No rotation
    }
}

// Quartimax rotation
int quartimax_rotation(struct Matrix* loadings, int max_iter) {
    int p = loadings->rows;    // Number of variables
    int m = loadings->cols;    // Number of factors
    int i, j, k, iter;
    double* h2 = NULL;         // Communalities
    double d = 0.0;
    
    // Allocate memory
    h2 = (double*)MATRIX_ALLOC(p * sizeof(double));
    if (!h2) return MATRIX_ALLOC_DATA;
    
    // Calculate communalities
    for (i = 0; i < p; i++) {
        h2[i] = 0.0;
        for (j = 0; j < m; j++) {
            h2[i] += matp(loadings,i,j) * matp(loadings,i,j);
        }
        if (h2[i] < MATRIX_EPSILON) h2[i] = 1.0;
    }
    
    // Iterative rotation
    for (iter = 0; iter < max_iter; iter++) {
        double old_d = d;
        d = 0.0;
        
        for (j = 0; j < m - 1; j++) {
            for (k = j + 1; k < m; k++) {
                double num = 0.0, den = 0.0;
                
                for (i = 0; i < p; i++) {
                    double x = matp(loadings,i,j);
                    double y = matp(loadings,i,k);
                    num += x * x * x * y - x * y * y * y;
                    den += x * x * y * y;
                }
                
                if (fabs(den) > MATRIX_EPSILON) {
                    double angle = atan2(2.0 * num, den) / 4.0;
                    double c = cos(angle);
                    double s = sin(angle);
                    
                    for (i = 0; i < p; i++) {
                        double x = matp(loadings,i,j);
                        double y = matp(loadings,i,k);
                        matp(loadings,i,j) = x * c + y * s;
                        matp(loadings,i,k) = -x * s + y * c;
                    }
                    
                    d += fabs(num / den);
                }
            }
        }
        
        if (fabs(d - old_d) < MATRIX_EPSILON) break;
    }
    
    MATRIX_FREE(h2);
    return MATRIX_SUCCESS;
}

// Promax rotation (k typically = 4)
int promax_rotation(struct Matrix* loadings, int max_iter) {
    int p = loadings->rows;
    int m = loadings->cols;
    int i, j;
    double k = 4.0;  // Typical value for k
    
    // First do Varimax rotation
    int status = varimax_rotation(loadings, max_iter);
    if (status != MATRIX_SUCCESS) return status;
    
    // Create temporary matrices
    int pattern_num = matcreate(p, m,1, "pattern");
    if (pattern_num < 0) return pattern_num;
    struct Matrix* pattern = (struct Matrix*)allVectors[pattern_num];
    
    // Calculate pattern matrix
    for (i = 0; i < p; i++) {
        for (j = 0; j < m; j++) {
            double load = matp(loadings,i,j);
            matp(pattern,i,j) = copysign(pow(fabs(load), k), load);
        }
    }
    
    // Solve for transformation matrix and apply it
    // Note: This is a simplified version. Full Promax would involve
    // solving a regression problem for the transformation matrix.
    for (i = 0; i < p; i++) {
        double row_sum = 0.0;
        for (j = 0; j < m; j++) {
            row_sum += matp(pattern,i,j) * matp(pattern,i,j);
        }
        row_sum = sqrt(row_sum);
        if (row_sum > MATRIX_EPSILON) {
            for (j = 0; j < m; j++) {
                matp(loadings,i,j) = matp(pattern,i,j) / row_sum;
            }
        }
    }
    
    freeMatrix(pattern_num);
    return MATRIX_SUCCESS;
}

// Calculate factor scores
int calculate_factor_scores(struct Matrix* data, struct Matrix* loadings, struct Matrix* scores) {
    int i, j, k;
    int n = data->rows;    // Number of observations
    int p = data->cols;    // Number of variables
    int m = loadings->cols; // Number of factors
    
    // Standardize the data first
    int std_num = matcreate(n, p, 1,"std_data");
    if (std_num < 0) return std_num;
    struct Matrix* std_data = (struct Matrix*)allVectors[std_num];
    
    for (j = 0; j < p; j++) {
        standardize_column(data, std_data, j);
    }
    
    // Calculate factor scores using regression method
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            double score = 0.0;
            for (k = 0; k < p; k++) {
                score += matp(std_data,i,k) * matp(loadings,k,j);
            }
            matp(scores,i,j) = score;
        }
    }
    
    freeMatrix(std_num);
    return MATRIX_SUCCESS;
}

// Calculate basic factor analysis diagnostics
int calculate_basic_diagnostics(struct Matrix* data, struct Matrix* loadings, struct Matrix* diag) {
    int i, j;
    int p = loadings->rows;    // Number of variables
    int m = loadings->cols;    // Number of factors
    double total_variance = 0.0;
    
    // Row 0: Communalities
    // Row 1: Eigenvalues
    // Row 2: Proportion of variance
    // Row 3: Cumulative proportion
    
    // Calculate communalities and eigenvalues
    for (i = 0; i < p; i++) {
        double comm = 0.0;
        for (j = 0; j < m; j++) {
            double loading = matp(loadings,i,j);
            comm += loading * loading;
        }
        matp(diag,0,i) = comm;  // Store communality
    }
    
    // Calculate eigenvalues and variance proportions
    for (j = 0; j < m; j++) {
        double eigenval = 0.0;
        for (i = 0; i < p; i++) {
            double loading = matp(loadings,i,j);
            eigenval += loading * loading;
        }
        matp(diag,1,j) = eigenval;
        total_variance += eigenval;
    }
    
    // Calculate proportions and cumulative proportions
    double cumulative = 0.0;
    for (j = 0; j < m; j++) {
        double prop = matp(diag,1,j) / p;  // Proportion of variance
        matp(diag,2,j) = prop;
        cumulative += prop;
        matp(diag,3,j) = cumulative;
    }
    
    return MATRIX_SUCCESS;
}

// Interpretation helpers for factor analysis
int interpret_loadings(struct Matrix* loadings, struct Matrix* interp, double threshold) {
    int i, j;
    int p = loadings->rows;    // Number of variables
    int m = loadings->cols;    // Number of factors
    
    // Create interpretation matrix (same size as loadings)
    // Values: 1 = significant positive, -1 = significant negative, 0 = not significant
    for (i = 0; i < p; i++) {
        for (j = 0; j < m; j++) {
            double loading = matp(loadings,i,j);
            if (fabs(loading) >= threshold) {
                matp(interp,i,j) = (loading > 0) ? 1 : -1;
            } else {
                matp(interp,i,j) = 0;
            }
        }
    }
    
    return MATRIX_SUCCESS;
}

// Helper to check adequacy of factor solution
int check_factor_adequacy(struct Matrix* diag) {
    int inadequate = 0;
    int i;
    
    // Check communalities (row 0)
    for (i = 0; i < diag->cols; i++) {
        if (matp(diag,0,i) < 0.3) {  // Less than 30% variance explained
            inadequate |= 1;
            break;
        }
    }
    
    // Check cumulative variance (row 3)
    if (matp(diag,3,diag->cols-1) < 0.6) {  // Less than 60% total variance
        inadequate |= 2;
    }
    
    return inadequate;
}

// Updated REXX procedure with interpretation
PROCEDURE(mfactor) {
    int status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);
    
    struct Matrix data = *(struct Matrix *) allVectors[GETINT(ARG0)];
    int factors = GETINT(ARG1);
    int rotate = GETINT(ARG2);  // Rotation method
    int scores = GETINT(ARG3);  // 0 = no scores, 1 = calculate scores
    
    // Create matrix for factor loadings
    int loadings_num = matcreate(data.cols, factors, 4, GETSTRING(ARG4));  // later maybe 2. matrix needed
    if (loadings_num < 0) RETURNINT(loadings_num);
    
    struct Matrix loadings = *(struct Matrix *) allVectors[loadings_num];
    
    // Perform factor analysis
    status = factor_analysis(&data, &loadings, factors);
    if (status != MATRIX_SUCCESS) {
        freeMatrix(loadings_num);
        RETURNINTX(status);
    }
    
    // Apply rotation if requested
    if (rotate != ROTATE_NONE) {
        status = factor_rotation(&loadings, rotate, 100);
        if (status != MATRIX_SUCCESS) {
            freeMatrix(loadings_num);
            RETURNINTX(status);
        }
    }
    
    // Calculate diagnostics if requested
    int diagnostics=2;
    if (diagnostics>0) {
        int diag_num = matcreate(4, max(data.cols, factors), 1, "Diagnostics");
        if (diag_num < 0) {
            freeMatrix(loadings_num);
            RETURNINTX(diag_num);
        }
        
        struct Matrix diag = *(struct Matrix *) allVectors[diag_num];
        status = calculate_basic_diagnostics(&data, &loadings, &diag);
        if (status != MATRIX_SUCCESS) {
            freeMatrix(loadings_num);
            freeMatrix(diag_num);
            RETURNINTX(status);
        }
        
        // Check adequacy
        int adequacy = check_factor_adequacy(&diag);
        if (adequacy) {
            printf("Warning: Factor solution may be inadequate:\n");
            if (adequacy & 1) printf("  - Some variables poorly explained (communality < 0.3)\n");
            if (adequacy & 2) printf("  - Insufficient total variance explained (< 60%%)\n");
        }
        
        // Create interpretation matrix if requested
        if (diagnostics==2) {
            int interp_num = matcreate(data.cols, factors, 1, "Diagnostic details");
            if (interp_num >= 0) {
                struct Matrix interp = *(struct Matrix *) allVectors[interp_num];
                interpret_loadings(&loadings, &interp, 0.4);  // 0.4 is typical threshold
            }
        }
    }
    
    // Calculate factor scores if requested
    if (scores) {
        char scorenote[32];
        sprintf(scorenote,"%s %s","Score:",GETSTRING(ARG4));
        int scores_num = matcreate(data.rows, factors, 1, scorenote);
        if (scores_num < 0) {
            freeMatrix(loadings_num);
            RETURNINTX(scores_num);
        }
        
        struct Matrix scores_mat = *(struct Matrix *) allVectors[scores_num];
        status = calculate_factor_scores(&data, &loadings, &scores_mat);
        if (status != MATRIX_SUCCESS) {
            freeMatrix(loadings_num);
            freeMatrix(scores_num);
            RETURNINTX(status);
        }
    }
    
    RETURNINT(loadings_num);
ENDPROC
}

// Additional statistical utility functions
double calculate_column_median(struct Matrix* matrix, int col) {
    int i,j, mid;
    double* values = (double*)MATRIX_ALLOC(matrix->rows * sizeof(double));
    double median;
    
    if (!values) return 0.0;
    
    // Copy column values
    for (i = 0; i < matrix->rows; i++) {
        values[i] = matp(matrix,i,col);
    }
    
    // Simple bubble sort (for small datasets)
    // TODO: Replace with quickselect for better performance on large datasets
    for (i = 0; i < matrix->rows - 1; i++) {
        for (j = 0; j < matrix->rows - i - 1; j++) {
            if (values[j] > values[j + 1]) {
                double temp = values[j];
                values[j] = values[j + 1];
                values[j + 1] = temp;
            }
        }
    }
    
    // Calculate median
    if (matrix->rows % 2 == 0) {
        mid = matrix->rows / 2;
        median = (values[mid-1] + values[mid]) / 2.0;
    } else {
        mid = matrix->rows / 2;
        median = values[mid];
    }
    
    MATRIX_FREE(values);
    return median;
}

double calculate_column_skewness(struct Matrix* matrix, int col) {
    int i;
    double mean = calculate_column_mean(matrix, col);
    double stddev = calculate_column_stddev(matrix, col, mean);
    double sum = 0.0;
    
    if (stddev < MATRIX_EPSILON) return 0.0;
    
    for (i = 0; i < matrix->rows; i++) {
        double diff = (matp(matrix,i,col) - mean) / stddev;
        sum += diff * diff * diff;
    }
    
    return sum / matrix->rows;
}

double calculate_column_kurtosis(struct Matrix* matrix, int col) {
    int i;
    double mean = calculate_column_mean(matrix, col);
    double stddev = calculate_column_stddev(matrix, col, mean);
    double sum = 0.0;
    
    if (stddev < MATRIX_EPSILON) return 0.0;
    
    for (i = 0; i < matrix->rows; i++) {
        double diff = (matp(matrix,i,col) - mean) / stddev;
        sum += diff * diff * diff * diff;
    }
    
    return sum / matrix->rows - 3.0;  // Excess kurtosis (normal = 0)
}

// Enhanced column statistics with more metrics
PROCEDURE(mcolstats) {
    int i, matnum, status;
    
    status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);
    
    struct Matrix* matrix = (struct Matrix*)allVectors[GETINT(ARG0)];
    
    // Create matrix for stats (5 rows: means, stddevs, medians, skewness, kurtosis)
    matnum = matcreate(5, matrix->cols, 1, GETSTRING(ARG1));
    if (matnum < 0) RETURNINT(matnum);
    
    struct Matrix* stats = (struct Matrix*)allVectors[matnum];
    
    #pragma omp parallel for if(matrix->cols >= 4)
    for (i = 0; i < matrix->cols; i++) {
        double mean = calculate_column_mean(matrix, i);
        matp(stats,0,i) = mean;                                    // Mean
        matp(stats,1,i) = calculate_column_stddev(matrix, i, mean);// StdDev
        matp(stats,2,i) = calculate_column_median(matrix, i);      // Median
        matp(stats,3,i) = calculate_column_skewness(matrix, i);    // Skewness
        matp(stats,4,i) = calculate_column_kurtosis(matrix, i);    // Kurtosis
    }
    
    RETURNINT(matnum);
}

// Optimized covariance calculation using blocking
double calculate_covariance_blocked(struct Matrix* matrix, int col1, int col2) {
    int i, block;
    double mean1 = calculate_column_mean(matrix, col1);
    double mean2 = calculate_column_mean(matrix, col2);
    double sum = 0.0;
    const int BLOCK_SIZE = 64;  // Cache-friendly block size
    
    for (block = 0; block < matrix->rows; block += BLOCK_SIZE) {
        double local_sum = 0.0;
        int end = MIN(block + BLOCK_SIZE, matrix->rows);
        
        for (i = block; i < end; i++) {
            local_sum += (matp(matrix,i,col1) - mean1) * 
                        (matp(matrix,i,col2) - mean2);
        }
        sum += local_sum;
    }
    
    return sum / (matrix->rows - 1);
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
    ADDPROC(mfactor,   "matrix.mfactor",   "b",  ".int", "m0=.int, factors=.int, rotation=.int, scores=.int, mid=.string");
    ADDPROC(mcolstats, "matrix.mcolstats", "b",  ".int", "m0=.int, mid=.string");
ENDLOADFUNCS