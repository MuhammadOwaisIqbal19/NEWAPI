#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "cJSON.h"
#include <string.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);

    if (mem->memory == NULL) {
        fprintf(stderr, "realloc() failed\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int main() {
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if (curl) {
        struct MemoryStruct chunk;
        chunk.memory = malloc(1);  // Initialize with an empty string
        chunk.size = 0;

        curl_easy_setopt(curl, CURLOPT_URL, "http://api.openweathermap.org/data/2.5/forecast?lat=24.86080000&lon=67.01040000&appid=960ab4072cce45cce5e399e8453ed270");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            cJSON *json = cJSON_Parse(chunk.memory);

            if (json != NULL) {
                // Print the JSON structure for debugging
                char *json_string = cJSON_Print(json);
                fprintf(stderr, "Parsed JSON: %s\n", json_string);
                free(json_string);

                // Replace "city" with the actual JSON key you want to retrieve
                cJSON *city = cJSON_GetObjectItem(json, "city");

                if (city != NULL && cJSON_IsObject(city)) {
                    cJSON *name = cJSON_GetObjectItem(city, "name");
                    cJSON *country = cJSON_GetObjectItem(city, "country");

                    if (name != NULL && cJSON_IsString(name) && country != NULL && cJSON_IsString(country)) {
                        printf("City: %s, Country: %s\n", name->valuestring, country->valuestring);
                    } else {
                        fprintf(stderr, "Failed to retrieve 'name' or 'country' from JSON.\n");
                    }
                } else {
                    fprintf(stderr, "Failed to retrieve 'city' from JSON.\n");
                }

                // Clean up
                cJSON_Delete(json);
            } else {
                fprintf(stderr, "Failed to parse JSON.\n");
            }
        }

        // Clean up
        curl_easy_cleanup(curl);
        free(chunk.memory);
    }

    curl_global_cleanup();

    return 0;
}

