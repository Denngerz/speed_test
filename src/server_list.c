#include "../include/server_list.h"
#include <stdio.h>
#include <cjson/cJSON.h>
#include <stdlib.h>
#include <string.h>

ServerInfo* load_servers_info(const char *filepath, int *out_count){
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file.\n");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, file_size, file);
    buffer[bytesRead] = '\0';
    fclose(file);

    cJSON *json = cJSON_Parse(buffer);
    if (json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }

        free(buffer);
        return NULL;
    }

    int array_size = cJSON_GetArraySize(json);

    *out_count = array_size;

    ServerInfo *list = malloc(sizeof(ServerInfo) * array_size);

    if (list == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        cJSON_Delete(json);
        free(buffer);
        return NULL;
    }

    for (int i = 0; i < array_size; i++) {
        cJSON *item = cJSON_GetArrayItem(json, i);

        cJSON *country = cJSON_GetObjectItemCaseSensitive(item, "country");
        cJSON *city    = cJSON_GetObjectItemCaseSensitive(item, "city");
        cJSON *provider = cJSON_GetObjectItemCaseSensitive(item, "provider");
        cJSON *host = cJSON_GetObjectItemCaseSensitive(item, "host");
        cJSON *id = cJSON_GetObjectItemCaseSensitive(item, "id");

        if (!cJSON_IsString(country) || !cJSON_IsString(city) || !cJSON_IsString(provider) || !cJSON_IsString(host) || !cJSON_IsNumber(id)) {
            fprintf(stderr, "Skipping malformed server entry at index %d\n", i);
            continue;
        }

        strncpy(list[i].country, country->valuestring, sizeof(list[i].country) - 1);
        list[i].country[sizeof(list[i].country) - 1] = '\0';

        strncpy(list[i].city, city->valuestring, sizeof(list[i].city) - 1);
        list[i].city[sizeof(list[i].city) - 1] = '\0';

        strncpy(list[i].provider, provider->valuestring, sizeof(list[i].provider) - 1);
        list[i].provider[sizeof(list[i].provider) - 1] = '\0';

        strncpy(list[i].host, host->valuestring, sizeof(list[i].host) - 1);
        list[i].host[sizeof(list[i].host) - 1] = '\0';

        list[i].id = id->valueint;
    }

    cJSON_Delete(json);
    free(buffer);

    return list;
}