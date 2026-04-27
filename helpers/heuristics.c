#include <math.h>
#include <stdlib.h>
#include "../include/heuristics.h"

// 2D Heuristic Functions

// Non-diagonal Movement
int manhattanDistance(int x1, int y1, int x2, int y2) {
    return abs(x2 - x1) + abs(y2 - y1);
}

// Straight-line Distance (any movement)
int euclideanDistance(int x1, int y1, int x2, int y2) {
    return 0;
}

// Minimum tiles to traverse with diagonal movement
int chebyshevDistance(int x1, int y1, int x2, int y2) {
    return 0;
}

// General Heuristic Functions

// Manhattan Distance
int manhattanDistance_General(int* p1, int* p2, int dimension) {
    int result = 0;
    for (int i = 0; i < dimension; i++) {
        result += abs(p2[i] - p1[i]);
    }
    return result;
}