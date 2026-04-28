#include<stdio.h>
#include<stdlib.h>

#include "../include/correctness.h"

int main(int argc, char* argv[]) {
    testCorrectness();
    return 0;
}

    /*
    //grid test
    int* data_ptr = (int*)malloc(sizeof(int));
    *data_ptr = 0;

    //Grid_2D* grid = CreateGrid(5, 5, data_ptr);
    *data_ptr = 0;
    //DestroyGrid(grid);

    //pq test
    PriorityQueue* pq = Init_PQ();

    Insert_PQ(pq, 0, 5);
    Insert_PQ(pq, 1, 2);
    Insert_PQ(pq, 2, 7);
    Insert_PQ(pq, 3, 0);
    Insert_PQ(pq, 4, 0);

    Node_PQ* current = pq->head;
    while(current != NULL) {
        printf("id: %d, weight: %d\n", current->id, current->weight);
        current = current->next;
    }

    Destroy_PQ(pq);
    */

    /*
    // bmh.c test
    BinaryMinHeap* heap = Init_BMH(7);

    Insert_BMH(heap, 0, 0);
    Insert_BMH(heap, 1, 1);
    Insert_BMH(heap, 2, 2);
    Insert_BMH(heap, 3, 4);
    Insert_BMH(heap, 4, 3);

    RemoveMin_BMH(heap);
    RemoveMin_BMH(heap);
    RemoveMin_BMH(heap);
    RemoveMin_BMH(heap);
    RemoveMin_BMH(heap);
    
    for (int i = 0; i < heap->size; i++)
    {
        printf("Index: %d, value: %d\n", i, heap->arr[i]->weight);
    }

    Destroy_BMH(heap);

    free(data_ptr);

    // NDGrid Testing
    EmptyGrid_Test(4, 3);
    */

/*
    printf("---------------A* Testing---------------\n");
    Grid_2D_Device* grid3 = CreateGrid(4, 4, 0);
    AStar_Output* output = InitOutputContainer(4 * 4);

    grid3->data[4] = 2;
    grid3->data[5] = 2;
    grid3->data[6] = 2;

    RunAStar(grid3, 0, 0, 0, 2, output);
    
    printf("Best cost: %d, nodes explored: %d, path length: %d\n", output->bestCost, output->nodesExplored, output->pathSize);

    if (output->validPath) {
        for (int i = 0; i < output->pathSize; i++) {
            printf("Node: %d\n", output->path[i]);
        }
    }

    DestroyGrid(grid3);
    DestroyOutputContainer(output);

    printf("---------------Maze Generator Testing---------------\n");
    int* seed = (int*)malloc(sizeof(int));
    *seed = 4;

    Grid_2D_Device* grid4 = GenerateMaze(10, 10, 0, 0, 4, 2, seed);
    PrintGrid(grid4);
    
    free(seed);
*/