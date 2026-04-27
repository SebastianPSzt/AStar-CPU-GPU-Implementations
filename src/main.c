#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/grid.h"
#include "../include/pq.h"
#include "../include/bmh.h"
#include "../include/a_star_cpu.h"
#include "../include/search_log_cpu.h"

int main(int argc, char *argv[])
{
    printf("---------------A* Testing---------------\n");

    Grid_2D *grid = GatherGrid("./godot/grids/region.json");
    if (!grid)
    {
        printf("Failed to load grid from JSON\n");
        return 1;
    }
    PrintGridFloat(grid);

    int start_x = 0;
    int start_y = 0;
    int goal_x = grid->size_x - 1;
    int goal_y = grid->size_y - 1;

    SearchLogNode *log = RunAStar(grid, start_x, start_y, goal_x, goal_y);

    log = ReverseLog(log);
    PrintSearchLog(log);

    FreeSearchLog(log);

    DestroyGrid(grid);

    return 0;
}