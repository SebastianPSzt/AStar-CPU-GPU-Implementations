#ifndef BMH_H
#define BMH_H

// Structs
typedef struct Node_BMH {
    int id;
    int weight;
} Node_BMH;

typedef struct BinaryMinHeap {
    Node_BMH** arr;
    int size;
    int maxSize;
} BinaryMinHeap;

// Functions
BinaryMinHeap* Init_BMH(int max_size);
void Insert_BMH(BinaryMinHeap* bmh, int id, int weight);
Node_BMH* PeekMin_BMH(BinaryMinHeap* bmh);
void RemoveMin_BMH(BinaryMinHeap* bmh);
void UpdateWeight_BMH(BinaryMinHeap* bmh, int id, int weight);
Node_BMH* GetElementById_BMH(BinaryMinHeap* bmh, int id);
void Destroy_BMH(BinaryMinHeap* bmh);

#endif
