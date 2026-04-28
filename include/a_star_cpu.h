#include "grid.h"
#include "a_star_output.h"

#ifndef A_STAR_CPU_H
#define A_STAR_CPU_H

void RunAStar(Grid_2D_Device* grid, int startIndex_x, int startIndex_y, int goalIndex_x, int goalIndex_y, AStar_Output* output);

#endif