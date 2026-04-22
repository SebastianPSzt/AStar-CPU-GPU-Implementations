#include<stdio.h>
#include<stdlib.h>

#include "../include/grid.h"

Grid_2D_Device* CreateGrid(int size_x, int size_y, float default_val) {
    size_t totalSize = size_x * size_y;

    Grid_2D_Device* grid = (Grid_2D_Device*) malloc(sizeof(Grid_2D_Device));

    grid->size_x = size_x;
    grid->size_y = size_y;

    grid->data = (float*)malloc(sizeof(float) * totalSize);
    grid->parent = (int*)malloc(sizeof(int)* totalSize);

    for (int i = 0; i < totalSize; i++)
    {
        grid->data[i] = default_val;
        grid->parent[i] = -1;
    }

    return grid;
}

void PrintGridFloat(Grid_2D_Device* grid) {
    for (int row = 0; row < grid->size_y; row++) 
    {
        for (int col = 0; col < grid->size_x; col++) 
        {
            size_t index = row * grid->size_x + col;
            printf("%f ", grid->data[index]);
        }
        printf("\n");
    }
}

void UpdateGridByIndex(Grid_2D_Device* grid, int row, int col, float val) {
    grid->data[row * grid->size_x + col] = val;
}

void DestroyGrid(Grid_2D_Device* grid) {
    free(grid->data);
    free(grid->parent);
    free(grid);
}

Grid_ND* CreateNDGrid(int* sizes, int dimension, float default_val) {
    // Initialize grid
    Grid_ND* grid = (Grid_ND*)malloc(sizeof(Grid_ND));

    grid->sizes = (int*)malloc(sizeof(int) * dimension);
    grid->dimension = dimension;
    grid->totalSize = 1;

    for (int i = 0; i < dimension; i++)
    {
        int size_i = sizes[i];
        grid->sizes[i] = size_i;
        grid->totalSize *= size_i;
    }

    grid->data = (float*)malloc(sizeof(float) * grid->totalSize);
    grid->parent = (int*)malloc(sizeof(int) * grid->totalSize);    

    int totalSize = grid->totalSize;
    for (int i = 0; i < totalSize; i++)
    {
        grid->data[i] = default_val;
        grid->parent[i] = -1;
    }

    return grid;
}

void UpdateNDGridByIndex(Grid_ND* grid, int* indices, float val) {
    // y * length_x + x
    // z * length_y * length_x + y * length_x + x
    // For 1..D, nth dimension is scaled by length_(N+1..D)

    int dim = grid->dimension;
    int index = 0;

    for (int i = 0; i < dim; i++)
    {
        int index_i = indices[i];
        for (int j = i+1; j < dim; j++)
        {
            index_i *= grid->sizes[j];
        }
        index += index_i;
    }

    grid->data[index] = val;
}

void DestroyNDGrid(Grid_ND* grid) {
    free(grid->data);
    free(grid->parent);
    free(grid->sizes);
    free(grid);
}