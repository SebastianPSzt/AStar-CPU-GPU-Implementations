#ifndef PQ_H
#define PQ_H

// Structs
typedef struct Node_PQ {
    struct Node_PQ* next;
    int id;
    int weight;
} Node_PQ;

typedef struct PriorityQueue {
    Node_PQ* head;
    int size;
} PriorityQueue;

// Functions
PriorityQueue* Init_PQ();
void Insert_PQ(PriorityQueue* pq, int id, int weight);
Node_PQ* Peek_PQ(PriorityQueue* pq);
Node_PQ* GetElementById_PQ(PriorityQueue* pq, int id);
void Remove_PQ(PriorityQueue* pq, int id);
void Destroy_PQ(PriorityQueue* pq);

#endif