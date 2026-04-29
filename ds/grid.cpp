#include<stdio.h>
#include<stdlib.h>

#include "grid.h"
#include "cJSON.h"

Grid_2D_Device* CreateGrid(int size_x, int size_y, int default_val) {
    int totalSize = size_x * size_y;

    Grid_2D_Device* grid = (Grid_2D_Device*) malloc(sizeof(Grid_2D_Device));

    grid->size_x = size_x;
    grid->size_y = size_y;

    grid->data = (int*)malloc(sizeof(int) * totalSize);
    grid->parent = (int*)malloc(sizeof(int)* totalSize);

    for (int i = 0; i < totalSize; i++)
    {
        grid->data[i] = default_val;
        grid->parent[i] = -1;
    }

    return grid;
}

void PrintGrid(Grid_2D_Device* grid) {
    for (int row = 0; row < grid->size_y; row++) 
    {
        for (int col = 0; col < grid->size_x; col++) 
        {
            size_t index = row * grid->size_x + col;
            //int data = grid->data[index];
            //if (data == 0) {
                //printf("\U00002B1C");
            //} else {
                //printf("\U0001F7E5");
            //}
            printf("%d ", (int)(grid->data[index]));
        }
        printf("\n");
    }
}

void UpdateGridByIndex(Grid_2D_Device* grid, int row, int col, int val) {
    grid->data[row * grid->size_x + col] = val;
}

void DestroyGrid(Grid_2D_Device* grid) {
    free(grid->data);
    free(grid->parent);
    free(grid);
}

Grid_ND* CreateNDGrid(int* sizes, int dimension, int default_val) {
    // Initialize grid
    Grid_ND* grid = (Grid_ND*)malloc(sizeof(Grid_ND));

    grid->sizes = (int*)malloc(sizeof(int) * dimension);
    grid->dimension = dimension;
    grid->totalSize = 1;

    for (int i = 0; i < dimension; i++)
    {
        int size_i = sizes[i];
        grid->sizes[i] = size_i;
        grid->totalSize *= size_i;
    }

    grid->data = (int*)malloc(sizeof(int) * grid->totalSize);
    grid->parent = (int*)malloc(sizeof(int) * grid->totalSize);    

    int totalSize = grid->totalSize;
    for (int i = 0; i < totalSize; i++)
    {
        grid->data[i] = default_val;
        grid->parent[i] = -1;
    }

    return grid;
}

void UpdateNDGridByIndex(Grid_ND* grid, int* indices, int val) {
    // y * length_x + x
    // z * length_y * length_x + y * length_x + x
    // For 1..D, nth dimension is scaled by length_(N+1..D)

    int dim = grid->dimension;
    int index = 0;

    for (int i = 0; i < dim; i++)
    {
        int index_i = indices[i];
        for (int j = i+1; j < dim; j++)
        {
            index_i *= grid->sizes[j];
        }
        index += index_i;
    }

    grid->data[index] = val;
}

void DestroyNDGrid(Grid_ND* grid) {
    free(grid->data);
    free(grid->parent);
    free(grid->sizes);
    free(grid);
}

static char *read_file_as_string(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    if (length < 0)
    {
        fclose(file);
        return NULL;
    }

    char *buffer = (char *)malloc((size_t)length + 1);
    if (!buffer)
    {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, (size_t)length, file);
    fclose(file);

    if (bytes_read != (size_t)length)
    {
        free(buffer);
        return NULL;
    }

    buffer[length] = '\0';
    return buffer;
}

Grid_2D_Device *GatherGrid(const char *path)
{
    char *json_text = read_file_as_string(path);
    if (!json_text)
    {
        printf("Failed to read file: %s\n", path);
        return NULL;
    }

    cJSON *root = cJSON_Parse(json_text);
    free(json_text);

    if (!root)
    {
        printf("Failed to parse JSON\n");
        return NULL;
    }

    cJSON *width_json = cJSON_GetObjectItemCaseSensitive(root, "width");
    cJSON *height_json = cJSON_GetObjectItemCaseSensitive(root, "height");
    cJSON *grid_json = cJSON_GetObjectItemCaseSensitive(root, "_grid");
    cJSON *threshold_json = cJSON_GetObjectItemCaseSensitive(root, "threshold");

    if (!cJSON_IsNumber(width_json) ||
        !cJSON_IsNumber(height_json) ||
        !cJSON_IsNumber(threshold_json) ||
        !cJSON_IsArray(grid_json))
    {
        printf("JSON missing valid size_x, size_y, or cells\n");
        cJSON_Delete(root);
        return NULL;
    }

    int size_x = width_json->valueint;
    int size_y = height_json->valueint;
    double threshold = threshold_json->valuedouble;
    int default_val = 0;
    int blocked_val = 2;

    Grid_2D_Device *grid = CreateGrid(size_x, size_y, default_val);
    if (!grid)
    {
        cJSON_Delete(root);
        return NULL;
    }

    for (int row = 0; row < size_y; row++)
    {
        cJSON *row_json = cJSON_GetArrayItem(grid_json, row);
        if (!cJSON_IsArray(row_json))
        {
            printf("Invalid row %d in cells\n", row);
            DestroyGrid(grid);
            cJSON_Delete(root);
            return NULL;
        }

        for (int col = 0; col < size_x; col++)
        {
            cJSON *cell_json = cJSON_GetArrayItem(row_json, col);
            if (!cell_json)
            {
                printf("Missing cell at row %d col %d\n", row, col);
                DestroyGrid(grid);
                cJSON_Delete(root);
                return NULL;
            }

            double val = cell_json->valuedouble;

            if (val > threshold)
            {
                UpdateGridByIndex(grid, row, col, blocked_val);
            }
        }
    }

    cJSON_Delete(root);
    return grid;
}