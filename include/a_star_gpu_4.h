#include "grid.h"
#include "a_star_output.h"

#ifndef A_STAR_GPU_4
#define A_STAR_GPU_4

void Run_AStar(Grid_2D_Device* grid, int startIndex_x, int startIndex_y, int goalIndex_x, int goalIndex_y, int* forward_gScore_d, int* backward_gScore_d, int* gridData_d, int* bucket_sizes_d, int* bucket_nodes_d, int* next_bucket_d, int* bestMeetCost_d, int* bestMeetIndex_d, int startBucket, AStar_Output* output);
void Init_AStar(Grid_2D_Device* grid, int startIndex_x, int startIndex_y, int goalIndex_x, int goalIndex_y, int print, AStar_Output* output);
int GetPathFromStartToGoal(Grid_2D_Device* grid, int* gScore, int startIndex, int goalIndex, int* path);
void Print_AStar(Grid_2D_Device* grid, int reachedPath, int bestCost, int* forward_gScore_h, int* backward_gScore_h, int startIndex, int goalIndex, int bestMeetIndex, AStar_Output* output);
void Clean_AStar(int* gScore, int* forward_gScore_d, int* backward_gScore_d, int* gridData_d, int* bucket_sizes_d, int* bucket_nodes_d, int* next_bucket_d, int* bestMeetCost_d, int* bestMeetIndex_d);

#endif