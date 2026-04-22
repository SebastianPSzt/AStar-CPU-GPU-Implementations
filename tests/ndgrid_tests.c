#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "../include/grid.h"
#include "../include/ndgrid_tests.h"

/**
 * @param dimension - total dimensions
 * @param size - uniform size along single dimension
 */
void EmptyGrid_Test(int dimension, int size) {
    int* sizes = (int*)malloc(sizeof(int) * dimension);
    for (int i = 0; i < dimension; i++) {
        sizes[i] = size;
    }

    Grid_ND* grid = CreateNDGrid(sizes, dimension, 1.0f);

    float sum = 0;
    for (int i = 0; i < grid->totalSize; i++){
        printf("%f\n", grid->data[i]);
        sum += grid->data[i];
    }

    printf("Dim: %d, Size: %d, Sum: %f, equal: %d\n", dimension, size, sum, sum == pow(size, dimension));

    DestroyNDGrid(grid);
    free(sizes);
}