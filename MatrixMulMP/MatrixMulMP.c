#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include <windows.h>
#include <intrin.h>

#define FILE_A	"\\matrixA1048576.txt"
#define FILE_B	"\\matrixB1048576.txt"
#define FILE_C	"\\matrixC.txt"

//#define DEBUG

#define THREADS	16

char cwd[100];			// Exec path

FILE *pFile;			// File handler for matrix files

__declspec(align(64))double* matrixA = NULL;	//Pointer to matrix A
__declspec(align(64))double* matrixB = NULL;	//Pointer to matrix B
__declspec(align(64))double* matrixC = NULL;	//Pointer to matrix C

// Variables to save exec time
time_t start, end, seqTimeArr[5], exTime2[5], exTime3[5];

errno_t err;			//File error catcher

// Intrinsics registers using AVX2
__m256 axi, bxi, sumxi;

char fName[100];		//Name of the file
int rA, cA, rB, cB, rC, cC = 0;

// Print matrix
void printMatrix(double* matrix, int nrow, int ncol) {
#ifdef DEBUG 
	for (int i = 0; i < nrow; i++) {
		for (int j = 0; j < ncol; j++)
			printf("% 8.2f ", *(matrix + i * ncol + j));
		printf("\n");
	}
#endif
}

// Save matrix to file
int saveMatrix(double* matrix, int rows, int cols) {
	err = fopen_s(&pFile, fName, "w");
	// Si se pudo abrir el archivo
	if (err == 0) {
		for (int i = 0; i < rows; i++)
			for (int j = 0; j < cols; j++)
				// Escribe los datos de la matriz en el archivo en el mismo 
				// formato de los archivos de entrada 
				fprintf_s(pFile, "% 20.15lf\n", *(matrix + i * cols + j));
	}
	else {
		printf("No se pudo abrir el archivo %s, saliendo...\n", fName);
		return 2;
	}
	fclose(pFile);
	return 0;
}

int checkResults(double* matrix, int rows, int cols) {
	err = fopen_s(&pFile, fName, "r");
	double row = 0.0;
	// Si no hay error abriendo el archivo
	if (err == 0){
		for (int i = 0; i < rows; i++)
			for (int j = 0; j < cols; j++)
				// Si no se ha llegado al fina del archivo
				if (!feof(pFile)) {
					fscanf_s(pFile, "%lf ", &row);
					// Compara el contenido de la linea del archivo con la celda de la matriz 
					if (row - *(matrix + i * cols + j) < -0.000001 || row - *(matrix + i * cols + j) > 0.000001)
						return 5;
				}
				else {
					printf("Elementos insuficientes en el archivo, saliendo...\n");
					return 3;
				}
	}	
	else {
		printf("No se pudo abrir el archivo %s, saliendo...\n", fName);
		return 2;
	}
	fclose(pFile);
	return 0;
}

// Fill matrix from file
int fillMatrixUI(char* fName, double* m, int rows, int cols){
	err = fopen_s(&pFile, fName, "r");
	// Si no hay error abriendo el archivo
	if (err == 0){
		for (int i = 0; i < rows; i++)
			for (int j = 0; j < cols; j++)
				// Si no se ha llegado al fina del archivo
				if (!feof(pFile)) {
					fscanf_s(pFile, "%lf ", (m + i * cols + j));
				}
				else {
					printf("Elementos insuficientes en el archivo, saliendo...\n");
					return 3;
				}
		// Impresion de la matriz
		printMatrix(m, rows, cols);
	}	
	else {
		printf("No se pudo abrir el archivo %s, saliendo...\n", fName);
		return 2;
	}
	fclose(pFile);
	return 0;
}

// Inicializa la matriz con 0
void zeroMatrix(double* m, int rows, int cols){
	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++)
			*(m + i * cols + j) = 0;
}

void trasposeMatrix(double* matrix, int rows, int cols) {
	__declspec(align(64))double* aux = (double *) malloc(sizeof(double) * rows * cols);
	// copy contents of matrix to auxiliar
	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < cols; ++j) 
			*(aux + i * cols + j) = *(matrix + i * cols + j);
	
	// computing the transpose
	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < cols; ++j) 
			*(matrix + j * cols + i) = *(aux + i * cols + j);
	free(aux);
}


// Multiplica la matriz A por la matriz B y guarda el resultado en la matriz C
void mulMatrix(double* mA, double* mB, double* mC, int rows, int cols, int el) {
	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++) {
			for (int k = 0; k < el; k++) {
				// Suma la multiplicacion de las filas de a por las columnas de b
				*(mC + i * cols + j) += *(mA + i * el + k) * *(mB + k * cols + j);
			}
		}
}

// Multiplica la matriz A por la matriz B y guarda el resultado en la matriz C utilizando OMP
void mulMatrixOMP(double* mA, double* mB, double* mC, int rows, int cols, int el) {
	#pragma omp parallel num_threads(THREADS) 
	{
		int i, j, k;
		#pragma omp for private(i,j,k)
		for (i = 0; i < rows; i++) {
			for (j = 0; j < cols; j++) {
				for (k = 0; k < el; k++) {
					// Suma la multiplicacion de las filas de a por las columnas de b
					*(mC + i * cols + j) += *(mA + i * el + k) * *(mB + k * cols + j);
				}
			}
		}
	}
}

// Multiplica la matriz A por la matriz B y guarda el resultado en la matriz C utilizando vectorizacion
void mulMatrixVec(double* mA, double* mB, double* mC, int rows, int cols, int el) {
	__m256d iA, iB, iC;
	double acum[4] = {0.0,0.0,0.0,0.0};

	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++) {
			for (int l = 0; l < 4; l++) {
				acum[l] = 0.0;
			}
			for (int k = 0; k < el / 4; k++) {
				iA = _mm256_load_pd(mA + i * el + k * 4);
				iB = _mm256_load_pd(mB + j * el + k * 4);
				iC = _mm256_mul_pd(iA, iB);
				_mm256_store_pd(acum, iC);
				for (int l = 0; l < 4; l++)
					*(mC + i * cols + j) += acum[l];
			}
		}
}
 
/********************/
/*** Main program ***/
/********************/
int main(void){
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		perror("getcwd() error");
		return 1;
	}

	int validMatrix = 0;
	do
	{
		/*-----------------------------Llenado de matriz A-----------------------------*/
		printf("Introduzca las dimensiones de la matriz A (filas columnas): ");
		scanf_s("%d %d", &rA, &cA);
		strcpy_s(fName, 100, cwd);
		strcat_s(fName, 100, FILE_A);
		matrixA = (double *) malloc(sizeof(double) * rA * cA);
		printf("Cargando matriz A ...\n");
		fillMatrixUI(fName, matrixA, rA, cA);

		/*-----------------------------Llenado de matriz B-----------------------------*/
		printf("Introduzca las dimensiones de la matriz B (filas columnas): ");
		scanf_s("%d %d", &rB, &cB);
		strcpy_s(fName, 100, cwd);
		strcat_s(fName, 100, FILE_B);
		matrixB = (double *) malloc(sizeof(double) * rB * cB);
		printf("Cargando matriz B ...\n");
		fillMatrixUI(fName, matrixB, rB, cB);

		/*---------------Validacion para poder multuplicar las matrices---------------*/
		if (cA != rB) {
			printf("\nLas matrices no se pueden multiplicar. Verifique que las filas y columnas.\n");
			return 4; 
		}
		else {
			validMatrix = 1;
		}
	} while (validMatrix == 0);

	// Calculo de la matriz C 
	matrixC = (double*)malloc(sizeof(double) * rA * cB);
	// Inicializar matriz c en 0
	zeroMatrix(matrixC, rA, cB);
	// Multiplicar matrices
	mulMatrix(matrixA, matrixB, matrixC, rA, cB, cA);
	
	printf("\nMatriz C:\n");
	// Muestra el resultado de las matrices
	printMatrix(matrixC, rA, cB);
	//Guarda el resultado en el archivo FILE_C
	printf("Guardando matrizC en archivo... \n");
	strcpy_s(fName, 100, cwd);
	strcat_s(fName, 100, FILE_C);
	saveMatrix(matrixC, rA, cB);

	// Calculo del tiempo en secuencial 
	printf("Tiempo de ejecucion en secuencial (5): \n");
	for (int i = 0; i < 5; i++) {
		// Reinicializar la matriz en 0
		zeroMatrix(matrixC, rA, cB);
		start = clock();
		mulMatrix(matrixA, matrixB, matrixC, rA, cB, cA);
		end = clock();
		seqTimeArr[i] = end - start; 
		printf("%I64d ",seqTimeArr[i]);
	}
	printf("\n");
	
	// Calculo de tiempo usando OMP
	printf("\nTiempo de ejecucion usando OMP (5): \n");
	for (int i = 0; i < 5; i++) {
		// Reinicializar la matriz en 0
		zeroMatrix(matrixC, rA, cB);
		start = clock();
		mulMatrixOMP(matrixA, matrixB, matrixC, rA, cB, cA);
		end = clock();
		if (i == 0)
			printMatrix(matrixC, rA, cB);
		if (checkResults(matrixC, rA, cB) != 0)
			printf("Error en calculo de matriz...\n");
		exTime2[i] = end - start; 
		printf("%I64d ",exTime2[i]);
	}
	printf("\n");

	// Trasponer la matriz para facilitar la vectorizacion
	trasposeMatrix(matrixB, rB, cB);

	// Calculo del tiempo usando vectorizacion
	printf("\nTiempo de ejecucion usando Vectorizacion (5): \n");
	for (int i = 0; i < 5; i++) {
		// Reinicializar la matriz en 0
		zeroMatrix(matrixC, rA, cB);
		start = clock();
		mulMatrixVec(matrixA, matrixB, matrixC, rA, cB, cA);
		end = clock();
		if (i == 0)
			printMatrix(matrixC, rA, cB);
		if (checkResults(matrixC, rA, cB) != 0)
			printf("Error en calculo de matriz...\n");
		exTime2[i] = end - start; 
		printf("%I64d ",exTime2[i]);
	}
	printf("\n");

	free(matrixA);
	free(matrixB);
	free(matrixC);

	return 0; 
}