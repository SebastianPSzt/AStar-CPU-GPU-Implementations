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

    // a_star_cpu test
    printf("---------------A* Testing---------------\n");
    Grid_2D* grid2 = CreateGrid(1024, 1024, data_ptr);
    grid2->grid_ptr[4]->data = 2.0;
    grid2->grid_ptr[5]->data = 2.0;
    grid2->grid_ptr[6]->data = 2.0;

    RunAStar(grid2, 0, 0, 1023, 1023);

    DestroyGrid(grid2);

    free(data_ptr);

    return 0;
}