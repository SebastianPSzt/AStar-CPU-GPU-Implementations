#include <stdio.h>
#include <stdlib.h>

#include "../include/correctness.h"
#include "../include/grid.h"
#include "../include/a_star_output.h"
#include "../include/dfs_maze_generator.h"
#include "../include/a_star_cpu.h"
#include "../include/a_star_gpu_4.h"

// Test Macros
#define N 5
#define SIZE_X 128
#define SIZE_Y 128

void testCorrectness() {
    int mismatch = 0;

    InitGPU_AStar();

    for (int i = 0; i < N; i++) {
        Grid_2D_Device* grid = GenerateMaze(SIZE_X, SIZE_Y, 0, 0, 0, 0, &i);
        AStar_Output* output_cpu = InitOutputContainer(SIZE_X * SIZE_Y);
        AStar_Output* output_gpu = InitOutputContainer(SIZE_X * SIZE_Y);

        // CPU Run
        RunAStar(grid, 0, 0, SIZE_X - 2, SIZE_Y - 2, output_cpu);

        // GPU Run
        Init_AStar(grid, 0, 0, SIZE_X - 2, SIZE_Y - 2, 1, output_gpu);

        if (output_cpu->validPath != output_gpu->validPath) {
            printf("CPU and GPU disagree on there being a valid path!\n");
            mismatch = 1;
            continue;
        }

        if (output_cpu->validPath == 0 && output_gpu->validPath == 0) {
            printf("no valid path!\n");
            continue;
        } else {
            printf("There is a valid path!\n");
        }

        if (output_cpu->bestCost != output_gpu->bestCost) {
            printf("Best cost mismatch on test %d, cpu: %d, gpu: %d\n", i, output_cpu->bestCost, output_gpu->bestCost);
            mismatch = 1;
        }

        if (output_cpu->pathSize != output_gpu->pathSize) {
            printf("Path size mismatch on test %d, cpu: %d, gpu: %d\n", i, output_cpu->pathSize, output_gpu->pathSize);
            mismatch = 1;
        }

        for (int j = 0; j < output_cpu->pathSize; j++) {
            if (output_cpu->path[j] != output_gpu->path[j]) {
                printf("Path mismatch on test %d index %d, cpu: %d, gpu: %d\n", i, j, output_cpu->path[j], output_gpu->path[j]);
                mismatch = 1;
            }
        }

        DestroyGrid(grid);
        DestroyOutputContainer(output_cpu);
        DestroyOutputContainer(output_gpu);
    }

    printf("Mismatch? (yes if 1) %d\n", mismatch);
}