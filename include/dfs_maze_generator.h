#include "grid.h"

#ifndef DFS_MAZE_GENERATOR_H
#define DFS_MAZE_GENERATOR_H

#define NEIGHBORS 4

Grid_2D_Device* GenerateMaze(int size_x, int size_y, int startIndex_x, int startIndex_y, int goalIndex_x, int goalIndex_y);

#endif