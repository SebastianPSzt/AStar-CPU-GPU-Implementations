#include <stdio.h>
#include <stdlib.h>

#include "../include/a_star_output.h"

AStar_Output *InitOutputContainer(int pathSize)
{
    AStar_Output *container = (AStar_Output *)malloc(sizeof(AStar_Output));

    container->validPath = 0;
    container->path = (int *)malloc(sizeof(int) * pathSize);
    container->pathSize = 0;
    container->nodesExplored = 0;
    container->bestCost = -1;
    container->history = nullptr;

    return container;
}

void DestroyOutputContainer(AStar_Output *container)
{
    Output_Node *curr = container->history;
    while (curr != nullptr)
    {
        Output_Node *next = curr->next;
        delete[] curr->ids_explored;
        delete curr;
        curr = next;
    }

    free(container->path);
    free(container);
}

void ReverseHistory(AStar_Output *container)
{
    Output_Node *prev = nullptr;
    Output_Node *curr = container->history;
    Output_Node *next = nullptr;

    while (curr != nullptr)
    {
        next = curr->next;
        curr->next = prev;
        prev = curr;
        curr = next;
    }

    container->history = prev;
}

void PrintOutputHistory(AStar_Output *output, const char *filename)
{
    if (output == nullptr)
        return;

    FILE *file = fopen(filename, "w");
    if (!file)
    {
        perror("Failed to open file");
        return;
    }

    for (Output_Node *curr = output->history; curr != nullptr; curr = curr->next)
    {
        for (int i = 0; i < curr->num_explored; i++)
        {
            fprintf(file, "%d", curr->ids_explored[i]);

            if (i < curr->num_explored - 1)
            {
                fprintf(file, ",");
            }
        }

        fprintf(file, "\n");
    }

    fclose(file);
}