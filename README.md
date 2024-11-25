# RSS Feed Fetcher

v0.0.1

---

# Overview:

A simple CLI tool for fetching and displaying rss feeds

## How to use:

Setup:

1. Install required c dependencies
2. Add your desired rss feeds to feeds.json file
3. Compile the binary with "gcc -o fetch fetch.c -lcurl -ljson-c"

4. Use "chmod +x" on the binary to make it executable

Run the binary:
- use "./fetch"

Filter results by date:

- Use "./fetch | grep -A 3 'Sun, 24' | grep -E 'Headline:|URL:|Publication Date:'" to search for headlines by date

---

## Credits & Acknowledgements:

Built with:
- **Standard Library**: `stdio.h`, `stdlib.h`, `string.h`
- **libcurl**: `curl/curl.h` (for handling HTTP requests and RSS parsing)
- **JSON-C**: `json-c/json.h` (for parsing `feeds.json`)
