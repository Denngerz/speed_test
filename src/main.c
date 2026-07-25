#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <unistd.h>
#include "../include/speedtest.h"
#include "../include/location.h"
#include "../include/server_select.h"
#include "../include/http_utils.h"
#include "../include/server_list.h"

void provide_location(){
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

void provide_upload(char *server_host){
    printf("\n===Upload Speed===\n");

    if(server_host == NULL){
        printf("Server host is not provided (-s)\n");
    }
    else{
        printf("Server with which test is being performed: %s\n", server_host);
        double random_server_upload_speed = get_server_upload_speed(server_host);
        printf("Data upload speed: %f Mb\n", random_server_upload_speed);
    }

    printf("================\n");
}

void provide_download(char *server_host){
    printf("\n===Download Speed===\n");

    if(server_host == NULL){
        printf("Server host is not provided (-s)\n");
    }
    else{
        printf("Server with which test is being performed: %s\n", server_host);
        double random_server_download_speed = get_server_download_speed(server_host);
        printf("Data download speed: %f Mb\n", random_server_download_speed);
    }

    printf("================\n");
}

void provide_all_info(ServerInfo *all_servers, int count){
    printf("\n===Full test===\n");

    char *location = detect_location();

    if(location == NULL){
        printf("Coundn`t detect user location");
    }
    else{
        printf("Your location: %s\n", location);

        printf("Searching for best valid server in %s ...\n", location);
        ServerInfo *matching_server = find_server_by_country(all_servers, count, location);
        if (matching_server == NULL) {
            printf("Couldn't find a valid server\n");
            free(location);
            printf("================\n");
            return;
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

int main(int argc, char *argv[]) {
    int count;
    ServerInfo *all_servers = load_servers_info("data/speedtest_server_list.json", &count);

    if(all_servers == NULL){
        fprintf(stderr, "Failed to load server list.\n");
        return 1;
    }

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
        provide_location();
    }
    if(run_upload == 1){
        provide_upload(server_host);
    }
    if(run_download == 1){
       provide_download(server_host);
    }
    if(run_all == 1){
       provide_all_info(all_servers, count);
    }

    free(all_servers);
    return 0;
}