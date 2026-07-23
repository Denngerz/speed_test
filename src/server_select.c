#include "../include/server_select.h"
#include <curl/curl.h>
#include <stdio.h>
#include <string.h>

ServerInfo* find_server_by_country(ServerInfo *servers, int count, const char *country) {
    ServerInfo *best_server = NULL;
    double best_pure_time = 0.0;

    for (int i = 0; i < count; i++) {
        if (strcmp(servers[i].country, country) == 0) {
            CURL *curl = curl_easy_init();
            if (!curl) {
                continue;
            }

            char url[256];
            snprintf(url, sizeof(url), "http://%s", servers[i].host);

            CURLcode result;
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3L);

            result = curl_easy_perform(curl);

            if(result == CURLE_OK){
                double dns_time = 0.0;
                double connect_time = 0.0;

                curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &dns_time);
                curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &connect_time);

                double latency = connect_time - dns_time;

                if (best_server == NULL || latency < best_pure_time) {
                    best_pure_time = latency;
                    best_server = &servers[i];
                }
            }

            curl_easy_cleanup(curl);
        }
    }
    return best_server;
}