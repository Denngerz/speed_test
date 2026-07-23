#ifndef SERVER_SELECT_H
#define SERVER_SELECT_H

#include "server_list.h"

ServerInfo* find_server_by_country(ServerInfo *servers, int count, const char *country);

#endif