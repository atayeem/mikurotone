#include <math.h>
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>

#define NAME "[internal] "

#ifdef _WIN32
#  define strdup _strdup
#endif

static bool has_extension(const char* filename, const char* extension) {
    const char* ext = strrchr(filename, '.');
    return ext && strcmp(ext, extension) == 0;
}

typedef struct {
    double notes[128];
} Tuning;

static Tuning* read_tun_file(const char* filename) {
    FILE* tunfile = fopen(filename, "r");
    if (!tunfile) {
        fprintf(stderr, NAME "Failed to open file '%s'. Exiting\n", filename);
        exit(EXIT_FAILURE);
    }

    Tuning* ret = calloc(1, sizeof(Tuning));
    if (!ret) {
        fprintf(stderr, NAME "Failed to allocate memory for tuning. Exiting\n");
        fclose(tunfile);
        exit(EXIT_FAILURE);
    }
    return ret;
}

static double parse_scl_num(const char* _s, const char* Dfile_name, uint64_t Dline_num) {
    double result = 0.0;
    
    size_t len = strlen(_s);
    char* s = strdup(_s);

    char* slash_pos = strchr(s, '/');
    char* dot_pos = strchr(s, '.');
    
    if (slash_pos && dot_pos) {
        fprintf(stderr, NAME "%s:%lld: Invalid scl value, has both a dot and slash: '%s'. Exiting\n", Dfile_name, Dline_num, _s);
        exit(EXIT_FAILURE);
    }

    else if (slash_pos) {
        if (len < 3) {
            fprintf(stderr, NAME "%s:%lld: Invalid scl rational number: '%s'. Exiting\n", Dfile_name, Dline_num, _s);
            exit(EXIT_FAILURE);
        }

        const char* num1 = s;
        const char* num2 = slash_pos + 1;

        *slash_pos = '\0';

        char *num_stop, *den_stop;
        double num = (double) strtoll(num1, &num_stop, 10);

        if (errno == ERANGE) {
            fprintf(stderr, "%s:%lld: Numerator %s was larger than maximum possible value %lld. Exiting\n", Dfile_name, Dline_num, num1, LONG_LONG_MAX);
            exit(EXIT_FAILURE);
        }
        double den = (double) strtoll(num2, &den_stop, 10);
        if (errno == ERANGE) {
            fprintf(stderr, "%s:%lld: Denominator %s was larger than maximum possible value %lld. Exiting\n", Dfile_name, Dline_num, num2, LONG_LONG_MAX);
            exit(EXIT_FAILURE);
        }
        if (num_stop < num2 - 1) {
            fprintf(stderr, NAME "%s:%lld: Failed to parse the numerator of the ratio, got %lf from '%s' in '%s'\n",
                                    Dfile_name, Dline_num, num, num1, _s);
            fprintf(stderr, NAME "Continuing...\n");
        }
        if (*den_stop != '\0') {
            fprintf(stderr, NAME "%s:%lld: Failed to parse the denominator of the ratio, got %lf from '%s' in '%s'\n",
                                    Dfile_name, Dline_num, num, num1, _s);
            fprintf(stderr, NAME "Continuing...\n");
        }

        result = log2(num / den) * 1200;
    }

    else if (dot_pos) {
        char *endptr;
        result = strtod(s, &endptr);
        if (endptr < s + len) {
            fprintf(stderr, NAME "%s:%lld: Failed to parse decimal value in scl file, got %lf from '%s'\n", Dfile_name, Dline_num, result, _s);
            fprintf(stderr, NAME "Continuing...\n");
        }
        if (errno == ERANGE) {
            fprintf(stderr, NAME "%s:%lld: Failed to parse decimal value, value was larger than maximum possible value, in '%s'\n", Dfile_name, Dline_num, _s);
            fprintf(stderr, NAME " Continuing...\n");
        }
    }

    else {
        
    }

    free(s);
    return result;
}

static Tuning* read_scl_file(const char* filename) {
    
}

Tuning* read_tuning_file(const char* filename) {
    if (has_extension(filename, ".tun")) {
        return read_tun_file(filename);
    } else if (has_extension(filename, ".scl")) {
        return read_scl_file(filename);
    } else {
        fprintf(stderr, NAME "Can't recognize file as either .tun or .scl: %s\n", filename);
        return NULL;
    }
}