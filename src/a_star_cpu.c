#include<limits.h>
#include<stdio.h>
#include<stdlib.h>
#include "../include/grid.h"
#include "../include/bmh.h"
#include "../include/a_star_cpu.h"

/*
Need for visuals:
- List - all explored nodes in order?
    -> Another list for when nodes are added to OpenList?
*/

/*
Grid Layout: (2D)
Values:
-> 0.0 indicates free-to-move slot
-> 1.0 indicates explored
-> 2.0 indicates cannot move through
-> Starting/ending nodes given as index params
*/

/*
0 = left
1 = top
2 = right
3 = bottom
*/
int* GetNeighborIDs(Grid_2D* grid, int currentId) {
    int size_x = grid->size_x;
    int size_y = grid->size_y;
    int currentId_x = currentId % size_x;
    int currentId_y = currentId / size_x;

    int* idList = (int*)malloc(sizeof(int) * 4);

    int offset_x[4] = {-1, 0, 1, 0};
    int offset_y[4] = {0, -1, 0, 1};

    for (int i = 0; i <= 3; i++)
    {
        if ((i == 0 && currentId_x == 0) 
        || (i == 1 && currentId_y == 0) 
        || (i == 2 && currentId_x == size_x - 1) 
        || (i == 3 && currentId_y == size_y - 1)) 
        {
            idList[i] = INT_MAX;
        } else 
        {
            idList[i] = (currentId_y + offset_y[i]) * size_x + (currentId_x + offset_x[i]);
        }
    }

    return idList;
}

int CheckIfOutOfBounds(int neighborId) {
    return neighborId == INT_MAX;
}

int ManhattanDistance(int col1, int row1, int col2, int row2) {
    return abs(col1-col2) + abs(row1-row2);
}

void RunAStar(Grid_2D* grid, int startIndex_x, int startIndex_y, int goalIndex_x, int goalIndex_y) {
    // Variables
    int totalNodesSearched = 0;

    int gridSize_x = grid->size_x;
    int gridSize_y = grid->size_y;
    int gridSize = gridSize_x * gridSize_y;

    int startIndex = startIndex_y * gridSize_x + startIndex_x;
    int goalIndex = goalIndex_y * gridSize_x + goalIndex_x;

    Grid_Node** grid_ptr = grid->grid_ptr;

    // openSet - list used to choose next least expensive point
    BinaryMinHeap* openSet = Init_BMH(gridSize);
    Insert_BMH(openSet, startIndex, ManhattanDistance(startIndex_x, startIndex_y, goalIndex_x, goalIndex_y)); // O(log n)

    // Current known best score from start to n
    int* gScore = (int*)malloc(sizeof(int) * gridSize);

    // Best guess of cost from start to finish when going through n : fScore[n] = gScore[n] + h(n)
    //float* fScore = (float*)malloc(sizeof(float) * gridSize);
    
    for (int i = 0; i < gridSize; i++) 
    {
        gScore[i] = INT_MAX;
        //fScore[i] = FLT_MAX;
    }

    gScore[startIndex] = 0;
    //fScore[startIndex] = ManhattanDistance(startIndex_x, startIndex_y, goalIndex_x, goalIndex_y);

    // Main loop - chooses next nearest reachable node
    while (openSet->size != 0) {
        int currentId = PeekMin_BMH(openSet)->id; // O(1)
        
        totalNodesSearched++;

        if (currentId == goalIndex) {
            printf("goal (%d, %d) distance needed: %d\n", goalIndex_x, goalIndex_y, gScore[currentId]);
            printf("total searched: %d\n", totalNodesSearched);
            Destroy_BMH(openSet);
            free(gScore);
            return;
        }

        RemoveMin_BMH(openSet); // O(1) for head removal
        grid_ptr[currentId]->data = 1.0;

        int* neighbors = GetNeighborIDs(grid, currentId);
        for (int i = 0; i <= 3; i++) {
            int neighborId = neighbors[i];
            printf("neigbor: %d\n", neighborId);

            if (CheckIfOutOfBounds(neighborId) == 1) continue;
            if (grid_ptr[neighborId]->data==2.0) continue;
            if (grid_ptr[neighborId]->data==1.0) continue; // Must be removed to get optimal path if heuristic function is not consistent

            int tentative_gScore = gScore[currentId] + 1;
            if (tentative_gScore < gScore[neighborId]) {
                grid_ptr[neighborId]->parent_index = currentId;
                gScore[neighborId] = tentative_gScore;
                
                int heuristicVal = ManhattanDistance(neighborId % grid->size_x, neighborId / grid->size_x, goalIndex_x, goalIndex_y);
                
                if (GetElementById_BMH(openSet, neighborId)) // O(n)
                {
                    UpdateWeight_BMH(openSet, neighborId, tentative_gScore + heuristicVal); // O(log n)
                } else
                {
                    Insert_BMH(openSet, neighborId, tentative_gScore + heuristicVal); // O(log n)
                }
            }
        }
        free(neighbors);
    }
    Destroy_BMH(openSet);
    free(gScore);
    fprintf(stderr, "Error<RunAStar>: Could not reach goal!\n");
}