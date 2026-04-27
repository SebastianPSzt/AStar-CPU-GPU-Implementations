#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/a_star_gpu_4.h"
#include "../include/grid.h"
#include "../include/dfs_maze_generator.h"
#include "../include/a_star_helpers.h"

#include "cuda_intellisense_fix.h"

// Bucket Macros
#define MAX_BUCKET_SIZE 4096
#define NUM_BUCKETS 512
#define DELTA 4 // # bucket width (low delta = A*)

// Direction Macros
#define FORWARD 0
#define BACKWARD 1

// Constant Memory
__constant__ int offset_x[4];
__constant__ int offset_y[4];

// GPU Helper Functions
__host__ __device__ int manhattanDistance(int x1, int y1, int x2, int y2) {
    return abs(y2 - y1) + abs(x2 - x1);
}

// GPU
__global__ void MultiFrontierExpansion(
int* bucket_nodes_d,
int* bucket_directions_d,
int* bucket_sizes_d,
int currentBucket,
int currentBucketSize,
int* forward_gScore_d,
int* backward_gScore_d,
int gridSize_x,
int gridSize_y,
float* gridData_d, 
int startIndex_x, int startIndex_y,
int goalIndex_x, int goalIndex_y,
int* bestMeetCost_d,
int* bestMeetIndex_d)
{
    int thread_id = blockIdx.x * blockDim.x + threadIdx.x;
    if (thread_id >= currentBucketSize) return;

    int index = bucket_nodes_d[currentBucket * MAX_BUCKET_SIZE + thread_id];
    int index_y = index / gridSize_x;
    int index_x = index % gridSize_x;

    int dir = bucket_directions_d[currentBucket * MAX_BUCKET_SIZE + thread_id];

    for (int i = 0; i < 4; i++) {
        int neighborIndex_x = (index_x + offset_x[i]);
        int neighborIndex_y = (index_y + offset_y[i]);
        int neighborIndex = neighborIndex_y * gridSize_x + neighborIndex_x;

        if (neighborIndex_x < 0 || neighborIndex_x >= gridSize_x || neighborIndex_y < 0 || neighborIndex_y >= gridSize_y) continue;
        if (gridData_d[neighborIndex] == 2.0f) continue;

        if (dir == FORWARD) {
            int newVal = forward_gScore_d[index] + 1;
            int oldVal = atomicMin(&forward_gScore_d[neighborIndex], newVal);

            if (newVal < oldVal) {
                // update lowest score
                int otherScore = atomicAdd(&backward_gScore_d[neighborIndex], 0);
                if (otherScore != INT_MAX) {
                    int old = atomicMin(bestMeetCost_d, newVal + otherScore);
                    if (newVal + otherScore < old) *bestMeetIndex_d = neighborIndex;
                }

                int heuristicVal = manhattanDistance(neighborIndex_x, neighborIndex_y, goalIndex_x, goalIndex_y);
                int fScore = newVal + heuristicVal;

                int bucket = fScore / DELTA;
                if (bucket >= NUM_BUCKETS) bucket = NUM_BUCKETS - 1;

                int idx = atomicAdd(&bucket_sizes_d[bucket], 1);

                if (idx < MAX_BUCKET_SIZE) {
                    bucket_nodes_d[bucket * MAX_BUCKET_SIZE + idx] = neighborIndex;
                    bucket_directions_d[bucket * MAX_BUCKET_SIZE + idx] = FORWARD;
                } else {
                    printf("error\n");
                    return;
                }
            }
        } else if (dir == BACKWARD) {
            int newVal = backward_gScore_d[index] + 1;
            int oldVal = atomicMin(&backward_gScore_d[neighborIndex], newVal);

            if (newVal < oldVal) {
                //update lowest score
                int otherScore = atomicAdd(&forward_gScore_d[neighborIndex], 0);
                if (otherScore != INT_MAX) {
                    int old = atomicMin(bestMeetCost_d, newVal + otherScore);
                    if (newVal + otherScore < old) *bestMeetIndex_d = neighborIndex;
                }

                int heuristicVal = manhattanDistance(neighborIndex_x, neighborIndex_y, startIndex_x, startIndex_y);
                int fScore = newVal + heuristicVal;

                int bucket = fScore / DELTA;
                if (bucket >= NUM_BUCKETS) bucket = NUM_BUCKETS - 1;

                int idx = atomicAdd(&bucket_sizes_d[bucket], 1);

                if (idx < MAX_BUCKET_SIZE) {
                    bucket_nodes_d[bucket * MAX_BUCKET_SIZE + idx] = neighborIndex;
                    bucket_directions_d[bucket * MAX_BUCKET_SIZE + idx] = BACKWARD;
                } else {
                    printf("error\n");
                    return;
                }
            }
        }
            // RACE CONDITION STILL PRESENT ON JUST PARENT_D
            // Otherwise we just have duplicate nodes in some buckets but thats okay
            //parent_d[neighborIndex] = index;
    }
} 

// CPU
void Run_AStar(Grid_2D_Device* grid, int startIndex_x, int startIndex_y, int goalIndex_x, int goalIndex_y, int* forward_gScore_d, 
int* backward_gScore_d, float* gridData_d, int* bucket_sizes_d, int* bucket_nodes_d, int* bucket_directions_d, int* bestMeetCost_d, int* bestMeetIndex_d) {
    // ----- Variables -----
    int gridSize_x = grid->size_x;
    int gridSize_y = grid->size_y;
    int gridSize = gridSize_x * gridSize_y;

    int startIndex = startIndex_y * gridSize_x + startIndex_x;
    int goalIndex = goalIndex_y * gridSize_x + goalIndex_x;

    // Keep going until no more non-empty buckets
    while (1) {
        // Get bucket to process
        int currentBucket = -1;
        int currentBucketSize = -1;

        int* bucket_sizes_h = (int*)malloc(sizeof(int)*NUM_BUCKETS);
        cudaMemcpy(bucket_sizes_h, bucket_sizes_d, sizeof(int)*NUM_BUCKETS, cudaMemcpyDeviceToHost);

        for (int i = 0; i < NUM_BUCKETS; i++) {
            size_t size = bucket_sizes_h[i];
            if (size != 0) {
                currentBucket = i;
                currentBucketSize = size;
                break;
            }
        }

        free(bucket_sizes_h);
        if (currentBucket == -1) break;
        
        // Compute #thread_blocks (guaranteed to be >=1 if non-empty bucket exists; always rounds up)
        size_t numBlocks = (currentBucketSize + 255) / 256;

        // Call kernel
        MultiFrontierExpansion<<<numBlocks, 256>>>(bucket_nodes_d, bucket_directions_d, bucket_sizes_d, currentBucket, currentBucketSize,
        forward_gScore_d, backward_gScore_d, gridSize_x, gridSize_y, gridData_d, startIndex_x, startIndex_y, goalIndex_x, goalIndex_y, bestMeetCost_d, bestMeetIndex_d);
        cudaDeviceSynchronize();

        // Get # elements added to current bucket
        int newBucketSize;
        cudaMemcpy(&newBucketSize, &bucket_sizes_d[currentBucket], sizeof(int), cudaMemcpyDeviceToHost);
        newBucketSize -= currentBucketSize;

        // Shift new elements to beginning of bucket
        // -> Potential bottleneck, better to update start/end ptrs
        if (newBucketSize > 0) {
            // Shift indices
            cudaMemcpy(&bucket_nodes_d[currentBucket * MAX_BUCKET_SIZE], 
            &bucket_nodes_d[currentBucket * MAX_BUCKET_SIZE + currentBucketSize],
            sizeof(int) * newBucketSize,
            cudaMemcpyDeviceToDevice);

            // Shift directions
            cudaMemcpy(&bucket_directions_d[currentBucket * MAX_BUCKET_SIZE],
            &bucket_directions_d[currentBucket * MAX_BUCKET_SIZE + currentBucketSize],
            sizeof(int)*newBucketSize,
            cudaMemcpyDeviceToDevice);
        }

        // Update current bucket size on the device
        cudaMemcpy(&bucket_sizes_d[currentBucket], &newBucketSize, sizeof(int), cudaMemcpyHostToDevice);

        // Additional termination condition : small enough best total cost between both directions
        int bestMeetCost_h;
        cudaMemcpy(&bestMeetCost_h, bestMeetCost_d, sizeof(int), cudaMemcpyDeviceToHost);
        if (bestMeetCost_h <= currentBucket * DELTA) break;
    }
}

void Init_AStar(Grid_2D_Device* grid, int startIndex_x, int startIndex_y, int goalIndex_x, int goalIndex_y, int print) {
    // ----- Variables -----
    int gridSize_x = grid->size_x;
    int gridSize_y = grid->size_y;
    int gridSize = gridSize_x * gridSize_y;

    int startIndex = startIndex_y * gridSize_x + startIndex_x;
    int goalIndex = goalIndex_y * gridSize_x + goalIndex_x;

    // ----- Host gScore Arrays -----
    int* gScore = (int*)malloc(sizeof(int)*gridSize);
    for (int i = 0; i < gridSize; i++) gScore[i] = INT_MAX; // set each byte to maximum value

    // ----- Device forrward and backward gScore Arrays -----
    int zero = 0;

    int* forward_gScore_d;
    cudaMalloc((void**)&forward_gScore_d, sizeof(int)*gridSize);
    cudaMemcpy(forward_gScore_d, gScore, sizeof(int)*gridSize, cudaMemcpyHostToDevice);
    cudaMemcpy(&forward_gScore_d[startIndex], &zero, sizeof(int), cudaMemcpyHostToDevice);

    int* backward_gScore_d;
    cudaMalloc((void**)&backward_gScore_d, sizeof(int)*gridSize);
    cudaMemcpy(backward_gScore_d, gScore, sizeof(int)*gridSize, cudaMemcpyHostToDevice);
    cudaMemcpy(&backward_gScore_d[goalIndex], &zero, sizeof(int), cudaMemcpyHostToDevice);

    // ----- Device Grid Data -----
    float* gridData_d;
    cudaMalloc((void**)&gridData_d, sizeof(float)*gridSize);
    cudaMemcpy(gridData_d, grid->data, sizeof(float)*gridSize, cudaMemcpyHostToDevice);

    // ----- Device Bucket Queue Sizes, Nodes & Directions -----
    int* bucket_sizes_d;
    cudaMalloc((void**)&bucket_sizes_d, sizeof(int)*NUM_BUCKETS);
    cudaMemset(bucket_sizes_d, 0, sizeof(int)*NUM_BUCKETS);

    int* bucket_nodes_d;
    cudaMalloc((void**)&bucket_nodes_d, sizeof(int)*NUM_BUCKETS*MAX_BUCKET_SIZE);

    int* bucket_directions_d;
    cudaMalloc((void**)&bucket_directions_d, sizeof(int)*NUM_BUCKETS*MAX_BUCKET_SIZE);

    // ----- Device BestMeetCost Between Start and End Frontiers -----
    int* bestMeetCost_d;
    cudaMalloc((void**)&bestMeetCost_d, sizeof(int));
    int inf = INT_MAX;
    cudaMemcpy(bestMeetCost_d, &inf, sizeof(int), cudaMemcpyHostToDevice);

    // ----- Device BestMeetIndex -----
    int* bestMeetIndex_d;
    cudaMalloc((void**)&bestMeetIndex_d, sizeof(int));

    // ----- Add Start/Goal Indices to Bucket Queue -----
    
    // Start Index
    int startBucket = manhattanDistance(startIndex_x, startIndex_y, goalIndex_x, goalIndex_y) / DELTA;
    int dirF = FORWARD;

    cudaMemcpy(&bucket_nodes_d[startBucket * MAX_BUCKET_SIZE], &startIndex, sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(&bucket_directions_d[startBucket * MAX_BUCKET_SIZE], &dirF, sizeof(int), cudaMemcpyHostToDevice);
    
    // Goal Index
    int goalBucket = startBucket;
    int dirB = BACKWARD;

    cudaMemcpy(&bucket_nodes_d[goalBucket * MAX_BUCKET_SIZE + 1], &goalIndex, sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(&bucket_directions_d[goalBucket * MAX_BUCKET_SIZE + 1], &dirB, sizeof(int), cudaMemcpyHostToDevice);

    // Set Bucket Size
    // -> Assumption that heuristic is commutative
    int two = 2;
    cudaMemcpy(&bucket_sizes_d[startBucket], &two, sizeof(int), cudaMemcpyHostToDevice);

    // ----- Call AStar Functions -----
    Run_AStar(grid, startIndex_x, startIndex_y, goalIndex_x, goalIndex_y, forward_gScore_d, backward_gScore_d, 
    gridData_d, bucket_sizes_d, bucket_nodes_d, bucket_directions_d, bestMeetCost_d, bestMeetIndex_d);

    if (print) {
        int* bestMeetCost_h = (int*)malloc(sizeof(int));
        int* bestMeetIndex_h = (int*)malloc(sizeof(int));
        int* forward_gScore_h = (int*)malloc(sizeof(int) * gridSize);
        int* backward_gScore_h = (int*)malloc(sizeof(int) * gridSize);

        cudaMemcpy(bestMeetCost_h, bestMeetCost_d, sizeof(int), cudaMemcpyDeviceToHost);
        cudaMemcpy(bestMeetIndex_h, bestMeetIndex_d, sizeof(int), cudaMemcpyDeviceToHost);
        cudaMemcpy(forward_gScore_h, forward_gScore_d, sizeof(int) * gridSize, cudaMemcpyDeviceToHost);
        cudaMemcpy(backward_gScore_h, backward_gScore_d, sizeof(int) * gridSize, cudaMemcpyDeviceToHost);

        int reachedPath = (*bestMeetCost_h == INT_MAX) ? 0 : 1;

        Print_AStar(grid, reachedPath, *bestMeetCost_h, forward_gScore_h, backward_gScore_h, startIndex, goalIndex, *bestMeetIndex_h);

        free(bestMeetCost_h);
        free(bestMeetIndex_h);
        free(forward_gScore_h);
        free(backward_gScore_h);
    }

    Clean_AStar(gScore, forward_gScore_d, backward_gScore_d, gridData_d, bucket_sizes_d, bucket_nodes_d, bucket_directions_d, bestMeetCost_d, bestMeetIndex_d);
}

void Print_AStar(Grid_2D_Device* grid, int reachedPath, int bestCost, int* forward_gScore_h, int* backward_gScore_h, 
int startIndex, int goalIndex, int bestMeetIndex) {
    if (!reachedPath) {
        printf("Could not find path between start and goal indices\n");
        return;
    }
    
    printf("----------------------------------------------\n");
    
    printf ("(%d -> %d) Cost: %d\n", startIndex, goalIndex, bestCost);
    
    printf("----------------------------------------------\n");
    printf("Path from start to goal:\n");

    int* path_firstHalf = (int*)malloc(sizeof(int) * grid->size_x * grid->size_y);
    int length_firstHalf = GetPathFromStartToGoal(grid, forward_gScore_h, startIndex, bestMeetIndex, path_firstHalf);

    int* path_secondHalf = (int*)malloc(sizeof(int) * grid->size_x * grid->size_y);
    int length_secondHalf = GetPathFromStartToGoal(grid, backward_gScore_h, goalIndex, bestMeetIndex, path_secondHalf);

    for (int i = 0; i < length_firstHalf; i++) {
        int index = length_firstHalf - 1 - i;
        printf("Grid_Index: %d\n", path_firstHalf[index]);
    }

    for (int i = 1; i < length_secondHalf; i++) {
        int index = i;
        printf("Grid_Index: %d\n", path_secondHalf[index]);
    }

    free(path_firstHalf);
    free(path_secondHalf);

    printf("----------------------------------------------\n");
}

void Clean_AStar(int* gScore, int* forward_gScore_d, int* backward_gScore_d, float* gridData_d, 
int* bucket_sizes_d, int* bucket_nodes_d, int* bucket_directions_d, int* bestMeetCost_d, int* bestMeetIndex_d) {
    // ----- Host Deallocation -----
    free(gScore);

    // ----- Device Deallocation -----
    cudaFree(forward_gScore_d);
    cudaFree(backward_gScore_d);
    cudaFree(gridData_d);
    cudaFree(bucket_sizes_d);
    cudaFree(bucket_nodes_d);
    cudaFree(bucket_directions_d);
    cudaFree(bestMeetCost_d);
    cudaFree(bestMeetIndex_d);
}

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

    Init_AStar(myGrid, 0, 0, 0, 2, 0);

    DestroyGrid(myGrid);
}

/*
0  1  2  3
4  5  6  7
8  9  10 11
12 13 14 15
*/