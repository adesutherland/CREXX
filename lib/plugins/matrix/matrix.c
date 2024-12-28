//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // For POSIX systems (Linux/macOS)
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file
#include <math.h>
#include <windows.h>

struct Matrix {
    double* CBselfref;     // CB self reference
    char id[32];
    int rows;
    int cols;
    double * vector;  // Pointer to the matrix CBselfref (flattened array), what an idiocy
};
#define mat(mptr,row,col)  mptr.vector[row * mptr.cols + col]

HANDLE hHeap = NULL;

int matrixmax=100;
void * allVectors[100];

uintptr_t getmain(int bytes) {
    // Allocate memory from the heap
   if (hHeap == NULL) {
       hHeap = GetProcessHeap();
    }
    LPVOID pMemory = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, bytes);
    if (pMemory == NULL) {
        printf("Heap allocation failed\n");
        return -1;
    }
    // Use the allocated memory
    //   printf("Memory allocated at address: %p\n", pMemory);
    //  HeapFree(hHeap, 0, pMemory);
    return (uintptr_t) pMemory;
}

void printMatrix(struct Matrix* matrix) {
    int i,j;
    printf("Matrix %s,dimension: %dx%d\n",matrix->id,matrix->cols,matrix->rows);
    for (i = 0; i < matrix->rows; ++i) {
        for (j = 0; j < matrix->cols; ++j) {
            printf("%10.6f ", matrix->vector[i * matrix->cols + j]);
        }
        printf("\n");
    }
}

// Function to calculate the mean of the vector
double calculate_mean(int matname,int col) {
    int i;
    double sum = 0.0;
    struct Matrix matrix= *(struct Matrix *) allVectors[matname];
    for (i = 0; i < matrix.rows; i++) {
        sum += mat(matrix,i,col);
    }
    return sum / matrix.rows;
}

// Function to calculate the standard deviation of the vector
double calculate_stdev(int matname,int col, double mean) {
    int i;
    struct Matrix matrix= *(struct Matrix *) allVectors[matname];
    double sum_squared_diff = 0.0;

    for (i = 0; i < matrix.rows; i++) {
        sum_squared_diff += pow(mat(matrix,i,col) - mean, 2);
    }
    return sqrt(sum_squared_diff / (matrix.rows-1));
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
    int matrixname;
    double * matCB;
    double * matVECTOR;
 // find empty matrix slot
    for (matrixname = 0; matrixname <= matrixmax; ++matrixname) {
        if (allVectors[matrixname] == 0) break;
    }
    if (matrixname > matrixmax) return -12;
   matCB = (double *) getmain(sizeof(struct Matrix));
   if (matCB < 0 ) return -16;

   struct Matrix * MATRIX = (struct Matrix *) matCB;
   MATRIX->CBselfref = matCB;     // save self referring CB address
   MATRIX->rows = rows;   // rows of matrix
   MATRIX->cols = cols;   // cols of matrix

 // allocate vector containing the real matrix
    matVECTOR=(double *) getmain(MATRIX->rows * MATRIX->cols * sizeof(double *));
    if (matVECTOR < 0 ) return -20;
    MATRIX->vector = matVECTOR;
    allVectors[matrixname]=matCB;
    strncpy(MATRIX->id, matid,sizeof(MATRIX->id));

   return matrixname;
}
PROCEDURE(mcreate) {
    int rows, cols,matnum;
    char * id= GETSTRING(ARG2);
    // -16 (100, "unable to allocate heap memory")
    //-20 (100, "unable to allocate heap memory")
    rows = GETINT(ARG0);   // rows of matrix
    cols = GETINT(ARG1);   // cols of matrix
    matnum=matcreate(rows,cols,id);
    RETURNINT(matnum);
}

PROCEDURE(mset){
    struct Matrix cmatrix = *(struct Matrix *) allVectors[GETINT(ARG0)];
    // Access and print matrices through pointers
    int row=GETINT(ARG1)-1;
    int col=GETINT(ARG2)-1;
    mat(cmatrix,row,col) = GETFLOAT((ARG3)); // Set a value
ENDPROC
}
PROCEDURE(mprint) {
    struct Matrix cmatrix = *(struct Matrix *) allVectors[GETINT(ARG0)];
    printMatrix((struct Matrix *) cmatrix.CBselfref);
    ENDPROC
}

PROCEDURE(mmultiply){
    int i,j,k, matnum;
    struct Matrix mptr1= *(struct Matrix *) allVectors[GETINT(ARG0)];
    struct Matrix mptr2= *(struct Matrix *) allVectors[GETINT(ARG1)];
    matnum=matcreate(mptr1.rows,mptr2.cols, GETSTRING(ARG2));
    struct Matrix mptr3=*(struct Matrix *) allVectors[matnum];
    // Matrix multiplication
    for (i = 0; i < mptr1.rows; i++) {
        for (j = 0; j < mptr2.cols; j++) {
            for (k = 0; k < mptr1.cols; k++) {
                mat(mptr3,i,j) = mat(mptr3,i,j) + mat(mptr1,i,k) * mat(mptr2,k,j);
            }
        }
    }
    RETURNINTX(matnum);
ENDPROC
 }

// Function to invert a matrix using Gaussian elimination
// int invertMatrix(int n, double matrix[n][n], double inverse[n][n])
PROCEDURE(minvert) {
    int i,j,k,n,matnum;
    struct Matrix matrix= *(struct Matrix *) allVectors[GETINT(ARG0)];
    matnum=matcreate(matrix.rows,matrix.cols, GETSTRING(ARG1));
    struct Matrix minv = *(struct Matrix *) allVectors[matnum];
    n=matrix.cols;

    double augmented[n][2 * n];   // Create an augmented matrix [matrix | I]
    // Initialize augmented matrix
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            augmented[i][j] = mat(matrix,i,j);
        }
        for (j = n; j < 2 * n; j++) {
            augmented[i][j] = (i == j - n) ? 1.0 : 0.0; // Identity matrix
        }
    }
    // Perform Gaussian elimination
    for (i = 0; i < n; i++) {
        // Make the diagonal element 1 (pivoting if necessary)
        if (fabs(augmented[i][i]) < 1e-9) {
            // Search for a row to swap
            int swapRow = -1;
            for (k = i + 1; k < n; k++) {
                if (fabs(augmented[k][i]) > 1e-9) {
                    swapRow = k;
                    break;
                }
            }
            if (swapRow == -1) {
                printf("Matrix is singular and cannot be inverted.\n");
                RETURNINTX(-1) // Not invertible
            }
            // Swap rows
            for (j = 0; j < 2 * n; j++) {
                double temp = augmented[i][j];
                augmented[i][j] = augmented[swapRow][j];
                augmented[swapRow][j] = temp;
            }
        }
        // Normalize the pivot row
        double pivot = augmented[i][i];
        for (j = 0; j < 2 * n; j++) {
            augmented[i][j] /= pivot;
        }
        // Eliminate other rows
        for (k = 0; k < n; k++) {
            if (k != i) {
                double factor = augmented[k][i];
                for (j = 0; j < 2 * n; j++) {
                    augmented[k][j] -= factor * augmented[i][j];
                }
            }
        }
    }

    // Extract the inverse matrix from the augmented matrix
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            mat(minv,i,j) = augmented[i][j + n];
        }
    }

    RETURNINTX(matnum); // Success
    ENDPROC
}
PROCEDURE(mtranspose) {
    int i,j,rows, cols, matnum;
    struct Matrix matrix= *(struct Matrix *)  allVectors[GETINT(ARG0)];
    matnum=matcreate(matrix.cols,matrix.rows, GETSTRING(ARG1));
    struct Matrix mtrans = *(struct Matrix *) allVectors[matnum];

    rows=matrix.rows;
    cols=matrix.cols;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            mat(mtrans,j,i) = mat(matrix,i,j); // Transpose logic
        }
    }
    RETURNINT(matnum);
}
PROCEDURE(mstandard) {
    int i,j,rows, cols, matnew;
    struct Matrix matrix= *(struct Matrix *)  allVectors[GETINT(ARG0)];
    matnew=matcreate(matrix.rows,matrix.cols, GETSTRING(ARG1));

    for (j = 0; j < matrix.cols; j++) {
        standardize_vector(GETINT(ARG0), matnew,j);
    }
    RETURNINT(matnew);
}
PROCEDURE(mprod) {
    int i,j,matprod,rows, cols,indx,m1,m2;
    struct Matrix matrix= *(struct Matrix *)  allVectors[GETINT(ARG0)];
    matprod=matcreate(matrix.cols,matrix.rows, GETSTRING(ARG2));
    struct Matrix matnew = *(struct Matrix *) allVectors[matprod];

    rows=matrix.rows;
    cols=matrix.cols;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            mat(matnew,i,j) = mat(matrix,i,j) * GETFLOAT(ARG1);
        }
    }
    RETURNINT(matprod);
}
int freeMatrix(int matnum) {
    int rc;
    if (allVectors[matnum]==NULL) return -4;
    struct Matrix matrix= *(struct Matrix *)  allVectors[matnum];
    rc=HeapFree(hHeap, 0, matrix.vector);
    if(rc==0) return -8;
    rc=HeapFree(hHeap, 0, matrix.CBselfref);
    if(rc==0) return -12;
    allVectors[matnum]=0;
    return 0;
}
PROCEDURE(mfree) {
    int i,rc;
    if(GETINT(ARG0)>=0) RETURNINT(freeMatrix(GETINT(ARG0)));
    else {
        for (i = 0; i < matrixmax; i++) {
            if (freeMatrix(i)==0) printf("Matrix freed %d\n",i);
        }
    }
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Functions to be provided to rexx
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