#include <stdio.h>
#include <stdlib.h>

#include "../include/a_star_output.h"

AStar_Output* InitOutputContainer(int pathSize) {
    AStar_Output* container = (AStar_Output*)malloc(sizeof(AStar_Output));
    
    container->validPath = 0;
    container->path = (int*)malloc(sizeof(int) * pathSize);
    container->pathSize = 0;
    container->nodesExplored = 0;
    container->bestCost = -1;

    return container;
}

void DestroyOutputContainer(AStar_Output* container) {
    free (container->path);
    free(container);
}