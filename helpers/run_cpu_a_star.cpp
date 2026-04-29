#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/grid.h"
#include "../include/pq.h"
#include "../include/bmh.h"
#include "../include/a_star_cpu.h"
#include "../include/run_cpu_a_star.h"

void run_cpu(const char *path_in, const char *path_out)
{
    Grid_2D_Device *grid = GatherGrid(path_in);

    if (!grid)
        return;

    int start_x = 0;
    int start_y = 0;
    int goal_x = grid->size_x - 1;
    int goal_y = grid->size_y - 1;

    if (grid->size_x <= 0 || grid->size_y <= 0)
    {
        DestroyGrid(grid);
        return;
    }



    if (start_x < 0 || start_x >= grid->size_x ||
        start_y < 0 || start_y >= grid->size_y ||
        goal_x < 0 || goal_x >= grid->size_x ||
        goal_y < 0 || goal_y >= grid->size_y)
    {
        DestroyGrid(grid);
        return;
    }

    AStar_Output *output_cpu = InitOutputContainer(grid->size_x * grid->size_y);
    if (!output_cpu) {
        DestroyGrid(grid);
        return;
    }
    RunAStar(grid, start_x, start_y, goal_x, goal_y, output_cpu);

    ReverseHistory(output_cpu);
    PrintOutputHistory(output_cpu, path_out);

    DestroyOutputContainer(output_cpu);
    DestroyGrid(grid);
    return;
}