#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<time.h>

#include "../include/grid.h"
#include "../include/bmh.h"

#include"cuda_intellisense_fix.h"

// Device Memory Pointers
static int* gScore_d;
static float* gridData_d;
static int* parent_d;
static int* buf_d; // [neighborIndex0, newOpenSetCost0, ...]

// Device Constant Memory
__constant__ int offset_x[4];
__constant__ int offset_y[4];

// Texture Memory
// ........................

__host__ __device__ int ManhattanDistance(int x1, int y1, int x2, int y2) {
    return abs(x2 - x1) + abs(y2 - y1);
}

__global__ void NeighborEvaluation(int* gScore, float* gridData, int* parent, int gridSize_x, int gridSize_y, 
int currentIndex, int goalIndex, int* buf) {
    // Thread id
    int id = threadIdx.x;
    if (threadIdx.x >= 4) return; // bounds safeguard

    //buf[id*2] = -1;
    //buf[id*2+1] = -1;

    // Current index 2D points
    int currentIndex_x = currentIndex % gridSize_x;
    int currentIndex_y = currentIndex / gridSize_x;

    // Goal index 2D points
    int goalIndex_x = goalIndex % gridSize_x;
    int goalIndex_y = goalIndex / gridSize_x;

    // Neighbor index 2D points
    int neighborIndex_x = currentIndex_x + offset_x[id];
    int neighborIndex_y = currentIndex_y + offset_y[id];

    // Ensure neighbor index is in bounds, else return
    if (neighborIndex_x < 0 || neighborIndex_x >= gridSize_x || neighborIndex_y < 0 || neighborIndex_y >= gridSize_y) return;

    // Neighbor index
    int neighborIndex = neighborIndex_y * gridSize_x + neighborIndex_x;
    buf[id*2] = neighborIndex;

    // Ensure neighboring point has not been visited and is not blocked, else return
    if (gridData[neighborIndex] == 1.0f) return; // consistent, admissable heuristic
    if (gridData[neighborIndex] == 2.0f) return;

    // A* neighbor update step    
    int tentative_gScore = gScore[currentIndex] + 1;
    if (tentative_gScore < gScore[neighborIndex])
    {
        parent[neighborIndex] = currentIndex;
        gScore[neighborIndex] = tentative_gScore;
        int fScore = tentative_gScore + ManhattanDistance(neighborIndex_x, neighborIndex_y, goalIndex_x, goalIndex_y);
        buf[id*2+1] = fScore;
    }
}

void RunAStar(Grid_2D_Device* grid, int startIndex_x, int startIndex_y, int goalIndex_x, int goalIndex_y) {
    int nodesVisited = 0;

    int gridSize_X = grid->size_x;
    int gridSize_Y = grid->size_y;
    int gridSize = gridSize_X*gridSize_Y;

    int startIndex = startIndex_y * gridSize_X + startIndex_x;
    int goalIndex = goalIndex_y * gridSize_X + goalIndex_x;

    BinaryMinHeap* openSet = Init_BMH(gridSize);
    Insert_BMH(openSet, startIndex, ManhattanDistance(startIndex_x, startIndex_y, goalIndex_x, goalIndex_y));

    int* gScore = (int*)malloc(sizeof(int) * gridSize);
    for (int i = 0; i < gridSize; i++)
        gScore[i] = INT_MAX;
    gScore[startIndex] = 0;

    while (openSet->size != 0) {
        Node_BMH* current = PeekMin_BMH(openSet);
        int currentIndex = current->id;
        
        if (current->id == goalIndex) {
            printf("Optimal path found!\n");
            printf("Nodes visited: %d\n", nodesVisited);
            Destroy_BMH(openSet);
            free(gScore);
            return;
        }

        RemoveMin_BMH(openSet);
        grid->data[currentIndex] = 1.0f; // Must be removed if heuristic fn is admissable but not consistent

        int* buf_h = (int*)malloc(sizeof(int) * 8);
        memset(buf_h, -1, 8 * sizeof(int));

        cudaMemcpy(gScore_d, gScore, sizeof(int) * gridSize, cudaMemcpyHostToDevice);
        cudaMemcpy(gridData_d, grid->data, sizeof(float) * gridSize, cudaMemcpyHostToDevice);
        cudaMemcpy(parent_d, grid->parent, sizeof(int) * gridSize, cudaMemcpyHostToDevice);
        cudaMemcpy(buf_d, buf_h, sizeof(int) * 8, cudaMemcpyHostToDevice);

        NeighborEvaluation<<<1, 4>>>(gScore_d, gridData_d, parent_d, grid->size_x, grid->size_y, currentIndex, goalIndex, buf_d);
        cudaDeviceSynchronize();

        cudaMemcpy(gScore, gScore_d, sizeof(int) * gridSize, cudaMemcpyDeviceToHost);
        cudaMemcpy(grid->data, gridData_d, sizeof(float) * gridSize, cudaMemcpyDeviceToHost);
        cudaMemcpy(grid->parent, parent_d, sizeof(int) * gridSize, cudaMemcpyDeviceToHost);
        cudaMemcpy(buf_h, buf_d, sizeof(int) * 8, cudaMemcpyDeviceToHost);

        for (int i = 0; i < 4; i++)
        {
            int neighborIndex = buf_h[i*2];
            int fScore = buf_h[i*2 + 1];
            if (fScore == -1) continue;
            if (GetElementById_BMH(openSet, neighborIndex)){
                UpdateWeight_BMH(openSet, neighborIndex, fScore);
            } else
            {
                Insert_BMH(openSet, neighborIndex, fScore);
            }
        }

        free(buf_h);

        nodesVisited++;
    }
    printf("Could not find a path from startIndex %d to goalIndex %d\n", startIndex, goalIndex);
    return;
}

void RunAStar_GP(Grid_2D_Device* grid, int startIndex_x, int startIndex_y, int goalIndex_x, int goalIndex_y) {
    // Variables
    int gridSize_x = grid->size_x;
    int gridSize_y = grid->size_y;
    int gridSize = gridSize_x * gridSize_y;

    // Init constant memory
    int offset_xh[4] = {-1, 0, 1, 0};
    int offset_yh[4] = {0, -1, 0, 1};

    cudaMemcpyToSymbol(offset_x, offset_xh, sizeof(int) * 4, 0, cudaMemcpyHostToDevice);
    cudaMemcpyToSymbol(offset_y, offset_yh, sizeof(int) * 4, 0, cudaMemcpyHostToDevice);

    // Init device pointers
   cudaMalloc(&gScore_d, sizeof(int) * gridSize);
   cudaMalloc(&gridData_d, sizeof(int) * gridSize);
   cudaMalloc(&parent_d, sizeof(int) * gridSize);
   cudaMalloc(&buf_d, sizeof(int) * 8);
   
   // Run A* algorithm
   RunAStar(grid, startIndex_x, startIndex_y, goalIndex_x, goalIndex_y);

    // Free memory
   cudaFree(gScore_d);
   cudaFree(gridData_d);
   cudaFree(parent_d);
   cudaFree(&buf_d);
}

int main(int argc, char* argv[]) {
    printf("compiled and executed!\n");

    Grid_2D_Device* myGrid = (Grid_2D_Device*)malloc(sizeof(Grid_2D_Device));
    myGrid->size_x = 4;
    myGrid->size_y = 4;

    myGrid->data = (float*)malloc(sizeof(float)*16);
    myGrid->parent = (int*)malloc(sizeof(int)*16);

    for (int i = 0; i<16; i++) {
        myGrid->data[i] = 0.0f;
        myGrid->parent[i] = 0;
    }

    myGrid->data[4] = 2.0f;
    myGrid->data[5] = 2.0f;
    myGrid->data[6] = 2.0f;

    time_t prevTime = time(NULL);
    RunAStar_GP(myGrid, 0, 0, 0, 2);
    time_t postTime = time(NULL);
    printf("Time taken: %ld", postTime - prevTime);

    free(myGrid->data);
    free(myGrid->parent);
    free(myGrid);

    exit(EXIT_SUCCESS);
}
// 0  1  2  3
// 4  5  6  7
// 8  9  10 11
// 12 13 14 15