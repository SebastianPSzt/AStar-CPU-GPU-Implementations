#include "grid.h"
#include "search_log_cpu.h"

#ifndef A_STAR_CPU_H
#define A_STAR_CPU_H

SearchLogNode *RunAStar(Grid_2D *grid, int startIndex_x, int startIndex_y, int goalIndex_x, int goalIndex_y);

#endif