#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LEN 100
#define MAX_WORDS_PER_LEN 100000
#define MAX_LEN 26

// Structure to hold words by length
typedef struct {
    char **list;
    size_t count;
    size_t capacity;
} WordList;

WordList words[MAX_LEN + 1];

// Read lines from a file into the given WordList
int load_dictionary(const char *path, WordList *wl) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char buf[MAX_WORD_LEN];
    while (fgets(buf, sizeof(buf), f)) {
        size_t len = strlen(buf);
        if (len && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
            buf[--len] = '\0';

        if (!wl->list) {
            wl->capacity = 1024;
            wl->count = 0;
            wl->list = malloc(wl->capacity * sizeof(char*));
        }
        else if (wl->count >= wl->capacity) {
            wl->capacity *= 2;
            wl->list = realloc(wl->list, wl->capacity * sizeof(char*));
        }

        wl->list[wl->count++] = strdup(buf);
    }
    fclose(f);
    return 0;
}

// Check if word matches pattern (_ is wildcard)
int match_word(const char *word, const char *pattern) {
    size_t n = strlen(pattern);
    if (strlen(word) != n) return 0;
    for (size_t i = 0; i < n; i++) {
        char pc = tolower(pattern[i]);
        char wc = tolower(word[i]);
        if (pc != '_' && pc != wc) return 0;
    }
    return 1;
}

void free_wordlist(WordList *wl) {
    if (!wl->list) return;
    for (size_t i = 0; i < wl->count; i++)
        free(wl->list[i]);
    free(wl->list);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s PATTERN\n", argv[0]);
        return EXIT_FAILURE;
    }
    char *pattern = argv[1];
    size_t plen = strlen(pattern);
    if (plen < 2 || plen > MAX_LEN) {
        fprintf(stderr, "pattern length must be between 2 and %d\n", MAX_LEN);
        return EXIT_FAILURE;
    }

    // Load dictionaries from length 2 up to MAX_LEN
    for (int len = 2; len <= MAX_LEN; len++) {
        char path[256];
        sprintf(path, "./dictionary/%d.txt", len);
        if (load_dictionary(path, &words[len]) != 0) {
            fprintf(stderr, "warning: could not load %s\n", path);
            // continue even if missing
        }
    }

    WordList *wl = &words[plen];
    if (!wl->list) {
        printf("no dictionary for length %zu\n", plen);
        return EXIT_SUCCESS;
    }

    // Find matches
    char **matched = NULL;
    size_t mcount = 0, mcap = 0;
    for (size_t i = 0; i < wl->count; i++) {
        if (match_word(wl->list[i], pattern)) {
            if (mcount >= mcap) {
                mcap = mcap ? mcap * 2 : 64;
                matched = realloc(matched, mcap * sizeof(char*));
            }
            matched[mcount++] = wl->list[i];
        }
    }

    // Output
    for (size_t i = 0; i < mcount; i++)
        printf("%s\n", matched[i]);
    printf("%zu matches\n", mcount);

    // Cleanup
    free(matched);
    for (int len = 2; len <= MAX_LEN; len++)
        free_wordlist(&words[len]);

    return EXIT_SUCCESS;
}

