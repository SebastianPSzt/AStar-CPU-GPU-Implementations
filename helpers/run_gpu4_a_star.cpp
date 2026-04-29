#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../include/run_gpu4_a_star.h"
#include "../include/grid.h"
#include "../include/a_star_output.h"
#include "../include/a_star_gpu_4.h"

extern "C" __declspec(dllexport)
void run_gpu4(const char *path_in, const char *path_out)
{
    printf("ENTERED run_gpu4\n");

    Grid_2D_Device *grid = GatherGrid(path_in);

    if (!grid)
        return;

    if (grid->size_x <= 0 || grid->size_y <= 0)
    {
        DestroyGrid(grid);
        return;
    }

    int start_x = 0;
    int start_y = 0;
    int goal_x = grid->size_x - 1;
    int goal_y = grid->size_y - 1;

    AStar_Output *output_gpu = InitOutputContainer(grid->size_x * grid->size_y);
    if (!output_gpu)
    {
        DestroyGrid(grid);
        return;
    }

    InitGPU_AStar();

    Init_AStar(
        grid,
        start_x,
        start_y,
        goal_x,
        goal_y,
        1,
        output_gpu
    );

    ReverseHistory(output_gpu);
    PrintOutputHistory(output_gpu, path_out);

    DestroyOutputContainer(output_gpu);
    DestroyGrid(grid);
}