#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/a_star_gpu_4.h"
#include "../include/grid.h"
#include "../include/a_star_output.h"
#include "../include/dfs_maze_generator.h"

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
__host__ __device__ static int manhattanDistance(int x1, int y1, int x2, int y2) {
    return abs(y2 - y1) + abs(x2 - x1);
}

// GPU
__global__ void SmallestBucketReduction(int* bucket_sizes_d, int startBucket, int* next_bucket_d) {
    __shared__ int sharedData[NUM_BUCKETS];

    int id = threadIdx.x;
    int bucketId = startBucket + id;

    if (bucketId < NUM_BUCKETS && bucket_sizes_d[bucketId] > 0) {
        sharedData[id] = bucketId;
    } else {
        sharedData[id] = NUM_BUCKETS;
    }

    __syncthreads();

    for (int stride = blockDim.x / 2; stride > 0; stride >>= 1) {
        if (id < stride) {
            int val_current = sharedData[id];
            int val_other = sharedData[id + stride];
            sharedData[id] = val_other < val_current ? val_other : val_current;
        }
        __syncthreads();
    }

    if (id == 0) {
        *next_bucket_d = sharedData[0] == NUM_BUCKETS ? -1 : sharedData[0];
    }
}

__global__ void MultiFrontierExpansion(
int* bucket_nodes_d,
int* bucket_sizes_d,
int currentBucket,
int currentBucketSize,
int* forward_gScore_d,
int* backward_gScore_d,
int gridSize_x,
int gridSize_y,
int* gridData_d, 
int startIndex_x, int startIndex_y,
int goalIndex_x, int goalIndex_y,
int* bestMeetCost_d,
int* bestMeetIndex_d)
{
    int thread_id = blockIdx.x * blockDim.x + threadIdx.x;
    if (thread_id >= currentBucketSize) return;

    int node = bucket_nodes_d[currentBucket * MAX_BUCKET_SIZE + thread_id];

    int index = node >> 1;
    int index_y = index / gridSize_x;
    int index_x = index % gridSize_x;

    int dir = node & 1;

    for (int i = 0; i < 4; i++) {
        int neighborIndex_x = (index_x + offset_x[i]);
        int neighborIndex_y = (index_y + offset_y[i]);
        int neighborIndex = neighborIndex_y * gridSize_x + neighborIndex_x;

        if (neighborIndex_x < 0 || neighborIndex_x >= gridSize_x || neighborIndex_y < 0 || neighborIndex_y >= gridSize_y) continue;
        if (gridData_d[neighborIndex] == 2) continue;

        // Choose scores based on direction (FORWARD | BACKWARD)
        // -> allows massive reduction in redundant code
        int isForward = (dir == FORWARD);
        int gScore = isForward ? forward_gScore_d[index] : backward_gScore_d[index];
        int* neighbor_gScore = isForward ? &forward_gScore_d[neighborIndex] : &backward_gScore_d[neighborIndex];
        int* otherDir_gScore = isForward ? &backward_gScore_d[neighborIndex] : &forward_gScore_d[neighborIndex];
        int heuristicGoalIndex_x = isForward ? goalIndex_x : startIndex_x;
        int heuristicGoalIndex_y = isForward ? goalIndex_y : startIndex_y;

        int newVal = gScore + 1;
        int oldVal = atomicMin(neighbor_gScore, newVal);

        if (newVal < oldVal) {
            // update lowest score
            int otherScore = atomicAdd(otherDir_gScore, 0);
            if (otherScore != INT_MAX) {
                int old = atomicMin(bestMeetCost_d, newVal + otherScore);
                if (newVal + otherScore < old) *bestMeetIndex_d = neighborIndex;
            }

            int heuristicVal = manhattanDistance(neighborIndex_x, neighborIndex_y, heuristicGoalIndex_x, heuristicGoalIndex_y);
            int fScore = newVal + heuristicVal;

            int bucket = fScore / DELTA;
            if (bucket >= NUM_BUCKETS) bucket = NUM_BUCKETS - 1;

            int idx = atomicAdd(&bucket_sizes_d[bucket], 1);

            if (idx < MAX_BUCKET_SIZE) {
                bucket_nodes_d[bucket * MAX_BUCKET_SIZE + idx] = neighborIndex << 1 | dir;
            } else {
                printf("error\n");
                return;
            }
        }
    }
} 

// CPU
void Run_AStar(Grid_2D_Device* grid, int startIndex_x, int startIndex_y, int goalIndex_x, int goalIndex_y, int* forward_gScore_d, int* backward_gScore_d, 
int* gridData_d, int* bucket_sizes_d, int* bucket_nodes_d, int* next_bucket_d, int* bestMeetCost_d, int* bestMeetIndex_d, int startBucket, AStar_Output* output) {
    // ----- Variables -----
    int gridSize_x = grid->size_x;
    int gridSize_y = grid->size_y;

    //int currentBucket = startBucket;
    //int currentBucketSize = -1;

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

        // --------------------------------------------------
        // Save all nodes currently on this frontier to output
        // --------------------------------------------------
        printf("---- Frontier Debug ----\n");
        printf("Bucket: %d, Size: %d\n", currentBucket, currentBucketSize);

        int* frontier_nodes_h = (int*)malloc(sizeof(int) * currentBucketSize);
        if (!frontier_nodes_h) {
            printf("Malloc failed\n");
            return;
        }

        cudaError_t err = cudaMemcpy(
            frontier_nodes_h,
            &bucket_nodes_d[currentBucket * MAX_BUCKET_SIZE],
            sizeof(int) * currentBucketSize,
            cudaMemcpyDeviceToHost
        );

        if (err != cudaSuccess) {
            printf("cudaMemcpy failed: %s\n", cudaGetErrorString(err));
            free(frontier_nodes_h);
            return;
        }

        printf("First few raw nodes:\n");
        for (int i = 0; i < currentBucketSize && i < 10; i++) {
            printf("%d ", frontier_nodes_h[i]);
        }
        printf("\n");

        Output_Node* node = new Output_Node;
        node->ids_explored = new int[currentBucketSize];
        node->num_explored = currentBucketSize;
        node->next = output->history;

        printf("Decoded indices:\n");
        for (int i = 0; i < currentBucketSize; i++) {
            int decoded = frontier_nodes_h[i] >> 1;
            node->ids_explored[i] = decoded;

            if (i < 10) {
                printf("%d ", decoded);
            }
        }
        printf("\n");

        output->history = node;

        printf("History head now has %d nodes\n", node->num_explored);

        free(frontier_nodes_h);
        printf("------------------------\n");
        // --------------------------------------------------
        
        // Count number of nodes explored
        output->nodesExplored += currentBucketSize;
        
        // Compute #thread_blocks (guaranteed to be >=1 if non-empty bucket exists; always rounds up)
        size_t numBlocks = (currentBucketSize + 255) / 256;

        // Call kernel
        MultiFrontierExpansion<<<numBlocks, 256>>>(bucket_nodes_d, bucket_sizes_d, currentBucket, currentBucketSize,
        forward_gScore_d, backward_gScore_d, gridSize_x, gridSize_y, gridData_d, startIndex_x, startIndex_y, goalIndex_x, goalIndex_y, bestMeetCost_d, bestMeetIndex_d);
        cudaDeviceSynchronize();

        // --- Check kernel ---
        cudaError_t kerr = cudaGetLastError();
        if (kerr != cudaSuccess) {
            printf("Kernel launch error: %s\n", cudaGetErrorString(kerr));
        }

        cudaError_t syncErr = cudaDeviceSynchronize();
        if (syncErr != cudaSuccess) {
            printf("Kernel sync error: %s\n", cudaGetErrorString(syncErr));
        }

        // --- Get bucket size AFTER kernel ---
        int rawBucketSize;
        cudaMemcpy(&rawBucketSize, &bucket_sizes_d[currentBucket], sizeof(int), cudaMemcpyDeviceToHost);

        printf("Bucket %d old size: %d, raw device size after kernel: %d\n",
            currentBucket, currentBucketSize, rawBucketSize);

        // --- Compute how many NEW nodes were added ---
        int newBucketSize = rawBucketSize - currentBucketSize;

        printf("New nodes added to SAME bucket: %d\n", newBucketSize);
        
        // Shift new elements to beginning of bucket
        // -> Potential bottleneck, better to update start/end ptrs
        if (newBucketSize > 0) {
            // Shift nodes (index << 1 | direction)
            cudaMemcpy(&bucket_nodes_d[currentBucket * MAX_BUCKET_SIZE], 
            &bucket_nodes_d[currentBucket * MAX_BUCKET_SIZE + currentBucketSize],
            sizeof(int) * newBucketSize,
            cudaMemcpyDeviceToDevice);
        }

        // Update current bucket size on the device
        cudaMemcpy(&bucket_sizes_d[currentBucket], &newBucketSize, sizeof(int), cudaMemcpyHostToDevice);

        // Additional termination condition : small enough best total cost between both directions
        int bestMeetCost_h;
        cudaMemcpy(&bestMeetCost_h, bestMeetCost_d, sizeof(int), cudaMemcpyDeviceToHost);
        printf("bestMeetCost_h: %d, cutoff: %d\n", bestMeetCost_h, currentBucket * DELTA);
        if (bestMeetCost_h <= currentBucket * DELTA) break;
    }
}

void Init_AStar(Grid_2D_Device* grid, int startIndex_x, int startIndex_y, int goalIndex_x, int goalIndex_y, 
int print, AStar_Output* output) {
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
    int* gridData_d;
    cudaMalloc((void**)&gridData_d, sizeof(int)*gridSize);
    cudaMemcpy(gridData_d, grid->data, sizeof(int)*gridSize, cudaMemcpyHostToDevice);

    // ----- Device Bucket Queue Sizes, Packed Nodes (INDEX << 1 | DIRECTION) -----
    int* bucket_sizes_d;
    cudaMalloc((void**)&bucket_sizes_d, sizeof(int)*NUM_BUCKETS);
    cudaMemset(bucket_sizes_d, 0, sizeof(int)*NUM_BUCKETS);

    int* bucket_nodes_d;
    cudaMalloc((void**)&bucket_nodes_d, sizeof(int)*NUM_BUCKETS*MAX_BUCKET_SIZE);

    // ----- Device Next Bucket -----
    int* next_bucket_d;
    cudaMalloc((void**)&next_bucket_d, sizeof(int));
    int num = -1;
    cudaMemcpy(next_bucket_d, &num, sizeof(int), cudaMemcpyHostToDevice);

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
    int packedStartIndex = startIndex << 1 | FORWARD;

    cudaMemcpy(&bucket_nodes_d[startBucket * MAX_BUCKET_SIZE], &packedStartIndex, sizeof(int), cudaMemcpyHostToDevice);
    
    // Goal Index
    int goalBucket = startBucket;
    int packedGoalIndex = goalIndex << 1 | BACKWARD;

    cudaMemcpy(&bucket_nodes_d[goalBucket * MAX_BUCKET_SIZE + 1], &packedGoalIndex, sizeof(int), cudaMemcpyHostToDevice);

    // Set Bucket Size
    // -> Assumption that heuristic is commutative
    int two = 2;
    cudaMemcpy(&bucket_sizes_d[startBucket], &two, sizeof(int), cudaMemcpyHostToDevice);

    // ----- Call AStar Functions -----
    Run_AStar(grid, startIndex_x, startIndex_y, goalIndex_x, goalIndex_y, forward_gScore_d, backward_gScore_d, 
    gridData_d, bucket_sizes_d, bucket_nodes_d, next_bucket_d, bestMeetCost_d, bestMeetIndex_d, startBucket, output);

    // bestCost for output
    cudaMemcpy(&(output->bestCost), bestMeetCost_d, sizeof(int), cudaMemcpyDeviceToHost);

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

        Print_AStar(grid, reachedPath, *bestMeetCost_h, forward_gScore_h, backward_gScore_h, startIndex, goalIndex, *bestMeetIndex_h, output);

        free(bestMeetCost_h);
        free(bestMeetIndex_h);
        free(forward_gScore_h);
        free(backward_gScore_h);
    }

    Clean_AStar(gScore, forward_gScore_d, backward_gScore_d, gridData_d, bucket_sizes_d, bucket_nodes_d, next_bucket_d, 
    bestMeetCost_d, bestMeetIndex_d);
}

/**
 * @brief starting at goal node, works its way back to start, creating a path as it goes
 * @note there must be a path between startIndex and goalIndex, otherwise results are undefined
 * @param path array to save path into, assumes length will be valid
 * @return length of path
 */
int GetPathFromStartToGoal(Grid_2D_Device* grid, int* gScore, int startIndex, int goalIndex, int* path) {
    int offset_xh[4] = {-1, 0, 1, 0};
    int offset_yh[4] = {0, -1, 0, 1};

    int length = 0;
    int currentIndex = goalIndex;
    int currentCost = gScore[goalIndex];

    while (currentCost >= 0) {
        path[length] = currentIndex;
        length++;

        // finished
        if (currentCost == 0) break;

        int currentIndex_x = currentIndex % grid->size_x;
        int currentIndex_y = currentIndex / grid->size_x;

        for (int i = 0; i < 4; i++) {
            int neighborIndex_y = currentIndex_y + offset_yh[i];
            int neighborIndex_x = currentIndex_x + offset_xh[i];
            int neighborIndex = neighborIndex_y * grid->size_x + neighborIndex_x;

            if (neighborIndex_x < 0 || neighborIndex_x >= grid->size_x || neighborIndex_y < 0 || neighborIndex_y >= grid->size_y) continue;
            if (gScore[neighborIndex] != (currentCost - 1)) continue;

            currentIndex = neighborIndex;
            currentCost -= 1;

            break;
        }
    }

    return length;
}

void Print_AStar(Grid_2D_Device* grid, int reachedPath, int bestCost, int* forward_gScore_h, int* backward_gScore_h, 
int startIndex, int goalIndex, int bestMeetIndex, AStar_Output* output) {
    if (!reachedPath) {
        //printf("Could not find path between start and goal indices\n");
        return;
    }
    
    // Set validPath in output
    output->validPath = 1;
    
    //printf("----------------------------------------------\n");
    
    //printf ("(%d -> %d) Cost: %d\n", startIndex, goalIndex, bestCost);
    //printf("Total nodes explored: %d\n", output->nodesExplored);
    
    //printf("----------------------------------------------\n");
    //printf("Path from start to goal:\n");

    int* path_firstHalf = (int*)malloc(sizeof(int) * grid->size_x * grid->size_y);
    int length_firstHalf = GetPathFromStartToGoal(grid, forward_gScore_h, startIndex, bestMeetIndex, path_firstHalf);

    int* path_secondHalf = (int*)malloc(sizeof(int) * grid->size_x * grid->size_y);
    int length_secondHalf = GetPathFromStartToGoal(grid, backward_gScore_h, goalIndex, bestMeetIndex, path_secondHalf);

    for (int i = 0; i < length_firstHalf; i++) {
        int index = length_firstHalf - 1 - i;
        //printf("Grid_Index: %d\n", path_firstHalf[index]);
        output->path[i] = path_firstHalf[index];
    }

    for (int i = 1; i < length_secondHalf; i++) {
        int index = i;
        //printf("Grid_Index: %d\n", path_secondHalf[index]);
        output->path[length_firstHalf + i - 1] = path_secondHalf[index];
    }

    output->pathSize = length_firstHalf + length_secondHalf - 1; // do not count middle length twice

    free(path_firstHalf);
    free(path_secondHalf);

    //printf("----------------------------------------------\n");
}

void Clean_AStar(int* gScore, int* forward_gScore_d, int* backward_gScore_d, int* gridData_d, int* bucket_sizes_d, 
int* bucket_nodes_d, int* next_bucket_d, int* bestMeetCost_d, int* bestMeetIndex_d) {
    // ----- Host Deallocation -----
    free(gScore);

    // ----- Device Deallocation -----
    cudaFree(forward_gScore_d);
    cudaFree(backward_gScore_d);
    cudaFree(gridData_d);
    cudaFree(bucket_sizes_d);
    cudaFree(bucket_nodes_d);
    cudaFree(next_bucket_d);
    cudaFree(bestMeetCost_d);
    cudaFree(bestMeetIndex_d);
}

void InitGPU_AStar() {
    // Init constant memory
    int offset_xh[4] = {-1, 0, 1, 0};
    int offset_yh[4] = {0, -1, 0, 1};

    cudaMemcpyToSymbol(offset_x, offset_xh, sizeof(int) * 4, 0, cudaMemcpyHostToDevice);
    cudaMemcpyToSymbol(offset_y, offset_yh, sizeof(int) * 4, 0, cudaMemcpyHostToDevice);
}

/*
0  1  2  3
4  5  6  7
8  9  10 11
12 13 14 15
*/

/*
Return: (All GPU versions and CPU must have)
-> Nodes explored ✅
-> Path (start-goal inclusive) ✅
-> Best cost ✅
*/

/*

    int gridSize_x = 4;
    int gridSize_y = 4;

    Grid_2D_Device* myGrid = CreateGrid(gridSize_x, gridSize_y, 0);
    
    myGrid->data[4] = 2;
    myGrid->data[5] = 2;
    myGrid->data[6] = 2;

    AStar_Output* output = InitOutputContainer(gridSize_x * gridSize_y);

    Init_AStar(myGrid, 0, 0, 0, 2, 1, output);

    printf("Total cost: %d, nodes visited: %d, path length: %d\n", output->bestCost, output->nodesExplored, output->pathSize);
    for (int i = 0; i < output->pathSize; i++) {
        printf("Node: %d\n", output->path[i]);
    }

    DestroyGrid(myGrid);
    DestroyOutputContainer(output);
*/