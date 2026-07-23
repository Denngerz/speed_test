#ifndef SERVER_LIST_H
#define SERVER_LIST_H

typedef struct {
    char country[64];
    char city[64];
    char provider[64];
    char host[128];
    int id;
} ServerInfo;

ServerInfo* load_servers_info(const char *filepath, int *out_count);

#endif