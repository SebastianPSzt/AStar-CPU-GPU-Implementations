#ifndef HEURISTICS_H
#define HEURISTICS_H

// Functions
int manhattanDistance(int x1, int y1, int x2, int y2);
int euclideanDistance(int x1, int y1, int x2, int y2);
int chebyshevDistance(int x1, int y1, int x2, int y2);
int manhattanDistance_General(int* p1, int* p2, int dimension);
#endif