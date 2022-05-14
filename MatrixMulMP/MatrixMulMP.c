#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

FILE *pFile;		//File handler for matrix files

float* A = NULL;	//Pointer to matrix A
float* B = NULL;	//Pointer to matrix B
float* C = NULL;	//Pointer to matrix C

//Variables to save exec time
time_t start, end;

errno_t err;		//File error catcher

char fName[50]="";	//Name of the file
int rA, cA, rB, cB, rC, cC = 0;

void printMatrix(float* matrix, int nrow, int ncol) {
	for (int i = 0; i < nrow; i++) {
		for (int j = 0; j < ncol; j++)
			printf("% 4.10f ", *(matrix + i * ncol + j));
		printf("\n");
	}
}

int main(void) {
	/*-----------------------------Llenado de matriz A-----------------------------*/
	printf("Introduzca las dimenciones de la matriz A (filas columnas): ");
	scanf_s("%d %d", &rA, &cA);
	err = fopen_s(&pFile, fName, "r");
	A = (float*)malloc(sizeof(float)*rA*cA);
	if (err == 0){
		for (int i = 0; i < rA; i++)
			for (int j = 0; j < cA; j++)
				if (!feof(pFile)) {
					fscanf_s(pFile, "%f", (A + i * cA + j));
				}
				else {
					printf("Elementos insuficientes en el archivo, saliendo...\n");
					return 2;
				}
		printMatrix(A, rA, cA);
	}	
	else {
		// Impresion de la matriz
		printf("No se pudo abrir el archivo %s, saliendo...\n", fName);
		return 1;
	}
	fclose(pFile);

	/*-----------------------------Llenado de matriz B-----------------------------*/
	printf("Introduzca relative path al archivo que contiene la matriz B: ");
	scanf_s("%s", &fName, 50);
	printf("Introduzca las dimenciones de la matriz B (filas columnas): ");
	scanf_s("%d %d", &rB, &cB);
	err = fopen_s(&pFile, fName, "r");
	B = (float*)malloc(sizeof(float)*rB*cB);
	if (err == 0) {
		for (int i = 0; i < rB; i++)
			for (int j = 0; j < cB; j++)
				if (!feof(pFile)) {
					fscanf_s(pFile, "%f", (B + i * cB + j));
				}
				else {
					printf("Elementos insuficientes en el archivo, saliendo...\n");
					return 2;
				}
		printMatrix(B, rB, cB);
	}
	else {
		printf("No se pudo abrir el archivo %s, saliendo...\n", fName);
		return 1;
	}
	fclose(pFile);

	return 0; 
}