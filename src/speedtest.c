#include "../include/speedtest.h"
#include "../include/http_utils.h"
#include <curl/curl.h>
#include <stdio.h>
#include <string.h>

double get_server_download_speed(const char *host){
    CURL *curl = curl_easy_init();
    float speed = 0.0f;
    if (curl) {
        char url[256];
        snprintf(url, sizeof(url), "http://%s/download?size=25000000", host);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_callback);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/8.5.0");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

        CURLcode result = curl_easy_perform(curl);

        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        curl_off_t downloaded;
        curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD_T, &downloaded);

        if (result != CURLE_OK && result != CURLE_OPERATION_TIMEDOUT) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
            speed = 0.0;
        } 
        else {
            curl_off_t speed_bytes;
            curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD_T, &speed_bytes);
            speed = (speed_bytes * 8.0) / 1000000.0;
        }

        curl_easy_cleanup(curl);
    }

    return speed;
}

static size_t upload_read_callback(char *buffer, size_t size, size_t nitems, void *userdata){
    size_t *state = (size_t *)userdata;
    size_t max_chunk = size * nitems;
    size_t to_send = (*state < max_chunk) ? *state : max_chunk;

    if (to_send == 0) {
        return 0;
    }

    memset(buffer, 0, to_send);
    *state -= to_send;
    return to_send;
}

double get_server_upload_speed(const char *host){
    double speed = 0.0;

    CURL *curl = curl_easy_init();
    if (curl) {
        char url[256];
        snprintf(url, sizeof(url), "http://%s/upload", host);

        size_t state = 25000000;

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, upload_read_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_callback);
        curl_easy_setopt(curl, CURLOPT_READDATA, &state);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)state);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/8.5.0");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

        CURLcode result = curl_easy_perform(curl);

        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if (result != CURLE_OK && result != CURLE_OPERATION_TIMEDOUT) {
            fprintf(stderr, "upload failed: %s\n", curl_easy_strerror(result));
        } 
        else {
            curl_off_t speed_bytes;
            curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD_T, &speed_bytes);
            speed = (speed_bytes * 8.0) / 1000000.0;
        }

        curl_easy_cleanup(curl);
    }
    return speed;
}