#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include "../include/grid.h"
#include "../include/pq.h"
#include "../include/bmh.h"
#include "../include/a_star_cpu.h"

int main(int argc, char* argv[]) {
    //grid test
    float* data_ptr = (float*)malloc(sizeof(int));
    *data_ptr = 0.0f;

    Grid_2D* grid = CreateGrid(5, 5, data_ptr);
    *data_ptr = 0.0f;
    PrintGridFloat(grid);
    DestroyGrid(grid);

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

    // a_star_cpu test
    printf("---------------A* Testing---------------\n");
    Grid_2D* grid2 = CreateGrid(1024, 1024, data_ptr);
    grid2->grid_ptr[4]->data = 2.0;
    grid2->grid_ptr[5]->data = 2.0;
    grid2->grid_ptr[6]->data = 2.0;

    time_t prev = time(NULL);
    RunAStar(grid2, 0, 0, 1023, 1023);
    time_t post = time(NULL);
    printf("time: %ld\n", post - prev);

    DestroyGrid(grid2);

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

    return 0;
}