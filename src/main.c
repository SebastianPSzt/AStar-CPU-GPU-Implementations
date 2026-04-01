#include<stdio.h>
#include<stdlib.h>
#include "../include/grid.h"
#include "../include/pq.h"
#include "../include/a_star_cpu.h"

int main(int argc, char* argv[]) {
    //grid test
    float* data_ptr = (float*) malloc(sizeof(int));
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
    Grid_2D* grid2 = CreateGrid(4, 4, data_ptr);
    grid2->grid_ptr[4]->data = 2.0;
    grid2->grid_ptr[5]->data = 2.0;
    grid2->grid_ptr[6]->data = 2.0;

    RunAStar(grid2, 0, 0, 0, 2);

    DestroyGrid(grid2);

    return 0;
}