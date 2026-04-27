#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/grid.h"
#include "../include/cJSON.h"

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

Grid_2D *CreateGrid(int size_x, int size_y, float *default_val)
{
    Grid_2D *grid = (Grid_2D *)malloc(sizeof(Grid_2D));

    grid->size_x = size_x;
    grid->size_y = size_y;

    grid->grid_ptr = (Grid_Node **)malloc(sizeof(Grid_Node *) * size_x * size_y);

    for (int row = 0; row < size_y; row++)
    {
        for (int col = 0; col < size_x; col++)
        {
            size_t index = row * size_x + col;

            Grid_Node *node = (Grid_Node *)malloc(sizeof(Grid_Node));
            node->data = *default_val;
            node->parent_index = -1;

            grid->grid_ptr[index] = node;
        }
    }

    return grid;
}

Grid_2D *GatherGrid(const char *path)
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
    cJSON *grid_json = cJSON_GetObjectItemCaseSensitive(root, "grid");
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
    float default_val = 0.0f;
    float blocked_val = 2.0f;

    Grid_2D *grid = CreateGrid(size_x, size_y, &default_val);
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
                UpdateGridByIndex(grid, row, col, &blocked_val);
            }
        }
    }

    cJSON_Delete(root);
    return grid;
}

void PrintGridFloat(Grid_2D *grid)
{
    for (int row = 0; row < grid->size_y; row++)
    {
        for (int col = 0; col < grid->size_x; col++)
        {
            size_t index = row * grid->size_x + col;
            printf("%f ", grid->grid_ptr[index]->data);
        }
        printf("\n");
    }
}

void UpdateGridByIndex(Grid_2D *grid, int row, int col, float *val)
{
    grid->grid_ptr[row * grid->size_x + col]->data = *val;
}

void DestroyGrid(Grid_2D *grid)
{

    for (int row = 0; row < grid->size_y; row++)
    {
        for (int col = 0; col < grid->size_x; col++)
        {
            size_t index = row * grid->size_x + col;
            free(grid->grid_ptr[index]);
        }
    }

    free(grid->grid_ptr);
    free(grid);
}