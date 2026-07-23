#ifndef HTTP_UTIL_H
#define HTTP_UTIL_H

#include <curl/curl.h>
#include <stddef.h>

typedef struct {
    char *data;
    size_t size;
} ResponseBuffer;

size_t discard_callback(char *buffer, size_t itemsize, size_t nitems, void *userdata);
size_t accumulate_callback(char *buffer, size_t itemsize, size_t nitems, void *userdata);
void apply_common_opts(CURL *curl);   // FOLLOWLOCATION + USERAGENT + TIMEOUT

#endif