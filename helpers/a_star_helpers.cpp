#include "grid.h"

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