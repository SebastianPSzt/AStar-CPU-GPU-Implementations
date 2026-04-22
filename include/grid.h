#ifndef GRID_H
#define GRID_H

// Structs
typedef struct Grid_2D_Device {
    float* data;
    int* parent;
    int size_x;
    int size_y;
} Grid_2D_Device;

// Uniform grid
typedef struct Grid_ND {
    float* data;
    int* parent;
    int* sizes;

    int totalSize;
    int dimension;
} Grid_ND;

// Functions
// 2D
Grid_2D_Device* CreateGrid(int size_x, int size_y, float default_val);
void UpdateGridByIndex(Grid_2D_Device* grid, int row, int col, float val);
void PrintGridFloat(Grid_2D_Device* grid);
void DestroyGrid(Grid_2D_Device* grid);

// ND
Grid_ND* CreateNDGrid(int* sizes, int dimension, float default_val);
void UpdateNDGridByIndex(Grid_ND* grid, int* indices, float val);
void DestroyNDGrid(Grid_ND* grid);

#endif