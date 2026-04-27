#include <stdio.h>
#include <stdlib.h>
#include "../include/search_log_cpu.h"

SearchLogNode *LogPushFront(SearchLogNode *head, int neighbor_id)
{
    SearchLogNode *node = malloc(sizeof(SearchLogNode));
    if (!node)
    {
        fprintf(stderr, "Error: malloc failed in LogPushFront\n");
        exit(1);
    }

    node->neighbor_id = neighbor_id;
    node->next = head;

    return node;
}

SearchLogNode *ReverseLog(SearchLogNode *head)
{
    SearchLogNode *prev = NULL;
    SearchLogNode *curr = head;

    while (curr != NULL)
    {
        SearchLogNode *next = curr->next;
        curr->next = prev;
        prev = curr;
        curr = next;
    }

    return prev;
}

void FreeSearchLog(SearchLogNode *head)
{
    while (head != NULL)
    {
        SearchLogNode *next = head->next;
        free(head);
        head = next;
    }
}

void PrintSearchLog(SearchLogNode *head, const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        perror("Failed to open file");
        return;
    }

    for (SearchLogNode *curr = head; curr != NULL; curr = curr->next)
    {
        fprintf(file, "%d\n", curr->neighbor_id);
    }

    fclose(file);
}