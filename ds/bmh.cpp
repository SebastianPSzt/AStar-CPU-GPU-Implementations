#include<stdio.h>
#include<stdlib.h>
#include "../include/bmh.h"

void HeapifyUp(BinaryMinHeap* bmh, int index) {
    // Empty Case
    if (index == 0) return;

    int parentIndex = (index-1)/2;
    Node_BMH* childNode = bmh->arr[index];
    Node_BMH* parentNode = bmh->arr[parentIndex];

    // Parent <= Child Case
    if (parentNode->weight <= childNode->weight) return;

    // Child < Parent Case
    bmh->arr[parentIndex] = childNode;
    bmh->arr[index] = parentNode;
    HeapifyUp(bmh, parentIndex);
}

void HeapifyDown(BinaryMinHeap* bmh, int index) {
    if (index >= bmh->size) return;

    Node_BMH* node = bmh->arr[index];

    int leftChildIndex = 2 * index + 1;
    int rightChildIndex = 2 * index + 2;

    if (leftChildIndex >= bmh->size) return;

    int minChildIndex = leftChildIndex;

    if (rightChildIndex < bmh->size &&
        bmh->arr[rightChildIndex]->weight < bmh->arr[leftChildIndex]->weight) {
        minChildIndex = rightChildIndex;
    }

    if (bmh->arr[minChildIndex]->weight >= node->weight) return;

    bmh->arr[index] = bmh->arr[minChildIndex];
    bmh->arr[minChildIndex] = node;

    HeapifyDown(bmh, minChildIndex);
}

BinaryMinHeap* Init_BMH(int max_size) {
    BinaryMinHeap* bmh = (BinaryMinHeap*)malloc(sizeof(BinaryMinHeap));
    bmh->arr = (Node_BMH**) malloc(sizeof(Node_BMH*) * max_size);
    bmh->size = 0;
    bmh->maxSize = max_size;
    return bmh;
}

void Insert_BMH(BinaryMinHeap* bmh, int id, int weight) {
    Node_BMH* newNode = (Node_BMH*)malloc(sizeof(Node_BMH));
    newNode->id = id;
    newNode->weight = weight;
    
    bmh->arr[bmh->size] = newNode;
    HeapifyUp(bmh, bmh->size);
    bmh->size++;
}

Node_BMH* PeekMin_BMH(BinaryMinHeap* bmh) {
    return bmh->arr[0];
}

void RemoveMin_BMH(BinaryMinHeap* bmh) {
    // Empty Case
    if (bmh->size == 0) return;
    
    // Single Node Case
    if (bmh->size == 1)
    {
        free(bmh->arr[0]);
        bmh->arr[0] = NULL;
        bmh->size--;
        return;
    }

    // General Case
    Node_BMH* minNode = bmh->arr[0];
    Node_BMH* lastNode = bmh->arr[bmh->size - 1];
    
    free(minNode);
    bmh->arr[0] = lastNode;
    bmh->arr[bmh->size - 1] = NULL;
    bmh->size--;
    HeapifyDown(bmh, 0);
}

void UpdateWeight_BMH(BinaryMinHeap* bmh, int id, int weight) {
    int index = -1;

    for (int i = 0; i < bmh->size; i++) {
        if (bmh->arr[i]->id == id) {
            index = i;
            break;
        }
    }

    if (index == -1) return;

    int oldWeight = bmh->arr[index]->weight;
    bmh->arr[index]->weight = weight;

    if (weight < oldWeight)
        HeapifyUp(bmh, index);
    else
        HeapifyDown(bmh, index);
}

Node_BMH* GetElementById_BMH(BinaryMinHeap* bmh, int id) {
    for (int i = 0; i < bmh->size; i++)
    {
        Node_BMH* node = bmh->arr[i];
        if (node->id == id)
        {
            return node;
        }
    }
    return NULL;
}

void Destroy_BMH(BinaryMinHeap* bmh) {
    for (int i = 0; i < bmh->size; i++)
        free(bmh->arr[i]);
    
    free(bmh->arr);
    free(bmh);
}