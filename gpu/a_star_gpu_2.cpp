#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/bmh.h"
#include "../include/grid.h"

#include "cuda_intellisense_fix.h"

// Constant Memory
__constant__ int offset_x[4];
__constant__ int offset_y[4];

// GPU
__device__ void MultiFrontierExpansion(int* frontier_d, int* nextFrontier_d, int frontier_size_h, int* nextFrontier_size_d,
int* gScore_d, int* parent_d, int gridSize_x, int gridSize_y, float* gridData_d) {
    int thread_id = blockIdx.x * blockDim.x + threadIdx.x;
    if (thread_id >= frontier_size_h) return;

    int index = frontier_d[thread_id];
    int index_y = index / gridSize_x;
    int index_x = index % gridSize_x;

    for (int i = 0; i < 4; i++) {
        int neighborIndex_x = (index_x + offset_x[i]);
        int neighborIndex_y = (index_y + offset_y[i]);
        int neighborIndex = neighborIndex_y * gridSize_x + neighborIndex_x;

        if (neighborIndex_x < 0 || neighborIndex_x >= gridSize_x || neighborIndex_y < 0 || neighborIndex_y >= gridSize_y) continue;
        if (gridData_d[neighborIndex] == 2.0f) continue;

        int newVal = gScore_d[index] + 1;
        int oldVal = atomicMin(&gScore_d[neighborIndex], newVal);

        if (newVal < oldVal) {
            parent_d[neighborIndex] = index;
            
            int frontierIndex = atomicAdd(nextFrontier_size_d, 1);
            nextFrontier_d[frontierIndex] = neighborIndex;
        }
    }
} 

// CPU
void Run_AStar(Grid_2D_Device* grid, int startIndex_x, int startIndex_y, int goalIndex_x, int goalIndex_y) {
    int gridSize_x = grid->size_x;
    int gridSize_y = grid->size_y;
    int gridSize = gridSize_x * gridSize_y;

    int startIndex = startIndex_y * gridSize_x + startIndex_x;
    int goalIndex = goalIndex_y * gridSize_x + goalIndex_x;

    int* gScore = (int*)malloc(sizeof(int)*gridSize);
    for (int i = 0; i < gridSize; i++) gScore[i] = INT_MAX; // set each byte to maximum value
    gScore[startIndex] = 0;

    int frontier_size_h = 1; // read-only primitive on device-side so no need to allocate memory

    // No need to set grid elements to 1.0f anymore
    // No f-score for now (search dijkstra-style)

    int* gScore_d;
    cudaMalloc((void**)&gScore_d, sizeof(int)*gridSize);
    cudaMemcpy(gScore_d, gScore, sizeof(int)*gridSize, cudaMemcpyHostToDevice);

    int* parent_d;
    cudaMalloc((void**)&parent_d, sizeof(int)*gridSize);
    cudaMemcpy(parent_d, grid->parent, sizeof(int)*gridSize, cudaMemcpyHostToDevice);

    int* frontier_d;
    cudaMalloc((void**)&frontier_d, sizeof(int)*gridSize);
    cudaMemcpy(frontier_d, &startIndex, sizeof(int), cudaMemcpyHostToDevice);

    int* nextFrontier_d;
    cudaMalloc((void**)&nextFrontier_d, sizeof(int)*gridSize);

    int* nextFrontier_size_d;
    cudaMalloc((void**)&nextFrontier_size_d, sizeof(int));

    float* gridData_d;
    cudaMalloc((void**)&gridData_d, sizeof(float)*gridSize);
    cudaMemcpy(gridData_d, grid->data, sizeof(float)*gridSize, cudaMemcpyHostToDevice);

    // Keep going till no more nodes in frontier to explore
    while (frontier_size_h > 0) {
        // Reset next frontier size to 0
        cudaMemset(nextFrontier_size_d, 0, sizeof(int));

        // Calculate thread block size
        size_t numBlocks = (frontier_size_h + 255) / 256;

        // Call kernel
        MultiFrontierExpansion<<<numBlocks, 256>>>(frontier_d, nextFrontier_d, frontier_size_h, nextFrontier_size_d,
        gScore_d, parent_d, gridSize_x, gridSize_y, gridData_d);
        cudaDeviceSynchronize();

        // Move next values to past
        cudaMemcpy(&frontier_size_h, nextFrontier_size_d, sizeof(int), cudaMemcpyDeviceToHost);
        
        // Swap
        int* temp = frontier_d;
        frontier_d = nextFrontier_d;
        nextFrontier_d = temp;
    }

    cudaMemcpy(gScore, gScore_d, sizeof(int)*gridSize, cudaMemcpyDeviceToHost);
    cudaMemcpy(grid->parent, parent_d, sizeof(int)*gridSize, cudaMemcpyDeviceToHost);

    for (int i = 0; i < grid->size_x * grid->size_y; i++) {
        printf("Cost to get to from start: %d\n", gScore[i]);
    }

    cudaFree(frontier_d);
    cudaFree(nextFrontier_d);
    cudaFree(nextFrontier_size_d);
    cudaFree(gScore_d);
    cudaFree(parent_d);
    cudaFree(gridData_d);

    free(gScore);
}

/*
The whole idea is to continuously expand the frontier (starting with just the start node) until there are no more nodes to expand
Process:
1) Set frontier_size_d to whatever frontier_size_h is, then set nextFrontier_size_d to 0
2) Call kernel, passing in frontier variables, gScore, and parent array
3) Kernel does work, expanding every node in frontier_d, and adding valid neighbors whose score was updated to nextFrontier_d
4) Host copies nextFrontier_size_d to frontier_size_h
5) Host swaps frontier_d and nextFrontier_d
*/

int main(int argc, char* argv[]) {
    // Init constant memory
    int offset_xh[4] = {-1, 0, 1, 0};
    int offset_yh[4] = {0, -1, 0, 1};

    cudaMemcpyToSymbol(offset_x, offset_xh, sizeof(int) * 4, 0, cudaMemcpyHostToDevice);
    cudaMemcpyToSymbol(offset_y, offset_yh, sizeof(int) * 4, 0, cudaMemcpyHostToDevice);

    Grid_2D_Device* myGrid = CreateGrid(4, 4, 0.0f);
    
    myGrid->data[4] = 2.0f;
    myGrid->data[5] = 2.0f;
    myGrid->data[6] = 2.0f;

    Run_AStar(myGrid, 0, 0, 0, 2);

    DestroyGrid(myGrid);
}

/*
0  1  2  3
4  5  6  7
8  9  10 11
12 13 14 15
*/