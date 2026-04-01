#ifndef GRID_H
#define GRID_H

// Structs
typedef struct Grid_Node {
    float data;
    int parent_index;
} Grid_Node;

typedef struct Grid_2D {
    Grid_Node** grid_ptr;
    int size_x;
    int size_y;
} Grid_2D;

typedef struct Grid_3D {
    Grid_Node** grid_ptr;
    int size_x; // width
    int size_y; // height
    int size_z; // length
} Grid_3D;

// Functions
Grid_2D* CreateGrid(int size_x, int size_y, float* default_val);
void UpdateGridByIndex(Grid_2D* grid, int row, int col, float* val);
void PrintGridFloat(Grid_2D* grid);
void DestroyGrid(Grid_2D*);

#endif