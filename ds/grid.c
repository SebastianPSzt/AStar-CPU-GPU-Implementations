#include<stdio.h>
#include<stdlib.h>
#include "../include/grid.h"

Grid_2D* CreateGrid(int size_x, int size_y, float *default_val) {
    Grid_2D *grid = (Grid_2D*) malloc(sizeof(Grid_2D));

    grid->size_x = size_x;
    grid->size_y = size_y;

    grid->grid_ptr = (Grid_Node**) malloc(sizeof(Grid_Node*) * size_x * size_y);

    for (int row = 0; row < size_y; row++) {
        for (int col = 0; col < size_x; col++) {
            size_t index = row * size_x + col;
            
            Grid_Node* node = (Grid_Node*) malloc(sizeof(Grid_Node));
            node->data = *default_val;
            node->parent_index = -1;
            
            grid->grid_ptr[index] = node;
        }
    }

    return grid;
}

void PrintGridFloat(Grid_2D* grid) {
    for (int row = 0; row < grid->size_y; row++) 
    {
        for (int col = 0; col < grid->size_x; col++) 
        {
            size_t index = row * grid->size_x + col;
            printf("%f ", grid->grid_ptr[index]->data);
        }
        printf("\n");
    }
}

void UpdateGridByIndex(Grid_2D* grid, int row, int col, float* val) {
    grid->grid_ptr[row * grid->size_x + col]->data = *val;
}

void DestroyGrid(Grid_2D* grid) {

    for (int row = 0; row < grid->size_y; row++) 
    {
        for (int col = 0; col < grid->size_x; col++) 
        {
            size_t index = row * grid->size_x + col;
            free(grid->grid_ptr[index]);
        }
    }

    free(grid->grid_ptr);
    free(grid);
}