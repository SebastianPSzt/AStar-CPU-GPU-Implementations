#ifndef A_STAR_OUTPUT_H
#define A_STAR_OUTPUT_H

typedef struct Output_Node {
    int* ids_explored;
    int num_explored;
    Output_Node *next = nullptr;
} Output_Node;

typedef struct AStar_Output {
    int validPath;
    int* path;
    int pathSize;
    int bestCost;
    int nodesExplored;
    Output_Node* history;
} AStar_Output;

AStar_Output* InitOutputContainer(int pathSize);
void DestroyOutputContainer(AStar_Output* container);
void ReverseHistory(AStar_Output* container);
void PrintOutputHistory(AStar_Output* output, const char* filename);

#endif