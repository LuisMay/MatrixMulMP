#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

FILE *pFile;		//File handler for matrix files

float* matrixA = NULL;	//Pointer to matrix A
float* matrixB = NULL;	//Pointer to matrix B
float* matrixC = NULL;	//Pointer to matrix C

//Variables to save exec time
time_t start, end;

errno_t err;		//File error catcher

char fName[100];	//Name of the file
int rA, cA, rB, cB, rC, cC = 0;

//Print matrix
void printMatrix(float* matrix, int nrow, int ncol) {
	for (int i = 0; i < nrow; i++) {
		for (int j = 0; j < ncol; j++)
			printf("% 4.2f ", *(matrix + i * ncol + j));
		printf("\n");
	}
}

// Fill matrix from console
void fillMatrixUI(char* fName, float* matrix, int rows, int cols){
	err = fopen_s(&pFile, fName, "r");
	if (err == 0){
		for (int i = 0; i < rows; i++)
			for (int j = 0; j < cols; j++)
				if (!feof(pFile)) {
					fscanf_s(pFile, "%f", (matrix + i * cols + j));
				}
				else {
					printf("Elementos insuficientes en el archivo, saliendo...\n");
					return 2;
				}
		// Impresion de la matriz
		printMatrix(matrix, rows, cols);
	}	
	else {
		printf("No se pudo abrir el archivo %s, saliendo...\n", fName);
		return 1;
	}
	fclose(pFile);
}
 
/********************/
/*** Main program ***/
/********************/
int main(void){
	int validMatrix = 0;
	do
	{
		/*-----------------------------Llenado de matriz A-----------------------------*/
		printf("Introduzca las dimensiones de la matriz A (filas columnas): ");
		scanf_s("%d %d", &rA, &cA);
		strcpy_s(fName, 100, "D:\\Users\\Alessandro Balcazar\\Documents\\Proyecto_Multi\\MatrixMulMP\\Debug\\matrixA2500.txt");
		matrixA = (float*)malloc(sizeof(float) * rA * cA);
		fillMatrixUI(fName, matrixA, rA, cA);

		/*-----------------------------Llenado de matriz B-----------------------------*/
		printf("Introduzca las dimensiones de la matriz B (filas columnas): ");
		scanf_s("%d %d", &rB, &cB);
		strcpy_s(fName, 100, "D:\\Users\\Alessandro Balcazar\\Documents\\Proyecto_Multi\\MatrixMulMP\\Debug\\matrixB2500.txt");
		err = fopen_s(&pFile, fName, "r");
		matrixB = (float*)malloc(sizeof(float) * rB * cB);
		fillMatrixUI(fName, matrixB, rB, cB);

		/*-----------Validacion para poder multuplicar las matrices-----------*/
		if (cA != rB) {
			printf("\nLas matrices no se pueden multiplicar. Verifique que las filas y columnas.\n");
		}
		else {
			validMatrix = 1;
		}
	} while (validMatrix == 0);

	return 0; 
}