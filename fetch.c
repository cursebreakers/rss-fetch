#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

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
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, total_size);
    mem->size += total_size;
    mem->memory[mem->size] = '\0';  // Use null-termination explicitly

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
            parse_rss_feed(chunk.memory);
        }

        free(chunk.memory);
        curl_easy_cleanup(curl);
    }
}

int main() {
const char *rss_feeds[] = {
    "https://news.google.com/rss/search?q=site%3Areuters.com&hl=en-US&gl=US&ceid=US%3Aen",
    "https://rss.nytimes.com/services/xml/rss/nyt/HomePage.xml",
    "https://www.theguardian.com/world/rss",
    "https://www.aljazeera.com/xml/rss/all.xml",
    "https://www.rt.com/rss/news/",
};

    size_t num_feeds = sizeof(rss_feeds) / sizeof(rss_feeds[0]);

    curl_global_init(CURL_GLOBAL_DEFAULT);

    for (size_t i = 0; i < num_feeds; i++) {
        fetch_rss_feed(rss_feeds[i]);
    }

    curl_global_cleanup();
    return 0;
}
