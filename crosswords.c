#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_LEN 26

// Check if word matches pattern (_ is wildcard)
int match_word(const char *word, size_t wlen, const char *pattern, size_t plen) {
    if (wlen != plen) return 0;
    for (size_t i = 0; i < plen; i++) {
        if (pattern[i] != '_' && pattern[i] != word[i]) {
            return 0;
        }
    }
    return 1;
}

// mmap file helper
char *mmap_file(const char *path, size_t *size_out) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return NULL;

    struct stat sb;
    if (fstat(fd, &sb) < 0) {
        close(fd);
        return NULL;
    }

    char *data = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (data == MAP_FAILED) return NULL;

    *size_out = sb.st_size;
    return data;
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

    // Load only dictionary for this length
    char path[256];
    snprintf(path, sizeof(path), "./dictionary/%zu.txt", plen);

    size_t fsize = 0;
    char *fdata = mmap_file(path, &fsize);
    if (!fdata) {
        fprintf(stderr, "failed to mmap %s\n", path);
        return EXIT_FAILURE;
    }

    size_t match_count = 0;
    char *ptr = fdata;
    char *end = fdata + fsize;

    while (ptr < end) {
        char *line_start = ptr;
        while (ptr < end && *ptr != '\n' && *ptr != '\r') ptr++;
        size_t wlen = ptr - line_start;

        if (wlen == plen && match_word(line_start, wlen, pattern, plen)) {
            fwrite(line_start, 1, wlen, stdout);
            fputc('\n', stdout);
            match_count++;
        }

        // Skip newline characters
        while (ptr < end && (*ptr == '\n' || *ptr == '\r')) ptr++;
    }

    printf("%zu matches\n", match_count);

    munmap(fdata, fsize);
    return EXIT_SUCCESS;
}

