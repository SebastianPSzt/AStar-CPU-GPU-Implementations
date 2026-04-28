#include <stdio.h>
#include <stdlib.h>
#include "../include/pq.h"

PriorityQueue* Init_PQ() {
    PriorityQueue* pq = (PriorityQueue*) malloc(sizeof(PriorityQueue));
    pq->size = 0;
    pq->head = NULL;
    return pq;
}

void Insert_PQ(PriorityQueue* pq, int id, int weight) {
    Node_PQ* node = (Node_PQ*) malloc(sizeof(Node_PQ));
    node->id = id;
    node->weight = weight;

    // Case 1 - Empty Queue
    if (pq->head == NULL) 
    {
        pq->head = node;
        node->next = NULL;
        pq->size++;
        return;
    }

    // Case 2 - Beginning Insert
    if (node->weight < pq->head->weight) 
    {
        node->next = pq->head;
        pq->head = node;
        pq->size++;
        return;
    }

    // Case 3 - Non-edge Insert
    Node_PQ* currentNode = pq->head;
    while (currentNode->next != NULL) 
    {
        if (node->weight < currentNode->next->weight) 
        {
            node->next = currentNode->next;
            currentNode->next = node;
            pq->size++;
            return;
        }
        currentNode = currentNode->next;
    }

    // Case 4 - End Insert
    currentNode->next = node;
    node->next = NULL;
    pq->size++;
    return;
}

Node_PQ* Peek_PQ(PriorityQueue* pq) {
    return pq->head;
}

Node_PQ* GetElementById_PQ(PriorityQueue* pq, int id) {
    Node_PQ* currentNode = pq->head;
    while (currentNode != NULL) 
    {
        if (currentNode->id == id) return currentNode;
        currentNode = currentNode->next;
    }

    return NULL;
}

void Remove_PQ(PriorityQueue* pq, int id) {
    // Case 1 - First node
    if (pq->head->id == id) 
    {
        Node_PQ* n = pq->head->next;;
        free(pq->head);
        pq->head = n;
        pq->size--;
        return;
    }

    // Case 2 - Any other node
    Node_PQ* currentNode = pq->head;
    while (currentNode->next != NULL) 
    {
        if (currentNode->next->id == id) 
        {
            Node_PQ* n = currentNode->next->next;
            free(currentNode->next);
            currentNode->next = n;
            pq->size--;
            return;
        }
        currentNode = currentNode->next;
    }
}

void Destroy_PQ(PriorityQueue* pq) {
    Node_PQ* currentNode = pq->head;
    while (currentNode != NULL) 
    {
        Node_PQ* n = currentNode->next;
        free(currentNode);
        currentNode = n;
    }
    free(pq);
}