#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>

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

// Returns non-zero if word contains any excluded letter.
int contains_excluded_letter(const char *word, size_t wlen, const unsigned int excluded_mask) {
    for (size_t i = 0; i < wlen; i++) {
        unsigned char c = (unsigned char)word[i];
        if (!isalpha(c)) {
            continue;
        }
        unsigned char lower = (unsigned char)tolower(c);
        if (excluded_mask & (1u << (lower - 'a'))) {
            return 1;
        }
    }
    return 0;
}

// Returns non-zero if word contains every included letter at least once.
int contains_all_included_letters(const char *word, size_t wlen, const unsigned int included_mask) {
    if (included_mask == 0) {
        return 1;
    }

    unsigned int word_mask = 0;
    for (size_t i = 0; i < wlen; i++) {
        unsigned char c = (unsigned char)word[i];
        if (!isalpha(c)) {
            continue;
        }
        unsigned char lower = (unsigned char)tolower(c);
        word_mask |= 1u << (lower - 'a');
    }

    return (word_mask & included_mask) == included_mask;
}

int main(int argc, char *argv[]) {
    static struct option long_options[] = {
        {"exclude", required_argument, 0, 'e'},
        {"include", required_argument, 0, 'i'},
        {0, 0, 0, 0},
    };

    const char *exclude_letters = NULL;
    const char *include_letters = NULL;
    int opt;
    while ((opt = getopt_long(argc, argv, "e:i:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'e':
                exclude_letters = optarg;
                break;
            case 'i':
                include_letters = optarg;
                break;
            default:
                fprintf(stderr, "usage: %s [--exclude letters] [--include letters] PATTERN\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (optind != argc - 1) {
        fprintf(stderr, "usage: %s [--exclude letters] [--include letters] PATTERN\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *pattern = argv[optind];
    size_t plen = strlen(pattern);
    if (plen < 2 || plen > MAX_LEN) {
        fprintf(stderr, "pattern length must be between 2 and %d\n", MAX_LEN);
        return EXIT_FAILURE;
    }

    unsigned int excluded_mask = 0;
    if (exclude_letters) {
        for (size_t i = 0; exclude_letters[i] != '\0'; i++) {
            unsigned char c = (unsigned char)exclude_letters[i];
            if (!isalpha(c)) {
                fprintf(stderr, "exclude letters must be alphabetic only\n");
                return EXIT_FAILURE;
            }
            c = (unsigned char)tolower(c);
            excluded_mask |= 1u << (c - 'a');
        }
    }

    unsigned int included_mask = 0;
    if (include_letters) {
        for (size_t i = 0; include_letters[i] != '\0'; i++) {
            unsigned char c = (unsigned char)include_letters[i];
            if (!isalpha(c)) {
                fprintf(stderr, "include letters must be alphabetic only\n");
                return EXIT_FAILURE;
            }
            c = (unsigned char)tolower(c);
            included_mask |= 1u << (c - 'a');
        }
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

        if (wlen == plen
            && match_word(line_start, wlen, pattern, plen)
            && !contains_excluded_letter(line_start, wlen, excluded_mask)
            && contains_all_included_letters(line_start, wlen, included_mask)) {
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
