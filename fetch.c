#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>

// Data structure to hold fetched RSS feed
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Write callback to store fetched data in memory
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + total_size + 1);
	if (ptr == NULL) {
    	fprintf(stderr, "Not enough memory!\n");
    	free(mem->memory);
    	return 0;
	}
	mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, total_size);
    mem->size += total_size;
    mem->memory[mem->size] = '\0';

    return total_size;
}

// Extract content between start_tag and end_tag
void extract_element(const char *content, const char *start_tag, const char *end_tag, char *output, size_t max_size) {
    char *start = strstr(content, start_tag);
    if (start) {
        start += strlen(start_tag);
        char *end = strstr(start, end_tag);
        if (end) {
            size_t length = end - start;
            if (length < max_size) {
                strncpy(output, start, length);
                output[length] = '\0';
            }
        }
    }
}

// Parse and print RSS feed
void parse_rss_feed(const char *rss_content) {
    const char *item_start = "<item>";
    const char *item_end = "</item>";
    char *item = strstr(rss_content, item_start);

    while (item) {
        char *item_close = strstr(item, item_end);
        if (!item_close) break;

        char title[512] = {0};
        char link[512] = {0};
        char pubDate[128] = {0};

        extract_element(item, "<title>", "</title>", title, sizeof(title));
        extract_element(item, "<link>", "</link>", link, sizeof(link));
        extract_element(item, "<pubDate>", "</pubDate>", pubDate, sizeof(pubDate));

        if (strlen(title) > 0 && strlen(link) > 0 && strlen(pubDate) > 0) {
            printf("Headline: %s\n", title);
            printf("URL: %s\n", link);
            printf("Publication Date: %s\n\n", pubDate);
        }

        item = strstr(item_close, item_start);
    }
}

// Fetch RSS feed and pass to parser
void fetch_rss_feed(const char *url) {
    CURL *curl;
    CURLcode res;

    struct MemoryStruct chunk = {NULL, 0};

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        printf("Fetching: %s\n", url);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            if (chunk.memory != NULL) {
			    parse_rss_feed(chunk.memory);
			} else {
				fprintf(stderr, "Error: No content fetched.\n");
			}
        }

        free(chunk.memory);
        curl_easy_cleanup(curl);
    }
}

// Read feeds from feeds.json
void read_feeds_from_json(const char *file_path, char ***feeds, size_t *num_feeds) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        perror("Unable to open feeds.json");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *json_data = malloc(file_size + 1);
    fread(json_data, 1, file_size, file);
    json_data[file_size] = '\0';
    fclose(file);

    struct json_object *parsed_json = json_tokener_parse(json_data);
    struct json_object *rss_feeds_array;

    if (json_object_object_get_ex(parsed_json, "rss_feeds", &rss_feeds_array)) {
        *num_feeds = json_object_array_length(rss_feeds_array);
        *feeds = malloc(*num_feeds * sizeof(char *));

        for (size_t i = 0; i < *num_feeds; i++) {
            (*feeds)[i] = strdup(json_object_get_string(json_object_array_get_idx(rss_feeds_array, i)));
        }
    }

    free(json_data);
    json_object_put(parsed_json);
}

int main() {
    char **rss_feeds;
    size_t num_feeds;

    read_feeds_from_json("feeds.json", &rss_feeds, &num_feeds);

    curl_global_init(CURL_GLOBAL_DEFAULT);

    for (size_t i = 0; i < num_feeds; i++) {
        fetch_rss_feed(rss_feeds[i]);
        free(rss_feeds[i]);
    }

    free(rss_feeds);
    curl_global_cleanup();

    return 0;
}
