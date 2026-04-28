#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/grid.h"

#include "cuda_intellisense_fix.h"

// Bucket Parameters
#define MAX_BUCKET_SIZE 4096
#define NUM_BUCKETS 512
#define DELTA 4 // # bucket width (low delta = A*)

// Constant Memory
__constant__ int offset_x[4];
__constant__ int offset_y[4];

// GPU Helper Functions
__host__ __device__ static int manhattanDistance(int x1, int y1, int x2, int y2) {
    return abs(y2 - y1) + abs(x2 - x1);
}

// GPU
__global__ void MultiFrontierExpansion(int* bucket_nodes_d, int* bucket_sizes_d, int currentBucket, int currentBucketSize,
int* gScore_d, int* parent_d, int gridSize_x, int gridSize_y, int* gridData_d, int goalIndex_x, int goalIndex_y) {
    int thread_id = blockIdx.x * blockDim.x + threadIdx.x;
    if (thread_id >= currentBucketSize) return;

    int index = bucket_nodes_d[currentBucket * MAX_BUCKET_SIZE + thread_id];
    int index_y = index / gridSize_x;
    int index_x = index % gridSize_x;

    for (int i = 0; i < 4; i++) {
        int neighborIndex_x = (index_x + offset_x[i]);
        int neighborIndex_y = (index_y + offset_y[i]);
        int neighborIndex = neighborIndex_y * gridSize_x + neighborIndex_x;

        if (neighborIndex_x < 0 || neighborIndex_x >= gridSize_x || neighborIndex_y < 0 || neighborIndex_y >= gridSize_y) continue;
        if (gridData_d[neighborIndex] == 2) continue;

        int newVal = gScore_d[index] + 1;
        int oldVal = atomicMin(&gScore_d[neighborIndex], newVal);

        if (newVal < oldVal) {
            //if (gScore_d[neighborIndex] != newVal) return; RACE CONDITION STILL PRESENT ON JUST PARENT_D
            // Otherwise we just have duplicate nodes in some buckets but thats okay
            parent_d[neighborIndex] = index;

            int heuristicVal = manhattanDistance(neighborIndex_x, neighborIndex_y, goalIndex_x, goalIndex_y);
            int fScore = newVal + heuristicVal;

            int neighborBucket = fScore / DELTA;
            if (neighborBucket >= NUM_BUCKETS) neighborBucket = NUM_BUCKETS - 1;

            int idx = atomicAdd(&bucket_sizes_d[neighborBucket], 1);

            if (idx >= MAX_BUCKET_SIZE) {
                printf("Error: bucket size exceeded, please increase MAX_BUCKET_SIZE as overflow is not handled\n");
                return;
            }

            bucket_nodes_d[MAX_BUCKET_SIZE * neighborBucket + idx] = neighborIndex;
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

    // No need to set grid elements to 1 anymore
    // No f-score for now (search dijkstra-style)

    int* gScore_d;
    cudaMalloc((void**)&gScore_d, sizeof(int)*gridSize);
    cudaMemcpy(gScore_d, gScore, sizeof(int)*gridSize, cudaMemcpyHostToDevice);

    int* parent_d;
    cudaMalloc((void**)&parent_d, sizeof(int)*gridSize);
    cudaMemcpy(parent_d, grid->parent, sizeof(int)*gridSize, cudaMemcpyHostToDevice);

    int* gridData_d;
    cudaMalloc((void**)&gridData_d, sizeof(int)*gridSize);
    cudaMemcpy(gridData_d, grid->data, sizeof(int)*gridSize, cudaMemcpyHostToDevice);

    int* bucket_sizes_d;
    cudaMalloc((void**)&bucket_sizes_d, sizeof(int)*NUM_BUCKETS);
    cudaMemset(bucket_sizes_d, 0, sizeof(int)*NUM_BUCKETS);

    int* bucket_nodes_d;
    cudaMalloc((void**)&bucket_nodes_d, sizeof(int)*NUM_BUCKETS*MAX_BUCKET_SIZE);

    int startNode_Cost = manhattanDistance(startIndex_x, startIndex_y, goalIndex_x, goalIndex_y);
    int startNode_Bucket = startNode_Cost / DELTA;

    cudaMemcpy(&bucket_nodes_d[startNode_Bucket * MAX_BUCKET_SIZE], &startIndex, sizeof(int), cudaMemcpyHostToDevice); // bucket entry for start node
    
    int one = 1;
    cudaMemcpy(&bucket_sizes_d[startNode_Bucket], &one, sizeof(int), cudaMemcpyHostToDevice); // set bucket size

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
        MultiFrontierExpansion<<<numBlocks, 256>>>(bucket_nodes_d, bucket_sizes_d, currentBucket, currentBucketSize,
        gScore_d, parent_d, gridSize_x, gridSize_y, gridData_d, goalIndex_x, goalIndex_y);
        cudaDeviceSynchronize();

        // Get # elements added to current bucket
        int newBucketSize;
        cudaMemcpy(&newBucketSize, &bucket_sizes_d[currentBucket], sizeof(int), cudaMemcpyDeviceToHost);
        newBucketSize -= currentBucketSize;

        // Shift new elements to beginning of bucket
        // -> Potential bottleneck, better to update start/end ptrs
        if (newBucketSize > 0) {
            cudaMemcpy(&bucket_nodes_d[currentBucket * MAX_BUCKET_SIZE], 
            &bucket_nodes_d[currentBucket * MAX_BUCKET_SIZE + currentBucketSize],
            sizeof(int) * newBucketSize,
            cudaMemcpyDeviceToDevice);
        }

        // Update current bucket size on the device
        cudaMemcpy(&bucket_sizes_d[currentBucket], &newBucketSize, sizeof(int), cudaMemcpyHostToDevice);
    }

    cudaMemcpy(gScore, gScore_d, sizeof(int)*gridSize, cudaMemcpyDeviceToHost);
    cudaMemcpy(grid->parent, parent_d, sizeof(int)*gridSize, cudaMemcpyDeviceToHost);

    for (int i = 0; i < grid->size_x * grid->size_y; i++) {
        printf("Cost to get to from start for index %d: %d\n", i, gScore[i]);
    }

    cudaFree(bucket_sizes_d);
    cudaFree(bucket_nodes_d);
    cudaFree(gScore_d);
    cudaFree(parent_d);
    cudaFree(gridData_d);

    free(gScore);
}

int main(int argc, char* argv[]) {
    // Init constant memory
    int offset_xh[4] = {-1, 0, 1, 0};
    int offset_yh[4] = {0, -1, 0, 1};

    cudaMemcpyToSymbol(offset_x, offset_xh, sizeof(int) * 4, 0, cudaMemcpyHostToDevice);
    cudaMemcpyToSymbol(offset_y, offset_yh, sizeof(int) * 4, 0, cudaMemcpyHostToDevice);

    Grid_2D_Device* myGrid = CreateGrid(4, 4, 0);
    
    myGrid->data[4] = 2;
    myGrid->data[5] = 2;
    myGrid->data[6] = 2;

    Run_AStar(myGrid, 0, 0, 0, 2);

    DestroyGrid(myGrid);
}

/*
0  1  2  3
4  5  6  7
8  9  10 11
12 13 14 15
*/