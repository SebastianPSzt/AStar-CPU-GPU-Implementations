#ifndef SEARCH_LOG_CPU_H
#define SEARCH_LOG_CPU_H

typedef struct SearchLogNode
{
    int neighbor_id;
    struct SearchLogNode *next;
} SearchLogNode;

SearchLogNode *LogPushFront(SearchLogNode *head, int neighbor_id);
SearchLogNode *ReverseLog(SearchLogNode *head);
void FreeSearchLog(SearchLogNode *head);
void PrintSearchLog(SearchLogNode *head, char *path);

#endif
