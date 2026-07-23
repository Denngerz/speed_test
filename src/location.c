#include "../include/location.h"
#include "../include/http_utils.h"
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* detect_location(void){
    ResponseBuffer resp = { .data = malloc(1), .size = 0 };
    resp.data[0] = '\0';

    char *country_name = NULL;
    
    CURL *curl = curl_easy_init();
    if (curl) {
        CURLcode result;
        curl_easy_setopt(curl, CURLOPT_URL, "http://ip-api.com/json/");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, accumulate_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);

        result = curl_easy_perform(curl);

        if (result != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
        } 
        else {
            cJSON *json = cJSON_Parse(resp.data);
            if (json == NULL) {
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL) {
                    fprintf(stderr, "JSON parse error near: %s\n", error_ptr);
                }
            }
            else {
                cJSON *country = cJSON_GetObjectItemCaseSensitive(json, "country");
                if (!cJSON_IsString(country)) {
                    fprintf(stderr, "Country field missing or invalid.\n");
                }
                else {
                    country_name = strdup(country->valuestring);
                }
            }
            cJSON_Delete(json);
        }
        curl_easy_cleanup(curl);
    }

    free(resp.data);
    return country_name;
}