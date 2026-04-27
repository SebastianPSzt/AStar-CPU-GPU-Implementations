#include "grid.h"

#ifndef A_STAR_HELPERS_H
#define A_STAR_HELPERS_H

int GetPathFromStartToGoal(Grid_2D_Device* grid, int* gScore, int startIndex, int goalIndex, int* path);

#endif