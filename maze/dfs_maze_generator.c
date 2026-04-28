#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#include "../include/dfs_maze_generator.h"
#include "../include/grid.h"

/*
0 -> left
1 -> right
2 -> up
3 -> down
*/
int movement_x[NEIGHBORS] = {-2, 2, 0, 0};
int movement_y[NEIGHBORS] = {0, 0, -2, 2};

/* Arrange the N elements of ARRAY in random order.
   Only effective if N is much smaller than RAND_MAX;
   if this may not be the case, use a better random
   number generator. */
void shuffle(int *array, size_t n)
{
    if (n > 1) 
    {
        size_t i;
        for (i = 0; i < n - 1; i++) 
        {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}

/**
 * @return returns whether or not coordinates are in-bounds
 */
bool ValidNeighbor(Grid_2D_Device* grid, int index_x, int index_y) {
    return (index_x >= 0 && index_x < grid->size_x && index_y >= 0 && index_y < grid->size_y);
}

/**
 * @return returns amount of valid unvisited neighbors
 */
int GetUnvisitedNeighbors(Grid_2D_Device* grid, int index_x, int index_y, int* neighbors) {
    int validNeighbors = 0;

    for (int i = 0; i < NEIGHBORS; ++i) {
        int neighborIndex_x = index_x + movement_x[i];
        int neighborIndex_y = index_y + movement_y[i];
        int neighborIndex = neighborIndex_y * grid->size_x + neighborIndex_x;

        if (!ValidNeighbor(grid, neighborIndex_x, neighborIndex_y)) continue;
        if (grid->data[neighborIndex] == 0) continue;
        
        neighbors[validNeighbors] = neighborIndex_y * grid->size_x + neighborIndex_x;
        validNeighbors++;
    }

    return validNeighbors;
}

void GenerateMaze_Step(Grid_2D_Device* grid, int index_x, int index_y) {
    // Set index as visited
    int index = index_y * grid->size_x + index_x;
    grid->data[index] = 0;

    int* neighbors;
    int neighborCount;

    while(true) {
        // Get updated list of neighbors
        neighbors = (int*)malloc(sizeof(int)*NEIGHBORS);
        neighborCount = GetUnvisitedNeighbors(grid, index_x, index_y, neighbors);

        // Break if no unvisited neighbors left
        if (neighborCount == 0) break;
        
        // Shuffle neighbors to randomly choose
        shuffle(neighbors, neighborCount);

        // Neighbor index components
        int neighborIndex = neighbors[0];
        int neighborIndex_y = neighborIndex / grid->size_x;
        int neighborIndex_x = neighborIndex % grid->size_x;

        // Wall index components
        int wallIndex_x = (neighborIndex_x - index_x) / 2 + index_x;
        int wallIndex_y = (neighborIndex_y - index_y) / 2 + index_y;
        int wallIndex = wallIndex_y * grid->size_x + wallIndex_x;

        // Break wall
        grid->data[wallIndex] = 0;

        // Recursive step
        GenerateMaze_Step(grid, neighborIndex_x, neighborIndex_y);
    }

    free(neighbors);
}

Grid_2D_Device* GenerateMaze(int size_x, int size_y, int startIndex_x, int startIndex_y, int goalIndex_x, int goalIndex_y) {
    // goal unused for now.

    Grid_2D_Device* grid = CreateGrid(size_x, size_y, 2);
    
    GenerateMaze_Step(grid, startIndex_x, startIndex_y);

    return grid;
}