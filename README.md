# RSS Feed Fetcher

v0.0.1

*A simple CLI tool for fetching and displaying rss feeds*

---

## How to use:

Setup:

1. Install required c dependencies

3. Compile the binary with `gcc -o fetch fetch.c -lcurl -ljson-c`

2. Add your desired rss feeds to feeds.json file

4. You may need to `chmod +x` the binary to make it executable

5. Use `./fetch` to run the binary

To filter results by date: `./fetch | grep -A 3 'Mon, 24' | grep -E 'Headline:|URL:|Publication Date:'`

---

## Credits & Acknowledgements:

Built with:
- **Standard Library**: `stdio.h`, `stdlib.h`, `string.h`
- **libcurl**: `curl/curl.h` (for handling HTTP requests and RSS parsing)
- **JSON-C**: `json-c/json.h` (for parsing `feeds.json`)
