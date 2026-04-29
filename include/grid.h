#ifndef GRID_H
#define GRID_H

// Structs
typedef struct Grid_2D_Device {
    int* data;
    int* parent;
    int size_x;
    int size_y;
} Grid_2D_Device;

// Uniform grid
typedef struct Grid_ND {
    int* data;
    int* parent;
    int* sizes;

    int totalSize;
    int dimension;
} Grid_ND;

// Functions
// 2D
Grid_2D_Device* CreateGrid(int size_x, int size_y, int default_val);
Grid_2D_Device* GatherGrid(const char *path);
void UpdateGridByIndex(Grid_2D_Device* grid, int row, int col, int val);
void PrintGrid(Grid_2D_Device* grid);
void DestroyGrid(Grid_2D_Device* grid);
int ReadGridByIndex(Grid_2D_Device* grid, int row, int col);

// ND
Grid_ND* CreateNDGrid(int* sizes, int dimension, int default_val);
void UpdateNDGridByIndex(Grid_ND* grid, int* indices, int val);
void DestroyNDGrid(Grid_ND* grid);

#endif