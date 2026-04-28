#ifndef A_STAR_OUTPUT_H
#define A_STAR_OUTPUT_H

typedef struct AStar_Output {
    int validPath;
    int* path;
    int pathSize;
    int bestCost;
    int nodesExplored;
} AStar_Output;

AStar_Output* InitOutputContainer(int pathSize);
void DestroyOutputContainer(AStar_Output* container);

#endif