#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define FILE_A	"\\matrixA1048576.txt"
#define FILE_B	"\\matrixB1048576.txt"
#define FILE_C	"\\matrixC.txt"

char cwd[100];			// Exec path

FILE *pFile;			// File handler for matrix files

double* matrixA = NULL;	//Pointer to matrix A
double* matrixB = NULL;	//Pointer to matrix B
double* matrixC = NULL;	//Pointer to matrix C

// Variables to save exec time
time_t start, end, seqTimeArr[5], exTime2[5], exTime3[5];

errno_t err;			//File error catcher

char fName[100];		//Name of the file
int rA, cA, rB, cB, rC, cC = 0;

// Print matrix
void printMatrix(double* matrix, int nrow, int ncol) {
	for (int i = 0; i < nrow; i++) {
		for (int j = 0; j < ncol; j++)
			printf("% 8.2f ", *(matrix + i * ncol + j));
		printf("\n");
	}
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

// Multiplica la matriz A por la matriz B y guarda el resultado en la matriz C
void mulMatrix(double* mA, double* mB, double* mC, int rows, int cols, int el) {
	double acum; 
	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++) {
			acum = 0;
			for (int k = 0; k < el; k++) {
				// Suma la multiplicacion de las filas de a por las columnas de b
				acum += *(mA + i * el + k) * *(mB + k * cols + j);
			}
			*(mC + i * cols + j) = acum;
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
		fillMatrixUI(fName, matrixA, rA, cA);

		/*-----------------------------Llenado de matriz B-----------------------------*/
		printf("Introduzca las dimensiones de la matriz B (filas columnas): ");
		scanf_s("%d %d", &rB, &cB);
		strcpy_s(fName, 100, cwd);
		strcat_s(fName, 100, FILE_B);
		matrixB = (double *) malloc(sizeof(double) * rB * cB);
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
	mulMatrix(matrixA, matrixB, matrixC, rA, cB, cA);
	printf("\nMatriz C:\n");
	printMatrix(matrixC, rA, cB);
	printf("Guardando matrizC en archivo... \n");
	strcpy_s(fName, 100, cwd);
	strcat_s(fName, 100, FILE_C);
	saveMatrix(matrixC, rA, cB);

	// Calculo del tiempo en secuencial 
	printf("Tiempo de ejecucion en secuencial (5): ");
	for (int i = 0; i < 5; i++) {
		start = clock();
		mulMatrix(matrixA, matrixB, matrixC, rA, cB, cA);
		end = clock();
		seqTimeArr[i] = end - start; 
		printf("%I64d ",seqTimeArr[i]);
	}


	return 0; 
}