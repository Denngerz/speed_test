#include "../include/http_utils.h"
#include <stdlib.h>
#include <string.h>

size_t discard_callback(char *buffer, size_t itemsize, size_t nitems, void *userdata){
    return itemsize * nitems;
}

size_t accumulate_callback(char *buffer, size_t itemsize, size_t nitems, void *userdata){
    size_t total_size = itemsize * nitems;
    ResponseBuffer *resp = (ResponseBuffer *)userdata;

    char *new_data = realloc(resp->data, resp->size + total_size + 1);
    if (new_data == NULL) {
        return 0;
    }
    resp->data = new_data;

    memcpy(resp->data + resp->size, buffer, total_size);
    resp->size += total_size;
    resp->data[resp->size] = '\0';

    return total_size;
}

void apply_common_opts(CURL *curl){
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/8.5.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
}