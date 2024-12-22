#define NEXUS_IMPLEMENTATION
#include "nexus.h"

/* clang-format off */
#define COMMON_FLAGS \
    "-fdiagnostics-color=always", \
    "-fPIE", \
    "-ansi", \
    "-O2", \
    "-march=native", \
    "-flto"
/* clang-format on */

static void print_help(void) {
    printf("Usage: fragart [options]\n\n");
    printf("Options:\n");
    printf("  --help, -h       Show this help menu\n");
    printf("  --clean, -c      Remove generated files\n");
}

static int clean(const char *files[], size_t file_count) {
    for (size_t i = 0; i < file_count; i++) {
        if (remove(files[i]) != 0) {
            perror(files[i]);
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    NX_REBUILD(argc, argv);

    const char *files_to_remove[] = {"glad.o", "fragart"};

    for (int i = 1; i < argc; i++) {
        if (!nx_strcmp(argv[i], "--help") || !nx_strcmp(argv[i], "-h")) {
            print_help();
            return EXIT_SUCCESS;
        }

        if (!nx_strcmp(argv[i], "--clean") || !nx_strcmp(argv[i], "-c")) {
            return clean(files_to_remove, nx_len(files_to_remove));
        }
    }

    /* clang-format off */
    const char *glad_args[] = {
        "cc",
        "-c",
        "glad/glad.c",
        "-o",
        "glad.o"
    };

    const char *nexus_args[] = {
        "cc",
        "main.c",
        "glad.o",
        "-o",
        "fragart",
        "-lglfw",
        "-lm",
        COMMON_FLAGS
    };
    /* clang-format on */

    int glad_result = nx_compile_command("glad", glad_args, nx_len(glad_args), false);
    if (glad_result != 0) {
        return glad_result;
    }

    int nexus_result = nx_compile_command("fragart", nexus_args, nx_len(nexus_args), true);
    if (nexus_result != 0) {
        return nexus_result;
    }

    return EXIT_SUCCESS;
}
