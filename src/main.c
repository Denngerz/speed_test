#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <unistd.h>

typedef struct {
    char country[64];
    char city[64];
    char provider[64];
    char host[128];
    int id;
} ServerInfo;

typedef struct {
    char *data;
    size_t size;
} ResponseBuffer;

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

size_t download_callback(char *buffer, size_t itemsize, size_t nitems, void *userdata) {
    return itemsize * nitems;
}

double get_server_download_speed(const char *host){
    CURL *curl = curl_easy_init();
    float speed = 0.0f;
    if (curl) {
        char url[256];
        snprintf(url, sizeof(url), "http://%s/download?size=25000000", host);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_callback);
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

size_t upload_callback(char *buffer, size_t itemsize, size_t nitems, void *userdata) {
    return itemsize * nitems;
}

size_t upload_read_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
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

double get_server_upload_speed(const char *host) {
    double speed = 0.0;

    CURL *curl = curl_easy_init();
    if (curl) {
        char url[256];
        snprintf(url, sizeof(url), "http://%s/upload", host);

        size_t state = 25000000;

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, upload_read_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, upload_callback);
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


size_t got_data(char *buffer, size_t itemsize, size_t nitems, void *userdata) {
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

char* detect_location(){
    ResponseBuffer resp = { .data = malloc(1), .size = 0 };
    resp.data[0] = '\0';

    char *country_name = NULL;
    
    CURL *curl = curl_easy_init();
    if (curl) {
        CURLcode result;
        curl_easy_setopt(curl, CURLOPT_URL, "http://ip-api.com/json/");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, got_data);
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

ServerInfo* find_server_by_country(ServerInfo *servers, int count, const char *country) {
    for (int i = 0; i < count; i++) {
        if (strcmp(servers[i].country, country) == 0) {
            CURL *curl = curl_easy_init();
            if (!curl) {
                return NULL;
            }

            char url[256];
            snprintf(url, sizeof(url), "http://%s", servers[i].host);

            CURLcode result;
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

            result = curl_easy_perform(curl);

            curl_easy_cleanup(curl);

            if (result != CURLE_OK) {
                fprintf(stderr, "Failed to connect to host: %s\n", url);
                continue;
            } 

            return &servers[i];
        }
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    int count;
    ServerInfo *all_servers = load_servers_info("data/speedtest_server_list.json", &count);

    int run_download = 0, run_upload = 0, run_location = 0, run_all = 0;
    char *server_host = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "ludas:")) != -1) {
        switch (opt) {
            case 'l': {
                run_location = 1; 
                break;
            }
            case 'u':{
                run_upload = 1;
                break;
            }
            case 'd': {
                run_download = 1;
                break;
            }
            case 'a': {
                run_all = 1;
                break;
            }
            case 's': {
                server_host = optarg;
                break;
            }
            default:
                return 1;
        }
    }

    if(run_location == 1){
        printf("\n===Location===\n");

        char *location = detect_location();

        if(location == NULL){
            printf("Coundn`t detect user location\n");
        }
        else{
            printf("Your location: %s\n", location);
        }

        free(location);

        printf("================\n");
    }
    if(run_upload == 1){
        printf("\n===Upload Speed===\n");

        if(server_host == NULL){
            printf("Server host is not provided (-s)\n");
        }
        else{
            double random_server_upload_speed = get_server_upload_speed(server_host);
            printf("Data upload speed: %f Mb\n", random_server_upload_speed);
        }

        printf("================\n");
    }
    if(run_download == 1){
        printf("\n===Download Speed===\n");

        if(server_host == NULL){
            printf("Server host is not provided (-s)\n");
        }
        else{
            double random_server_download_speed = get_server_download_speed(server_host);
            printf("Data download speed: %f Mb\n", random_server_download_speed);
        }

        printf("================\n");
    }
    if(run_all == 1){
        printf("\n===Full test===\n");

        char *location = detect_location();

        printf("Your location: %s\n", location);

        if(location == NULL){
            printf("Coundn`t detect user location");
        }
        else{
            ServerInfo *matching_server = find_server_by_country(all_servers, count, location);

            if(matching_server == NULL){
                printf("Coudn`t find a valid server\n");
                return 1;
            }

            printf("Server with which test is being performed: %s\n", matching_server->host);
            
            double random_server_download_speed = get_server_download_speed(matching_server->host);
            printf("Data download speed: %f Mb\n", random_server_download_speed);

            double random_server_upload_speed = get_server_upload_speed(matching_server->host);
            printf("Data upload speed: %f Mb\n", random_server_upload_speed);
            
            free(location);
        }

         printf("================\n");
    }

    free(all_servers);
    return 0;
}