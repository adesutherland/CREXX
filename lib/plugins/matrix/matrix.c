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
    #define MATRIX_ALLOC_HEAP(size) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size)
    #define MATRIX_FREE_HEAP(ptr) HeapFree(GetProcessHeap(), 0, ptr)
    HANDLE hHeap = NULL;
    #define GNUPLOT_DEFAULT_PATH "C:\\Program Files\\gnuplot\\bin\\gnuplot.exe"
#else
    #include <stdlib.h>
    #define MATRIX_ALLOC_HEAP(size) calloc(1, size)
    #define MATRIX_FREE_HEAP(ptr) free(ptr)
    #define GNUPLOT_DEFAULT_PATH "gnuplot"
#endif
// for simple storage allocation
#define MATRIX_ALLOC(size) malloc(size)
#define MATRIX_FREE(ptr) free(ptr)

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

// Plot types for factor analysis
#define PLOT_SCREEN   "screen"
#define PLOT_LOADINGS "loadings"
#define PLOT_BIPLOT   "biplot"

// ASCII plot types and constants
#define ASCII_HIST   1    // Histogram
#define ASCII_BAR    2    // Bar chart
#define ASCII_LINE   3    // Line plot
#define ASCII_HEAT   4    // Heat map
#define ASCII_SCATTER 5   // Scatter plot
#define ASCII_BOX    6    // Box plot

struct Matrix {
    double* CBselfref;     // CB self reference
    char id[32];
    int rows;
    int cols;
    double * vector;  // Pointer to the matrix data
};
struct FactorOptions {
    int score;
    int rotate;
    int diag;
    char option1[16];
    char option2[16];
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
    
    void *pMemory = MATRIX_ALLOC_HEAP(bytes);
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
    printf("Matrix %d: %s, dimension: %dx%d\n",matname, matrix.id, matrix.rows, matrix.cols);
    for (i = 0; i < matrix.rows; ++i) {
        for (j = 0; j < matrix.cols; ++j) {
            printf("%10.4f ", matrix.vector[i * matrix.cols + j]);
        }
        printf("\n");
    }
}

// Comparison function for qsort
static int compare_doubles(const void* a, const void* b) {
    double diff = *(const double*)a - *(const double*)b;
    return (diff > 0) - (diff < 0);
}

// Helper function for box plot calculations
void calculate_box_stats(double* data, int n, double* min, double* q1, 
                        double* median, double* q3, double* max) {
    // Sort data
    qsort(data, n, sizeof(double), compare_doubles);
    
    *min = data[0];
    *max = data[n-1];
    *median = n % 2 ? data[n/2] : (data[n/2-1] + data[n/2]) / 2;
    *q1 = data[n/4];
    *q3 = data[3*n/4];
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
// Calculate correlation between two columns
double calculate_correlation(struct Matrix* matrix, int col1, int col2) {
    int i;
    double mean1 = 0.0, mean2 = 0.0;
    double std1 = 0.0, std2 = 0.0;
    double covar = 0.0;
    
    // Calculate means
    mean1 = calculate_column_mean(matrix, col1);
    mean2 = calculate_column_mean(matrix, col2);
    
    // Calculate covariance and standard deviations
    for (i = 0; i < matrix->rows; i++) {
        double diff1 = matp(matrix,i,col1) - mean1;
        double diff2 = matp(matrix,i,col2) - mean2;
        
        covar += diff1 * diff2;
        std1 += diff1 * diff1;
        std2 += diff2 * diff2;
    }
    
    std1 = sqrt(std1 / (matrix->rows - 1));
    std2 = sqrt(std2 / (matrix->rows - 1));
    covar = covar / (matrix->rows - 1);
    
    // Handle zero standard deviations
    if (std1 < MATRIX_EPSILON || std2 < MATRIX_EPSILON) {
        return 0.0;
    }
    
    return covar / (std1 * std2);
}
/*
int score;
int rotate;
int diagnostics;
char option1[16];
char option2[16];
*/
void to_uppercase(char str[]) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        str[i] = toupper((unsigned char)str[i]); // Convert each character to uppercase
    }
}
void parse_option_parms(const char* options, struct FactorOptions * fOptions) {
    char* options_copy = strdup(options); // Create a copy of the string
    to_uppercase(options_copy);
    char* working_copy = options_copy;    // Keep a pointer to the start for freeing later
        char* token;
        while ((token = strtok_r(working_copy, ",", &working_copy)) != NULL) {
         // Check if this token contains an '='
            char* equals_pos = strchr(token, '=');
            if (equals_pos != NULL) {
                // This is a key-value pair
                *equals_pos = '\0';  // Split the string at '='
                char* key = token;
                char* value = equals_pos + 1;
                if (strcmp("ROTATE", key) == 0) {
                    if (strncmp("PROMAX", value, 3) == 0) {
                        fOptions->rotate = ROTATE_PROMAX;
                    } else if (strncmp("VARIMAX", value, 3) == 0) {
                        fOptions->rotate = ROTATE_VARIMAX;
                    } else if (strncmp("QUARTIMAX", value, 3) == 0) {
                        fOptions->rotate = ROTATE_QUARTIMAX;
                    } else if (strncmp("NONE", value, 2) == 0) {
                        fOptions->rotate = ROTATE_NONE;
                    }
                } else if (strcmp("DIAG", key) == 0) {
                    fOptions->diag = atoi(value);
                }
            } else { // This is a standalone option
                if (strncmp("SCORE", token,3) == 0) {
                    fOptions->score = 1;
                } else if (strncmp("NSCORE", token,3) == 0) {
                    fOptions->score = 0;
                }
            }
        }

        free(options_copy);
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

PROCEDURE(mget) {
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

    RETURNFLOAT(mat(cmatrix,row,col));
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
    
    if (!MATRIX_FREE_HEAP(matrix->vector)) {
        return -8;
    }
    
    if (!MATRIX_FREE_HEAP(matrix->CBselfref)) {
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
            MATRIX_FREE_HEAP(temp);
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
    
    int U_num = matcreate(matrix.rows, matrix.cols, 1,GETSTRING(ARG2));
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

double power_method(struct Matrix* A, double* vector, int n, int max_iter, double tolerance) {
    double* temp = (double*)malloc(n * sizeof(double));
    int i,j,iter;
    if (temp == NULL) return 0.0;

    double eigenvalue = 0.0;
    double prev_eigenvalue = 0.0;

    for (iter = 0; iter < max_iter; iter++) {
        // Multiply matrix A with vector
        for (i = 0; i < n; i++) {
            temp[i] = 0.0;
            for (j = 0; j < n; j++) {
                temp[i] += matp(A, i, j) * vector[j];
            }
        }

        // Calculate eigenvalue (Rayleigh quotient)
        double numerator = 0.0;
        double denominator = 0.0;
        for (i = 0; i < n; i++) {
            numerator += temp[i] * vector[i];
            denominator += vector[i] * vector[i];
        }
        eigenvalue = numerator / denominator;

        // Normalize the vector
        double norm = 0.0;
        for (i = 0; i < n; i++) {
            norm += temp[i] * temp[i];
        }
        norm = sqrt(norm);
        for (i = 0; i < n; i++) {
            vector[i] = temp[i] / norm;
        }

        // Check for convergence
        if (fabs(eigenvalue - prev_eigenvalue) < tolerance) {
            break;
        }
        prev_eigenvalue = eigenvalue;
    }

    free(temp);
    return eigenvalue;
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
    
    status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);
    
    struct Matrix* matrix = (struct Matrix*)allVectors[GETINT(ARG0)];
    
    // Create correlation matrix
    matnum = matcreate(matrix->cols, matrix->cols,1, GETSTRING(ARG1));
    if (matnum < 0) RETURNINT(matnum);
    
    struct Matrix* corr = (struct Matrix*)allVectors[matnum];
    
    // Calculate correlations
    for (i = 0; i < matrix->cols; i++) {
        matp(corr,i,i) = 1.0;  // Diagonal is always 1
        for (j = i + 1; j < matrix->cols; j++) {
            double correlation = calculate_correlation(matrix, i, j);
            matp(corr,i,j) = correlation;
            matp(corr,j,i) = correlation;  // Correlation matrix is symmetric
        }
    }
    
    RETURNINT(matnum);
}

// Calculate row/column means
PROCEDURE(mmean) {
    int i, j, matnum, status;
    int axis = GETINT(ARG1)-1;     // column number

    status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);

    struct Matrix *matrix = (struct Matrix *) allVectors[GETINT(ARG0)];

    RETURNFLOATX(calculate_column_mean(matrix, axis));
ENDPROC
}

// Calculate standard deviation for matrix rows or columns
PROCEDURE(mstdev) {
    int i, j, matnum, status;
    double mean;
    int axis = GETINT(ARG1)-1;     // column number

    status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);

    struct Matrix *matrix = (struct Matrix *) allVectors[GETINT(ARG0)];
    mean=calculate_column_mean(matrix, axis);
    RETURNFLOATX(calculate_column_stddev(matrix, axis,mean));
ENDPROC
}

// Function to validate the quality of factor analysis
void validate_factor_analysis(struct Matrix* loadings, int factors, struct Matrix* data) {
    int i, j;
    int cols = data->cols;

    // Check communalities
    printf("Communalities:\n");
    for (i = 0; i < cols; i++) {
        double communality = 0.0;
        for (j = 0; j < factors; j++) {
            communality += pow(matp(loadings, i, j), 2); // Sum of squared loadings
        }
        printf("Variable %d: Communality = %.4f\n", i + 1, communality);
    }

    // Check factor loadings
    printf("\nFactor Loadings:\n");
    for (i = 0; i < cols; i++) {
        for (j = 0; j < factors; j++) {
            double loading = matp(loadings, i, j);
            printf("Variable %d, Factor %d: Loading = %.4f\n", i + 1, j + 1, loading);
            if (fabs(loading) < 0.4) {
                printf("Warning: Low loading for Variable %d on Factor %d\n", i + 1, j + 1);
            }
        }
    }

    // Calculate total variance explained
    double total_variance = 0.0;
    for (j = 0; j < factors; j++) {
        double eigenvalue = 0.0; // Assume you have a way to get eigenvalues
        // eigenvalue = get_eigenvalue(j); // Implement this function
        total_variance += eigenvalue;
    }
    printf("\nTotal Variance Explained by Factors: %.4f\n", total_variance);
}

// Function to perform factor analysis
int factor_analysis(struct Matrix* data, struct Matrix* loadings, int factors) {
    int i, j, k;
    int rows = data->rows;
    int cols = data->cols;
    
    // Validate input dimensions
    if (factors <= 0 || factors > cols) {
        printf("Invalid number of factors requested: %d\n", factors);
        return -1;
    }
    
    // Create correlation matrix
    int corr_num = matcreate(cols, cols, 1, "temp_corr");
    if (corr_num < 0) return corr_num;
    struct Matrix* corr = (struct Matrix*)allVectors[corr_num];
    
    // Calculate correlation matrix with validation
    for (i = 0; i < cols; i++) {
        matp(corr, i, i) = 1.0;
        for (j = i + 1; j < cols; j++) {
            double correlation = calculate_correlation(data, i, j);
            if (isnan(correlation)) {
                printf("Warning: NaN correlation detected at (%d,%d)\n", i, j);
                correlation = 0.0;
            }
            matp(corr, i, j) = correlation;
            matp(corr, j, i) = correlation;
        }
    }
    
    // Allocate memory
    double* eigenvalues = (double*)calloc(factors, sizeof(double));
    double* eigenvector = (double*)calloc(cols, sizeof(double));
    
    if (eigenvalues == NULL || eigenvector == NULL) {
        printf("Memory allocation failed\n");
        freeMatrix(corr_num);
        free(eigenvalues);
        free(eigenvector);
        return -1;
    }

    // Initialize loadings matrix to zero
    for (i = 0; i < loadings->rows; i++) {
        for (j = 0; j < loadings->cols; j++) {
            matp(loadings, i, j) = 0.0;
        }
    }

    // Extract factors
    for (i = 0; i < factors; i++) {
        // Initialize eigenvector
        double norm = 0.0;
        for (j = 0; j < cols; j++) {
            eigenvector[j] = 1.0 / sqrt(cols); // Start with uniform values
            norm += eigenvector[j] * eigenvector[j];
        }
        norm = sqrt(norm);
        for (j = 0; j < cols; j++) {
            eigenvector[j] /= norm;
        }

        // Apply power method with validation
        double eigenvalue = power_method(corr, eigenvector, cols, 1000, 1e-10);
        
        // Validate eigenvalue
        if (isnan(eigenvalue) || eigenvalue < 1e-10) {
            printf("Warning: Invalid eigenvalue %f for factor %d\n", eigenvalue, i);
            continue;
        }
        
        eigenvalues[i] = eigenvalue;

        // Store loadings with validation
        for (j = 0; j < cols; j++) {
            double loading = eigenvector[j] * sqrt(fabs(eigenvalue));
            if (!isnan(loading)) {
                matp(loadings, j, i) = loading;
            } else {
                printf("Warning: NaN loading detected at (%d,%d)\n", j, i);
                matp(loadings, j, i) = 0.0;
            }
        }

        // Deflate correlation matrix
        for (j = 0; j < cols; j++) {
            for (k = 0; k < cols; k++) {
                double deflation = eigenvalue * eigenvector[j] * eigenvector[k];
                if (!isnan(deflation)) {
                    matp(corr, j, k) -= deflation;
                }
            }
        }

        // Regularize diagonal
        for (j = 0; j < cols; j++) {
            if (matp(corr, j, j) < 1e-10) {
                matp(corr, j, j) = 1e-10;
            }
        }
    }

    // Verify results
    printf("Eigenvalues: ");
    for (i = 0; i < factors; i++) {
        printf("%f ", eigenvalues[i]);
    }
    printf("\n");

    // Clean up
    free(eigenvalues);
    free(eigenvector);
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
    u =  (double*)MATRIX_ALLOC(p * sizeof(double));
    v =  (double*)MATRIX_ALLOC(p * sizeof(double));
    A =  (double*)MATRIX_ALLOC(p * sizeof(double));
    B =  (double*)MATRIX_ALLOC(p * sizeof(double));

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
    int pattern_num = matcreate(p, m,1,"pattern");
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

// Enhanced adequacy checks for factor analysis
int check_factor_adequacy(struct Matrix* diag, struct Matrix* data, struct Matrix* loadings) {
    int inadequate = 0;
    int i, j;

    // Check communalities (row 0)
    for (i = 0; i < diag->cols; i++) {
        if (matp(diag,0,i) < 0.3) {
            inadequate |= 1;
            break;
        }
    }

    // Check cumulative variance (row 3)
    if (matp(diag,3,diag->cols-1) < 0.6) {
        inadequate |= 2;
    }

    // Check for cross-loadings
    for (i = 0; i < loadings->rows; i++) {
        int significant_loadings = 0;
        for (j = 0; j < loadings->cols; j++) {
            if (fabs(matp(loadings,i,j)) > 0.4) {
                significant_loadings++;
            }
        }
        if (significant_loadings > 1) {
            inadequate |= 4;  // Cross-loading detected
            break;
        }
    }

    // Check sample size adequacy
    if (data->rows < 5 * data->cols) {
        inadequate |= 8;  // Less than 5 cases per variable
    }

    // Check for extreme correlations using calculate_correlation
    for (i = 0; i < data->cols; i++) {
        for (j = i + 1; j < data->cols; j++) {
            double corr = calculate_correlation(data, i, j);
            if (fabs(corr) > 0.9) {
                inadequate |= 16;  // Potential multicollinearity
                break;
            }
        }
        if (inadequate & 16) break;
    }

    return inadequate;
}

// Create visualization data for screenplot
int create_screen_data(struct Matrix* diag, char* id) {
    int i;
    int matnum = matcreate(2, diag->cols, 1, id);
    if (matnum < 0) return matnum;

    struct Matrix* screen= (struct Matrix*)allVectors[matnum];

    // Row 0: Factor number (1-based)
    // Row 1: Eigenvalues
    for (i = 0; i < diag->cols; i++) {
        matp(screen,0,i) = i + 1;
        matp(screen,1,i) = matp(diag,1,i);  // Copy eigenvalues
    }

    return matnum;
}

// Enhanced interpretation with more details
int create_detailed_interpretation(struct Matrix* loadings, struct Matrix* diag,
                                char* id) {
    int i, j;
    int p = loadings->rows;
    int m = loadings->cols;

    // Create interpretation matrix with additional rows for metrics
    int matnum = matcreate(p + 3, m + 2,1 ,id);
    if (matnum < 0) return matnum;

    struct Matrix* interp = (struct Matrix*)allVectors[matnum];

    // Variable loadings and metrics
    for (i = 0; i < p; i++) {
        // Loadings
        for (j = 0; j < m; j++) {
            double loading = matp(loadings,i,j);
            matp(interp,i,j) = loading;
        }

        // Communality
        matp(interp,i,m) = matp(diag,0,i);

        // Complexity (number of significant loadings)
        int complexity = 0;
        for (j = 0; j < m; j++) {
            if (fabs(matp(loadings,i,j)) > 0.4) complexity++;
        }
        matp(interp,i,m+1) = complexity;
    }

    // Factor statistics
    for (j = 0; j < m; j++) {
        matp(interp,p,j) = matp(diag,1,j);      // Eigenvalues
        matp(interp,p+1,j) = matp(diag,2,j);    // Proportion
        matp(interp,p+2,j) = matp(diag,3,j);    // Cumulative
    }

    return matnum;
}


// Updated REXX procedure
PROCEDURE(mfactor) {
    int status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);
    struct FactorOptions options={0};
    struct Matrix data = *(struct Matrix *) allVectors[GETINT(ARG0)];
    char * parse=GETSTRING(ARG3);
    int factors = GETINT(ARG1);
    int rotate  = 0;  // Rotation method
    int scores  = 0;  // 0 = no scores, 1 = calculate scores
    printf("parse %s\n",parse);
    parse_option_parms(parse, &options);
    // Create matrix for factor loadings
    int loadings_num = matcreate(data.cols, factors, 3,GETSTRING(ARG2));  // later maybe 2. matrix needed
    if (loadings_num < 0) RETURNINT(loadings_num);

    struct Matrix loadings = *(struct Matrix *) allVectors[loadings_num];

    // Perform factor analysis
    status = factor_analysis(&data, &loadings, factors);
    if (status != MATRIX_SUCCESS) {
        freeMatrix(loadings_num);
        RETURNINTX(status);
    }

    // Apply rotation if requested
    if (options.rotate != ROTATE_NONE) {
        status = factor_rotation(&loadings, options.rotate, 100);
        if (status != MATRIX_SUCCESS) {
            freeMatrix(loadings_num);
            RETURNINTX(status);
        }
    }

    // Calculate diagnostics if requested
    int diagnostics=3;
    if (diagnostics>0) {
        int diag_num = matcreate(4, max(data.cols, factors), 1,"Diagnostics");
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

        // Check adequacy with enhanced checks
        int adequacy = check_factor_adequacy(&diag, &data, &loadings);
        if (adequacy) {
            printf("Factor Analysis Diagnostics:\n");
            if (adequacy & 1) printf("WARNING: Some variables poorly explained (communality < 0.3)\n");
            if (adequacy & 2) printf("WARNING: Insufficient total variance explained (< 60%%)\n");
            if (adequacy & 4) printf("WARNING: Cross-loadings detected (variables load on multiple factors)\n");
            if (adequacy & 8) printf("WARNING: Sample size may be inadequate (< 5 cases per variable)\n");
            if (adequacy & 16) printf("WARNING: Potential multicollinearity detected\n");
        }

        // Create screenplot data if requested
        if (diagnostics>=2) {
            create_screen_data(&diag, "Diagnostics details I");
        }

        // Create detailed interpretation if requested
        if (diagnostics>=3) {
            create_detailed_interpretation(&loadings, &diag, "Diagnostics details II");
        }
    }

    // Calculate factor scores if requested
    if (options.score) {
        char scorenote[32];
        sprintf(scorenote,"%s %s","Score:",GETSTRING(ARG4));
        int scores_num = matcreate(data.rows, factors, 1,scorenote);
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
    matnum = matcreate(5, matrix->cols, 1,GETSTRING(ARG1));
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

PROCEDURE(stats) {
    int i, status;
    int matnum=GETINT(ARG0);
    char * mode=GETSTRING(ARG1);

    status = validateMatrix(matnum);
    if (status != MATRIX_VALID) RETURNINT(-1);

    struct Matrix* matrix = (struct Matrix*)allVectors[GETINT(ARG0)];
    if(mode[0]=='R' || mode[0] == 'r')  RETURNINT(matrix->rows);
    else  RETURNINT(matrix->cols);
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


// Function to create an augmented matrix with an intercept term
PROCEDURE(mexpand) {
    int matnum=GETINT(ARG0);
    int newcols=GETINT(ARG1);
    double init=GETFLOAT(ARG2);
    int status = validateMatrix(matnum);
    if (status != MATRIX_VALID) RETURNINTX(-1);
    int i,j,k;

    struct Matrix* original = (struct Matrix*)allVectors[matnum];
   // int Xrows = original->rows;
   // int Xcols = original->cols;

    // Create the augmented matrix with an additional column for the intercept
    int matnew = matcreate(original->rows, original->cols + newcols,1, "Augmented Matrix");
    struct Matrix* augmented= (struct Matrix*)allVectors[matnew];
    if (augmented == NULL) {
        RETURNINTX(-2); // Return NULL if matrix creation fails
    }
    // Fill the augmented matrix
    for (i = 0; i < original->rows; i++) {
        for(k=0;k<newcols;k++) {
            matp(augmented, i,k) = init; // Set intercept term to 1.0
        }
        for (j = 0; j < original->cols; j++) {
            matp(augmented, i, j + newcols) = matp(original, i, j); // Copy original X values
        }
    }
    RETURNINTX(matnew);   // Return the augmented matrix
    ENDPROC
}
// Basic matrix plotting functions
PROCEDURE(mplot) {
    int status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);

    struct Matrix* matrix = (struct Matrix*)allVectors[GETINT(ARG0)];
    char* plot_type = GETSTRING(ARG1);

    // Construct gnuplot command with full path
    char cmd[256];
    #ifdef _WIN32
        snprintf(cmd, sizeof(cmd), "\"%s\" -persist", GNUPLOT_DEFAULT_PATH);
        FILE* gnuplot = _popen(cmd, "w");
    #else
        snprintf(cmd, sizeof(cmd), "%s -persist", GNUPLOT_DEFAULT_PATH);
        FILE* gnuplot = popen(cmd, "w");
    #endif

    if (!gnuplot) {
        printf("Error: Could not open gnuplot at: %s\n", GNUPLOT_DEFAULT_PATH);
        printf("Please check if gnuplot is installed at this location.\n");
        RETURNINT(-1);
    }

    // Create temporary data file
    FILE* temp = fopen("temp_plot.dat", "w");
    if (!temp) {
        pclose(gnuplot);
        printf("Error: Could not create temporary file\n");
        RETURNINT(-2);
    }

    int i, j;

    // Write data to temporary file
    if (strcmp(plot_type, "heatmap") == 0) {
        // Heatmap format: x y value
        for (i = 0; i < matrix->rows; i++) {
            for (j = 0; j < matrix->cols; j++) {
                fprintf(temp, "%d %d %g\n", j, i, matp(matrix,i,j));
            }
            fprintf(temp, "\n");
        }
    } else {
        // Line/scatter/bar format: row_index value
        for (i = 0; i < matrix->rows; i++) {
            for (j = 0; j < matrix->cols; j++) {
                fprintf(temp, "%d %g\n", j, matp(matrix,i,j));
            }
            fprintf(temp, "\n\n");  // Double newline separates data series
        }
    }
    fclose(temp);

    // Set up gnuplot commands
    if (strcmp(plot_type, "heatmap") == 0) {
        fprintf(gnuplot, "set view map\n");
        fprintf(gnuplot, "set palette defined (0 'blue', 0.5 'white', 1 'red')\n");
        fprintf(gnuplot, "splot 'temp_plot.dat' matrix with image\n");
    } else {
        fprintf(gnuplot, "plot ");
        for (i = 0; i < matrix->rows; i++) {
            if (i > 0) fprintf(gnuplot, ", ");
            fprintf(gnuplot, "'temp_plot.dat' index %d ", i);

            if (strcmp(plot_type, "line") == 0) {
                fprintf(gnuplot, "with lines");
            } else if (strcmp(plot_type, "scatter") == 0) {
                fprintf(gnuplot, "with points");
            } else if (strcmp(plot_type, "bar") == 0) {
                fprintf(gnuplot, "with boxes");
            }
        }
        fprintf(gnuplot, "\n");
    }

    pclose(gnuplot);
    remove("temp_plot.dat");

    RETURNINT(0);
}

// Helper function to detect elbow point in screenplot
int detect_elbow(struct Matrix* eigenvalues) {
    int i;
    double max_angle = 0;
    int elbow_point = 1;

    // Need at least 3 points to detect elbow
    if (eigenvalues->rows < 3) return 1;

    // Calculate angles between consecutive line segments
    for (i = 1; i < eigenvalues->rows - 1; i++) {
        double x1 = i - 1;
        double y1 = matp(eigenvalues,i-1,0);
        double x2 = i;
        double y2 = matp(eigenvalues,i,0);
        double x3 = i + 1;
        double y3 = matp(eigenvalues,i+1,0);

        // Calculate vectors
        double v1x = x2 - x1;
        double v1y = y2 - y1;
        double v2x = x3 - x2;
        double v2y = y3 - y2;

        // Calculate angle
        double angle = atan2(v1x * v2y - v1y * v2x, v1x * v2x + v1y * v2y);
        if (fabs(angle) > fabs(max_angle)) {
            max_angle = angle;
            elbow_point = i;
        }
    }

    return elbow_point;
}

// Enhanced plotting for factor analysis
PROCEDURE(mfaplot) {
    int status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);

    struct Matrix* matrix = (struct Matrix*)allVectors[GETINT(ARG0)];
    char* plot_type = GETSTRING(ARG1);

    #ifdef _WIN32
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "\"\"%s\" -persist\"", GNUPLOT_DEFAULT_PATH);
        FILE* gnuplot = _popen(cmd, "w");
    #else
        FILE* gnuplot = popen("gnuplot -persist", "w");
    #endif

    if (!gnuplot) {
        printf("Error: Could not open gnuplot\n");
        RETURNINT(-1);
    }

    // Create temporary data file
    FILE* temp = fopen("temp_plot.dat", "w");
    if (!temp) {
        pclose(gnuplot);
        RETURNINT(-2);
    }

    if (strcmp(plot_type, PLOT_SCREEN) == 0) {
        // screenplot with elbow detection
        int elbow = detect_elbow(matrix);
        int i;
        // Write eigenvalues
        for (i = 0; i < matrix->rows; i++) {
            fprintf(temp, "%d %g\n", i + 1, matp(matrix,i,0));
        }
        fclose(temp);

        // Set up screenplot
        fprintf(gnuplot, "set title 'screenPlot with Elbow Point'\n");
        fprintf(gnuplot, "set xlabel 'Factor Number'\n");
        fprintf(gnuplot, "set ylabel 'Eigenvalue'\n");
        fprintf(gnuplot, "set grid\n");
        fprintf(gnuplot, "set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7 ps 1.5\n");
        fprintf(gnuplot, "set style line 2 lc rgb '#dd181f' lt 1 lw 2 pt 7 ps 2.0\n");
        fprintf(gnuplot, "plot 'temp_plot.dat' index 0 with linespoints ls 1 notitle, \\\n");
        fprintf(gnuplot, "     'temp_plot.dat' using 1:($0+1==%d ? $2 : 1/0) with points ls 2 title 'Elbow Point'\n", elbow);

    } else if (strcmp(plot_type, PLOT_LOADINGS) == 0) {
        // Factor loadings plot (first two factors)
        if (matrix->cols < 2) {
            printf("Error: Need at least 2 factors for loadings plot\n");
            fclose(temp);
            pclose(gnuplot);
            RETURNINT(-3);
        }
        int i;
        // Write loadings
        for (i = 0; i < matrix->rows; i++) {
            fprintf(temp, "%g %g\n", matp(matrix,i,0), matp(matrix,i,1));
        }
        fclose(temp);

        // Set up loadings plot
        fprintf(gnuplot, "set title 'Factor Loadings Plot'\n");
        fprintf(gnuplot, "set xlabel 'Factor 1'\n");
        fprintf(gnuplot, "set ylabel 'Factor 2'\n");
        fprintf(gnuplot, "set grid\n");
        fprintf(gnuplot, "set size square\n");
        fprintf(gnuplot, "set xrange [-1:1]\n");
        fprintf(gnuplot, "set yrange [-1:1]\n");
        fprintf(gnuplot, "plot 'temp_plot.dat' with points pt 7 ps 1.5 notitle, \\\n");
        fprintf(gnuplot, "     circle(x,y) = sqrt(1-x**2), \\\n");
        fprintf(gnuplot, "     '-' with lines lt 0 notitle\n");
        fprintf(gnuplot, "-1 0\n1 0\n\n0 -1\n0 1\n\ne\n");

    } else if (strcmp(plot_type, PLOT_BIPLOT) == 0) {
        // Biplot (requires scores as second matrix)
/*      if (ARGLEN(2) == 0) {
            printf("Error: Biplot requires factor scores matrix as second argument\n");
            fclose(temp);
            pclose(gnuplot);
            RETURNINT(-4);
        }
*/

        status = validateMatrix(GETINT(ARG2));
        if (status != MATRIX_VALID) {
            fclose(temp);
            pclose(gnuplot);
            RETURNINT(status);
        }

        struct Matrix* scores = (struct Matrix*)allVectors[GETINT(ARG2)];

        // Write loadings and scores
        fprintf(temp, "# Loadings\n");
        int i;
        for (i = 0; i < matrix->rows; i++) {
            fprintf(temp, "%g %g\n", matp(matrix,i,0), matp(matrix,i,1));
        }
        fprintf(temp, "\n\n# Scores\n");
        for (i = 0; i < scores->rows; i++) {
            fprintf(temp, "%g %g\n", matp(scores,i,0), matp(scores,i,1));
        }
        fclose(temp);

        // Set up biplot
        fprintf(gnuplot, "set title 'Biplot'\n");
        fprintf(gnuplot, "set xlabel 'Factor 1'\n");
        fprintf(gnuplot, "set ylabel 'Factor 2'\n");
        fprintf(gnuplot, "set grid\n");
        fprintf(gnuplot, "plot 'temp_plot.dat' index 0 with points pt 7 ps 1.5 title 'Loadings', \\\n");
        fprintf(gnuplot, "     'temp_plot.dat' index 1 with points pt 6 ps 1.0 title 'Scores'\n");
    }

    pclose(gnuplot);
    remove("temp_plot.dat");

    RETURNINT(0);
}

// Helper function to find min/max values
void get_matrix_range(struct Matrix* matrix, double* min_val, double* max_val) {
    int i, j;
    *min_val = *max_val = matp(matrix,0,0);

    for (i = 0; i < matrix->rows; i++) {
        for (j = 0; j < matrix->cols; j++) {
            double val = matp(matrix,i,j);
            if (val < *min_val) *min_val = val;
            if (val > *max_val) *max_val = val;
        }
    }
}

// Helper function to get plot type from string
int get_plot_type(const char* type_str) {
    if (strcmp(type_str, "hist") == 0) return ASCII_HIST;
    if (strcmp(type_str, "bar") == 0)  return ASCII_BAR;
    if (strcmp(type_str, "line") == 0) return ASCII_LINE;
    if (strcmp(type_str, "heat") == 0) return ASCII_HEAT;
    if (strcmp(type_str, "scatter") == 0) return ASCII_SCATTER;
    if (strcmp(type_str, "box") == 0) return ASCII_BOX;
    return -1;
}

// ASCII plotting function with new plot types
PROCEDURE(masciiplot) {
    int status = validateMatrix(GETINT(ARG0));
    if (status != MATRIX_VALID) RETURNINT(status);

    struct Matrix* matrix = (struct Matrix*)allVectors[GETINT(ARG0)];
    int plot_type = get_plot_type(GETSTRING(ARG1));

    if (plot_type < 0) {
        printf("Error: Invalid plot type. Use 'hist', 'bar', 'line', 'heat', 'scatter', or 'box'\n");
        RETURNINT(-1);
    }

    const int WIDTH = 60;   // Plot width in characters
    const int HEIGHT = 20;  // Plot height in characters
    double min_val, max_val;
    get_matrix_range(matrix, &min_val, &max_val);

    // Ensure we have a reasonable range
    if (fabs(max_val - min_val) < MATRIX_EPSILON) {
        max_val = min_val + 1.0;
    }

    if (plot_type == ASCII_HIST) {
        // Simple histogram
        int bins[60] = {0};  // Use fixed size instead of WIDTH
        int max_count = 0;
        int i, j;

        // Fill bins
        for (i = 0; i < matrix->rows; i++) {
            for (j = 0; j < matrix->cols; j++) {
                int bin = (int)((matp(matrix,i,j) - min_val) * (WIDTH-1) / (max_val - min_val));
                if (bin >= 0 && bin < WIDTH) {
                    bins[bin]++;
                    if (bins[bin] > max_count) max_count = bins[bin];
                }
            }
        }

        // Print histogram
        printf("\nHistogram (%g to %g):\n", min_val, max_val);
        for (i = HEIGHT-1; i >= 0; i--) {
            for (j = 0; j < WIDTH; j++) {
                printf("%c", bins[j] * HEIGHT / max_count > i ? '*' : ' ');
            }
            printf("\n");
        }
        for (i = 0; i < WIDTH; i++) printf("-");
        printf("\n");

    } else if (plot_type == ASCII_BAR) {
        // Bar chart (first row or column)
        int i,j;
        int values = matrix->cols > 1 ? matrix->cols : matrix->rows;

        printf("\nBar Chart:\n");
        for (i = 0; i < values; i++) {
            double val = matrix->cols > 1 ? matp(matrix,0,i) : matp(matrix,i,0);
            int bars = (int)((val - min_val) * WIDTH / (max_val - min_val));
            printf("%3d |", i+1);
            for (j = 0; j < bars; j++) printf("#");
            printf(" %g\n", val);
        }

    } else if (plot_type == ASCII_LINE) {
        // Line plot
        char plot[20][60];  // Use fixed sizes instead of HEIGHT/WIDTH
        int i, j;

        // Initialize plot area
        for (i = 0; i < HEIGHT; i++) {
            for (j = 0; j < WIDTH; j++) {
                plot[i][j] = ' ';
            }
        }

        // Plot points and lines
        int last_y = -1;
        for (j = 0; j < matrix->cols; j++) {
            double val = matp(matrix,0,j);
            int x = j * (WIDTH-1) / (matrix->cols-1);
            int y = (int)((val - min_val) * (HEIGHT-1) / (max_val - min_val));
            y = HEIGHT - 1 - y;  // Invert y for display

            if (y >= 0 && y < HEIGHT && x >= 0 && x < WIDTH) {
                plot[y][x] = '*';

                // Draw connecting lines
                if (last_y >= 0) {
                    int start_y = last_y < y ? last_y : y;
                    int end_y = last_y < y ? y : last_y;
                    for (i = start_y; i <= end_y; i++) {
                        plot[i][x-1] = '|';
                    }
                }
                last_y = y;
            }
        }

        // Print plot
        printf("\nLine Plot:\n");
        for (i = 0; i < HEIGHT; i++) {
            printf("%6.2f |", max_val - i * (max_val - min_val) / (HEIGHT-1));
            for (j = 0; j < WIDTH; j++) {
                printf("%c", plot[i][j]);
            }
            printf("\n");
        }
        for (i = 0; i < WIDTH+8; i++) printf("-");
        printf("\n");
    } else if (plot_type == ASCII_HEAT) {
        // Heat map using ASCII characters
        printf("\nHeat Map:\n");
        const char* density = " .:+*#";  // Intensity characters
        int levels = strlen(density);
        int i,j;
        for (i = 0; i < matrix->rows; i++) {
            printf("%2d |", i);
            for (j = 0; j < matrix->cols; j++) {
                double val = (matp(matrix,i,j) - min_val) / (max_val - min_val);
                int level = (int)(val * (levels - 1));
                printf("%c", density[level]);
            }
            printf("\n");
        }

        // Print column numbers
        printf("    ");
        for (j = 0; j < matrix->cols; j++) {
            printf("%d", j % 10);
        }
        printf("\n");

    } else if (plot_type == ASCII_SCATTER) {
        // Scatter plot (first two columns)
        if (matrix->cols < 2) {
            printf("Error: Need at least 2 columns for scatter plot\n");
            RETURNINT(-1);
        }

        char plot[20][60] = {{' '}};  // Initialize with spaces

        // Find ranges for both columns
        double min_x = matrix->rows ? matp(matrix,0,0) : 0;
        double max_x = min_x;
        double min_y = matrix->rows ? matp(matrix,0,1) : 0;
        double max_y = min_y;
        int i,j;
        for (i = 0; i < matrix->rows; i++) {
            double x = matp(matrix,i,0);
            double y = matp(matrix,i,1);
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (y < min_y) min_y = y;
            if (y > max_y) max_y = y;
        }
        // Plot points
        for (i = 0; i < matrix->rows; i++) {
            int x = (int)((matp(matrix,i,0) - min_x) * 58 / (max_x - min_x));
            int y = (int)((matp(matrix,i,1) - min_y) * 18 / (max_y - min_y));
            y = 19 - y;  // Invert y for display
            if (x >= 0 && x < 60 && y >= 0 && y < 20) {
                plot[y][x] = '*';
            }
        }

        // Print plot
        printf("\nScatter Plot:\n");
        for (i = 0; i < 20; i++) {
            printf("%6.2f |", max_y - i * (max_y - min_y) / 19);
            for (j = 0; j < 60; j++) {
                printf("%c", plot[i][j]);
            }
            printf("\n");
        }
        printf("       ");
        for (i = 0; i < 60; i++) printf("-");
        printf("\n       %6.2f", min_x);
        printf("%*s%6.2f\n", 48, "", max_x);

    } else if (plot_type == ASCII_BOX) {
        // Box plot
        double* data = (double*)malloc(matrix->rows * matrix->cols * sizeof(double));
        if (!data) {
            printf("Error: Memory allocation failed\n");
            RETURNINT(-1);
        }

        // Copy data to linear array
        int i,j,n = 0;
        for (i = 0; i < matrix->rows; i++) {
            for (j = 0; j < matrix->cols; j++) {
                data[n++] = matp(matrix,i,j);
            }
        }

        double min, q1, median, q3, max;
        calculate_box_stats(data, n, &min, &q1, &median, &q3, &max);
        free(data);

        // Scale to plot width
        int p_min = 0;
        int p_q1 = (int)((q1 - min_val) * 58 / (max_val - min_val));
        int p_med = (int)((median - min_val) * 58 / (max_val - min_val));
        int p_q3 = (int)((q3 - min_val) * 58 / (max_val - min_val));
        int p_max = 58;

        // Print box plot
        printf("\nBox Plot:\n");
        printf("      |");
        for (i = 0; i < 60; i++) {
            if (i == p_min || i == p_max) printf("|");
            else if (i == p_med) printf("|");
            else if (i >= p_q1 && i <= p_q3) printf("#");
            else if (i > p_min && i < p_max) printf("-");
            else printf(" ");
        }
        printf("\n");

        // Print values
        printf("      %6.2f", min_val);
        printf("%*s%6.2f\n", 48, "", max_val);

    }

    RETURNINT(0);
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
    ADDPROC(mmultiply, "matrix.mmult",     "b",  ".int", "m0=.int, m1=.int,mid=.string");
    ADDPROC(mprod,     "matrix.mprod",     "b",  ".int", "m0=.int, prod=.float,mid=.string");
    ADDPROC(minvert,   "matrix.minvert",   "b",  ".int", "m0=.int, mid=.string");
    ADDPROC(mtranspose,"matrix.mtranspose","b",  ".int", "m0=.int, mid=.string");
    ADDPROC(mstandard, "matrix.mstandard", "b",  ".int", "m0=.int, mid=.string");
    ADDPROC(mcreate,   "matrix.mcreate",   "b",  ".int", "rows=.int,cols=.int,id=.string");
    ADDPROC(mset,      "matrix.mset",      "b",  ".int", "m0=.int, row=.int, col=.int,value=.float");
    ADDPROC(mget,      "matrix.mget",      "b",  ".float","m0=.int, row=.int, col=.int");
    ADDPROC(mexpand,   "matrix.mexpand",   "b",  ".int", "m0=.int, newrows=.int, init=.float");
    ADDPROC(mprint,    "matrix.mprint",    "b",  ".int", "m0=.int");
    ADDPROC(mfree,     "matrix.mfree",     "b",  ".int", "m0=.int");
    ADDPROC(mdet,      "matrix.mdet",      "b",  ".int", "m0=.int");
    ADDPROC(mlu,       "matrix.mlu",       "b",  ".int", "m0=.int, L=.string, U=.string");
    ADDPROC(mrank,     "matrix.mrank",     "b",  ".int", "m0=.int");
    ADDPROC(mcov,      "matrix.mcov",      "b",  ".int", "m0=.int, mid=.string");
    ADDPROC(mcorr,     "matrix.mcorr",     "b",  ".int", "m0=.int, mid=.string");
    ADDPROC(mmean,     "matrix.mmean",     "b",  ".float","m0=.int, axis=.int");
    ADDPROC(mstdev,    "matrix.mstdev",    "b",  ".float","m0=.int, axis=.int");
    ADDPROC(mfactor,   "matrix.mfactor",   "b",  ".int",  "m0=.int, factors=.int, mid=.string, parms=.string");
    ADDPROC(mcolstats, "matrix.mcolstats", "b",  ".int",  "m0=.int, mid=.string");
    ADDPROC(stats,     "matrix.stats",     "b",  ".int",  "m0=.int, mode=.string");
    ADDPROC(mplot,     "matrix.mplot",     "b",  ".int",  "m0=.int, plot_type=.string");
    ADDPROC(mfaplot,   "matrix.mfaplot",   "b",  ".int",  "m0=.int, plot_type=.string");
    ADDPROC(masciiplot,"matrix.masciiplot", "b",  ".int", "m0=.int, plot_type=.string");
ENDLOADFUNCS

