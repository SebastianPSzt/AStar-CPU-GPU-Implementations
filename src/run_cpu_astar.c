#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/grid.h"
#include "../include/pq.h"
#include "../include/bmh.h"
#include "../include/a_star_cpu.h"
#include "../include/search_log_cpu.h"

void run(const char *path_in, const char *path_out)
{
    // FILE *debug = fopen("C:/Users/thysv/source/AStar-CPU-GPU-Implementations/debug_c.txt", "w");
    // if (debug)
    // {
    //     fprintf(debug, "entered C\n");
    //     fprintf(debug, "path = %s\n", path);
    //     fflush(debug);
    // }

    Grid_2D *grid = GatherGrid(path_in);

    if (!grid)
        return;
    PrintGridFloat(grid);

    int start_x = 0;
    int start_y = 0;
    int goal_x = grid->size_x - 1;
    int goal_y = grid->size_y - 1;

    // fprintf(debug, "grid size: %d x %d\n", grid->size_x, grid->size_y);
    // fprintf(debug, "start=(%d,%d), goal=(%d,%d)\n", start_x, start_y, goal_x, goal_y);
    // fprintf(debug, "before RunAStar\n");
    // fflush(debug);

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

    SearchLogNode *log = RunAStar(grid, start_x, start_y, goal_x, goal_y);

    // fprintf(debug, "after RunAStar, log=%p\n", (void *)log);
    // fflush(debug);

    log = ReverseLog(log);
    PrintSearchLog(log, path_out);

    FreeSearchLog(log);

    DestroyGrid(grid);

    return;

    // FILE *debug = fopen("C:/Users/thysv/source/AStar-CPU-GPU-Implementations/debug_c.txt", "w");
    // if (debug)
    // {
    //     fprintf(debug, "entered C\n");
    //     fprintf(debug, "path = %s\n", path);
    //     fflush(debug);
    // }

    // FILE *f = fopen(path, "r");
    // if (!f)
    // {
    //     if (debug)
    //     {
    //         fprintf(debug, "fopen failed\n");
    //         fclose(debug);
    //     }
    //     return;
    // }

    // if (debug)
    // {
    //     fprintf(debug, "fopen succeeded\n");
    //     fflush(debug);
    // }

    // fclose(f);

    // if (debug)
    // {
    //     fprintf(debug, "leaving C\n");
    //     fclose(debug);
    // }
}