/* Copyright (C) 2024 Nexus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Mozilla Public License, version 2.0.
 *
 * This program is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 * See the Mozilla Public License for more details.
 *
 * You should have received a copy of the Mozilla Public License
 * along with this program. If not, see <https://www.mozilla.org/MPL/2.0/>.
 *
 * ===== DOCUMENTATION ====================================================
 *
 * This single header ANSI C library is designed to make your C life easier.
 * It does not depend on anything. It includes a memory tracker, a command
 * runner, a file IO helper, handy macros and more to make C suck less.
 *
 * This library is implemented in ANSI C, but there is support for some C99
 * features.
 *
 * ===== USAGE ============================================================
 *
 * To use the implementation of this library, define the following macro
 * before including this file:
 *
 *    #define NEXUS_IMPLEMENTATION
 *
 * i.e. it should look like this:
 *    #include ...
 *    #include ...
 *    #define NEXUS_IMPLEMENTATION
 *    #include "nexus.h"
 *
 * ===== OPTIONS ==========================================================
 *
 *    #define NX_DEBUG
 *        Enables memory leak detection and printing of memory leaks.
 *
 *    #define NX_MATH
 *        Includes math.h. Disabled by default.
 *
 *    #define NX_REBUILD(argc, argv)
 *        Enables rebuilding of the executable when the source file is
 *        modified.
 *
 *    #define NX_ARENA_BLOCK_SIZE
 *        Sets the size of the arena blocks. Default is 4096.
 *
 *    #define NX_HASHMAP_INITIAL_CAPACITY
 *        Sets the initial capacity of the hashmap. Default is 16.
 *
 *    #define NX_HASHMAP_LOAD_FACTOR
 *        Sets the load factor of the hashmap. Default is 0.75f.
 *
 *    #define NX_STRING_BUILDER_INITIAL_CAPACITY
 *        Sets the initial capacity of the string builder. Default is 256.
 *
 *    #define NX_STRING_BUILDER_GROWTH_FACTOR
 *        Sets the growth factor of the string builder. Default is 2.
 *
 * ===== VERSIONING =======================================================
 * Version: 0.1.2
 * Release Date: 05-04-2025
 *
 * Changelog:
 * - Some functions that were public should've been private
 * - Some simple refactoring
 * - Die on allocation failure in string builder
 *
 * Version: 0.1.1
 * Release Date: 22-12-2024
 *
 * Changelog:
 * - Make use of boolean flags
 *
 * Version: 0.1.0
 * Release Date: 22-12-2024
 *
 * Changelog:
 * - Added NX_MATH macro to include math.h
 * - Make documentation pretty
 * - Fix rebuild using old binary
 * - Make rebuild safe
 * - Add better overwritable macros section
 * - Add support for logging
 * - Add all color macros
 *
 * Version: 0.0.2
 * Release Date: 20-12-2024
 *
 * Changelog:
 * - Better build system that is less repetitive
 * - 100 char clang-format
 * - nx_die macro
 *
 * Version: 0.0.1
 * Release Date: 17-12-2024
 *
 * Changelog:
 * - Handy everyday macros
 * - Overwritable macros
 * - Memory tracker to track mem leaks
 * - Arena allocation
 * - Single and Double Linked Lists
 * - Hashmap
 * - String Builder
 * - Command Runner (in which you can also build your project with)
 * - File IO helpers
 */
#ifndef NEXUS_H
#define NEXUS_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
typedef int bool;
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#endif /* __STDC_VERSION__ */

#ifdef NX_MATH
#include <math.h>
#endif

#ifndef NX_ARENA_BLOCK_SIZE
#define NX_ARENA_BLOCK_SIZE 4096
#endif

#ifndef NX_HASHMAP_INITIAL_CAPACITY
#define NX_HASHMAP_INITIAL_CAPACITY 16
#endif

#ifndef NX_HASHMAP_LOAD_FACTOR
#define NX_HASHMAP_LOAD_FACTOR 0.75f
#endif

#ifndef NX_STRING_BUILDER_INITIAL_CAPACITY
#define NX_STRING_BUILDER_INITIAL_CAPACITY 256
#endif

#ifndef NX_STRING_BUILDER_GROWTH_FACTOR
#define NX_STRING_BUILDER_GROWTH_FACTOR 2
#endif

/* Macros {{{ */
#define nx_join_(a, b) a##b
#define nx_join(a, b) nx_join_(a, b)
#define nx_stringify_(x) #x
#define nx_stringify(x) nx_stringify_(x)

#define nx_len(arr) (sizeof(arr) / sizeof(arr[0]))
#define nx_end(arr) (arr + nx_len(arr))
#define nx_min(a, b) ((a) < (b) ? (a) : (b))
#define nx_max(a, b) ((a) > (b) ? (a) : (b))
#define nx_abs(a) ((a) < 0 ? -(a) : (a))
#define nx_clamp(a, min, max) nx_min(nx_max(a, min), max)
#define nx_member(T, m) (((T *) 0)->m)
#define nx_statement(code)                                                                         \
    do {                                                                                           \
        code                                                                                       \
    } while (0)
#define nx_swap(a, b, tmp)                                                                         \
    nx_statement({                                                                                 \
        tmp = a;                                                                                   \
        a   = b;                                                                                   \
        b   = tmp;                                                                                 \
    })
#define nx_assert(condition, message)                                                              \
    nx_statement({                                                                                 \
        if (!(condition)) {                                                                        \
            fprintf(stderr, "ASSERTION FAILED: %s:%d: %s\n", __FILE__, __LINE__, message);         \
            exit(EXIT_FAILURE);                                                                    \
        }                                                                                          \
    })

/* Mathematical constants */
#define PI 3.14159265358979323846f
#define E 2.71828182845904523536f
#define PHI 1.61803398874989484820f /* Golden ratio */

/* Conversions */
#define DEG_TO_RAD(angle) ((angle) * (PI / 180.0f))
#define RAD_TO_DEG(angle) ((angle) * (180.0f / PI))

/* Physics constants */
#define LIGHT_SPEED 299792458 /* Speed of light in m/s */
#define GRAVITY 9.80665f      /* Gravitational constant in m/s^2 */

/* Common values */
#define MAX_INT 2147483647
#define MIN_INT (-2147483648)
#define MAX_DOUBLE 1.7976931348623158e+308
#define MIN_DOUBLE 2.2250738585072014e-308

#define COLOR_BLACK "\x1b[30m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_BLUE "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_WHITE "\x1b[37m"

#define COLOR_BRIGHT_BLACK "\x1b[90m"
#define COLOR_BRIGHT_RED "\x1b[91m"
#define COLOR_BRIGHT_GREEN "\x1b[92m"
#define COLOR_BRIGHT_YELLOW "\x1b[93m"
#define COLOR_BRIGHT_BLUE "\x1b[94m"
#define COLOR_BRIGHT_MAGENTA "\x1b[95m"
#define COLOR_BRIGHT_CYAN "\x1b[96m"
#define COLOR_BRIGHT_WHITE "\x1b[97m"

#define COLOR_BG_BLACK "\x1b[40m"
#define COLOR_BG_RED "\x1b[41m"
#define COLOR_BG_GREEN "\x1b[42m"
#define COLOR_BG_YELLOW "\x1b[43m"
#define COLOR_BG_BLUE "\x1b[44m"
#define COLOR_BG_MAGENTA "\x1b[45m"
#define COLOR_BG_CYAN "\x1b[46m"
#define COLOR_BG_WHITE "\x1b[47m"

#define COLOR_RESET "\x1b[0m"
#define COLOR_BOLD "\x1b[1m"
#define COLOR_DIM "\x1b[2m"
#define COLOR_ITALIC "\x1b[3m"
#define COLOR_UNDERLINE "\x1b[4m"
#define COLOR_BLINK "\x1b[5m"
#define COLOR_REVERSE "\x1b[7m"
#define COLOR_HIDDEN "\x1b[8m"
/* }}} */

/* Overwritable macros {{{ */
#ifndef nx_memcpy
#define nx_memcpy memcpy
#endif
#ifndef nx_memmove
#define nx_memmove memmove
#endif
#ifndef nx_memset
#define nx_memset memset
#endif
#ifndef nx_memcmp
#define nx_memcmp memcmp
#endif

#ifndef nx_printf
#define nx_printf printf
#endif
#ifndef nx_fprintf
#define nx_fprintf fprintf
#endif
#ifndef nx_sprintf
#define nx_sprintf sprintf
#endif
#ifndef nx_scanf
#define nx_scanf scanf
#endif
#ifndef nx_fscanf
#define nx_fscanf fscanf
#endif
#ifndef nx_sscanf
#define nx_sscanf sscanf
#endif
#ifndef nx_vprintf
#define nx_vprintf vprintf
#endif
#ifndef nx_vfprintf
#define nx_vfprintf vfprintf
#endif
#ifndef nx_vsprintf
#define nx_vsprintf vsprintf
#endif

#ifndef nx_fopen
#define nx_fopen fopen
#endif
#ifndef nx_fclose
#define nx_fclose fclose
#endif
#ifndef nx_fread
#define nx_fread fread
#endif
#ifndef nx_fwrite
#define nx_fwrite fwrite
#endif
#ifndef nx_fseek
#define nx_fseek fseek
#endif
#ifndef nx_ftell
#define nx_ftell ftell
#endif
#ifndef nx_fflush
#define nx_fflush fflush
#endif
#ifndef nx_ferror
#define nx_ferror ferror
#endif
#ifndef nx_feof
#define nx_feof feof
#endif
#ifndef nx_remove
#define nx_remove remove
#endif
#ifndef nx_rename
#define nx_rename rename
#endif

#ifndef nx_strlen
#define nx_strlen strlen
#endif
#ifndef nx_strcpy
#define nx_strcpy strcpy
#endif
#ifndef nx_strncpy
#define nx_strncpy strncpy
#endif
#ifndef nx_strcat
#define nx_strcat strcat
#endif
#ifndef nx_strncat
#define nx_strncat strncat
#endif
#ifndef nx_strcmp
#define nx_strcmp strcmp
#endif
#ifndef nx_strncmp
#define nx_strncmp strncmp
#endif
#ifndef nx_strchr
#define nx_strchr strchr
#endif
#ifndef nx_strrchr
#define nx_strrchr strrchr
#endif
#ifndef nx_strstr
#define nx_strstr strstr
#endif
#ifndef nx_strtok
#define nx_strtok strtok
#endif

#ifndef nx_atoi
#define nx_atoi atoi
#endif
#ifndef nx_atof
#define nx_atof atof
#endif
#ifndef nx_atol
#define nx_atol atol
#endif
#ifndef nx_strtod
#define nx_strtod strtod
#endif
#ifndef nx_strtol
#define nx_strtol strtol
#endif
#ifndef nx_strtoul
#define nx_strtoul strtoul
#endif
#ifndef nx_sqrt
#define nx_sqrt sqrt
#endif
#ifndef nx_sin
#define nx_sin sin
#endif
#ifndef nx_cos
#define nx_cos cos
#endif
#ifndef nx_tan
#define nx_tan tan
#endif
#ifndef nx_asin
#define nx_asin asin
#endif
#ifndef nx_acos
#define nx_acos acos
#endif
#ifndef nx_atan
#define nx_atan atan
#endif
#ifndef nx_atan2
#define nx_atan2 atan2
#endif
#ifndef nx_sinh
#define nx_sinh sinh
#endif
#ifndef nx_cosh
#define nx_cosh cosh
#endif
#ifndef nx_tanh
#define nx_tanh tanh
#endif
#ifndef nx_log
#define nx_log log
#endif
#ifndef nx_log10
#define nx_log10 log10
#endif
#ifndef nx_exp
#define nx_exp exp
#endif
#ifndef nx_pow
#define nx_pow pow
#endif
#ifndef nx_floor
#define nx_floor floor
#endif
#ifndef nx_ceil
#define nx_ceil ceil
#endif
#ifndef nx_fabs
#define nx_fabs fabs
#endif
#ifndef nx_fmod
#define nx_fmod fmod
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) || defined(_MSC_VER)
#define nx_snprintf snprintf
#define nx_vsnprintf vsnprintf
#else
static int fallback_snprintf(char *str, size_t size, const char *format, ...)
    __attribute__((unused));
static int fallback_vsnprintf(char *str, size_t size, const char *format, va_list ap)
    __attribute__((unused));

static int fallback_snprintf(char *str, size_t size, const char *format, ...) {
    int     ret;
    va_list args;
    char   *buffer;
    va_start(args, format);

    buffer = (char *) malloc(size);
    if (buffer == NULL) {
        va_end(args);
        return -1;
    }

    ret = vsprintf(buffer, format, args);
    if (ret < 0) {
        free(buffer);
        va_end(args);
        return -1;
    }
    strncpy(str, buffer, size - 1);
    str[size - 1] = '\0';
    free(buffer);

    va_end(args);
    return ret;
}

static int fallback_vsnprintf(char *str, size_t size, const char *format, va_list ap) {
    int   ret;
    char *buffer;

    buffer = (char *) malloc(size);
    if (buffer == NULL) {
        return -1;
    }

    ret = vsprintf(buffer, format, ap);
    if (ret < 0) {
        free(buffer);
        return -1;
    }
    strncpy(str, buffer, size - 1);
    str[size - 1] = '\0';
    free(buffer);

    return ret;
}

#define nx_snprintf fallback_snprintf
#define nx_vsnprintf fallback_vsnprintf
#endif

#ifndef nx_strdup
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define nx_strdup strdup
#else
#define nx_strdup(s) (strcpy(malloc(strlen(s) + 1), s))
#endif
#endif

#ifndef nx_round
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define nx_round round
#else
#define nx_round(x) (floor((x) + 0.5))
#endif
#endif
/* }}} */

/* Memory Tracker {{{ */
#ifdef NX_DEBUG
void *nx_malloc_debug(size_t size, const char *file, int line);
void *nx_calloc_debug(size_t num, size_t size, const char *file, int line);
void *nx_realloc_debug(void *ptr, size_t size, const char *file, int line);
void  nx_free_debug(void *ptr);
void  nx_print_memory_leaks(void);

#define nx_malloc(size) nx_malloc_debug(size, __FILE__, __LINE__)
#define nx_calloc(num, size) nx_calloc_debug(num, size, __FILE__, __LINE__)
#define nx_realloc(ptr, size) nx_realloc_debug(ptr, size, __FILE__, __LINE__)
#define nx_free(ptr) nx_free_debug(ptr)
#else
#define nx_malloc(size) malloc(size)
#define nx_calloc(num, size) calloc(num, size)
#define nx_realloc(ptr, size) realloc(ptr, size)
#define nx_free(ptr) free(ptr)
#define nx_print_memory_leaks() (void) 0
#endif /* NX_DEBUG */
/* }}} */

/* Error Handling {{{ */
void nx_die_impl(const char *file, int line, const char *fmt, ...);

#define nx_die(fmt) nx_die_impl(__FILE__, __LINE__, fmt)
#define nx_die1(fmt, a1) nx_die_impl(__FILE__, __LINE__, fmt, a1)
#define nx_die2(fmt, a1, a2) nx_die_impl(__FILE__, __LINE__, fmt, a1, a2)
#define nx_die3(fmt, a1, a2, a3) nx_die_impl(__FILE__, __LINE__, fmt, a1, a2, a3)
/* }}} */

/* Arena {{{ */
typedef struct NXArenaBlock {
    void                *memory;
    size_t               used;
    size_t               size;
    struct NXArenaBlock *next;
} NXArenaBlock;

typedef struct {
    NXArenaBlock *first;
    NXArenaBlock *current;
} NXArena;

NXArena *nx_arena_create(void);
void    *nx_arena_alloc(NXArena *arena, size_t size);
void     nx_arena_reset(NXArena *arena);
void     nx_arena_destroy(NXArena *arena);
/* }}} */

/* Linked Lists {{{ */
typedef struct NXSSLNode {
    void             *data;
    struct NXSSLNode *next;
} NXSLLNode;

typedef struct {
    NXSLLNode *head;
    NXSLLNode *tail;
} NXSinglyLinkedList;

NXSinglyLinkedList *nx_sll_create(void);
void                nx_sll_destroy(NXSinglyLinkedList *list);
void                nx_sll_append(NXSinglyLinkedList *list, void *data);
void                nx_sll_prepend(NXSinglyLinkedList *list, void *data);
void                nx_sll_remove(NXSinglyLinkedList *list, void *data);

typedef struct NXDLLNode {
    void             *data;
    struct NXDLLNode *next;
    struct NXDLLNode *prev;
} NXDLLNode;

typedef struct {
    NXDLLNode *head;
    NXDLLNode *tail;
} NXDoublyLinkedList;

NXDoublyLinkedList *nx_dll_create(void);
void                nx_dll_destroy(NXDoublyLinkedList *list);
void                nx_dll_append(NXDoublyLinkedList *list, void *data);
void                nx_dll_prepend(NXDoublyLinkedList *list, void *data);
void                nx_dll_remove(NXDoublyLinkedList *list, void *data);
/* }}} */

/* Hashmap {{{ */
typedef struct nx_hashmap_entry {
    void                    *key;
    void                    *value;
    struct nx_hashmap_entry *next;
} NXHashMapEntry;

typedef struct nx_hashmap {
    NXHashMapEntry **buckets;
    size_t           capacity;
    size_t           size;
    size_t (*hash_func)(void *key);
    bool (*key_equal)(void *key1, void *key2);
} NXHashMap;

NXHashMap *nx_hashmap_create(size_t (*hash_func)(void *key),
                             bool (*key_equal)(void *key1, void *key2));
void       nx_hashmap_destroy(NXHashMap *map);
bool       nx_hashmap_insert(NXHashMap *map, void *key, void *value);
void      *nx_hashmap_get(NXHashMap *map, void *key);
bool       nx_hashmap_remove(NXHashMap *map, void *key);
/* }}} */

/* String Builder {{{ */
typedef struct {
    char  *buffer;
    size_t length;
    size_t capacity;
} NXStringBuilder;

NXStringBuilder *nx_string_builder_create(void);
void             nx_string_builder_destroy(NXStringBuilder *sb);
void             nx_string_builder_append(NXStringBuilder *sb, const char *str);
void             nx_string_builder_append_char(NXStringBuilder *sb, char c);
const char      *nx_string_builder_to_cstring(NXStringBuilder *sb);
void             nx_string_builder_clear(NXStringBuilder *sb);
/* }}} */

/* Command Runner {{{ */
typedef struct {
    NXStringBuilder *command;
    char            *output;
    int              exit_code;
    int              capture_output;
} NXCR;

NXCR       *nx_cr_create(void);
void        nx_cr_destroy(NXCR *cr);
void        nx_cr_append(NXCR *cr, const char *str);
int         nx_cr_execute(NXCR *cr);
const char *nx_cr_get_output(NXCR *cr);
int         nx_cr_get_exit_code(NXCR *cr);
int         nx_cr_run(const char *command);
/* }}} */

/* Command Runner (Build) {{{ */
int nx_rebuild(const char *source_file, int argc, char **argv);
int nx_compile_command(const char *description, const char **args, int arg_count,
                       bool enable_warnings);

#define NX_REBUILD(argc, argv)                                                                     \
    do {                                                                                           \
        if (nx_rebuild(__FILE__, (argc), (argv)) != 0) {                                           \
            exit(EXIT_FAILURE);                                                                    \
        }                                                                                          \
    } while (0)

#define nx_cr_enable_gcc_warnings(cr)                                                              \
    nx_cr_append(cr, "-Wall "                                                                      \
                     "-Wextra "                                                                    \
                     "-Wpedantic "                                                                 \
                     "-Wshadow "                                                                   \
                     "-Wpointer-arith "                                                            \
                     "-Wcast-qual "                                                                \
                     "-Wno-unused-parameter "                                                      \
                     "-fstack-protector-strong "                                                   \
                     "-Wswitch-default "                                                           \
                     "-Wstrict-prototypes "                                                        \
                     "-Wmissing-prototypes "                                                       \
                     "-Wmissing-declarations "                                                     \
                     "-Wredundant-decls "                                                          \
                     "-Wconversion "                                                               \
                     "-Wsign-conversion")
/* }}} */

/* File IO {{{ */
typedef struct {
    char       *filename;
    const char *mode;
    FILE       *file;
} NXFile;

NXFile *nx_file_open(const char *filename, const char *mode);
void    nx_file_close(NXFile *nx_file);
char   *nx_file_read_all(const char *filename);
int     nx_file_write_all(const char *filename, const char *data);
int     nx_file_exists(const char *filename);
long    nx_file_size(const char *filename);
/* }}} */

/* Logging {{{ */
typedef enum {
    NX_LOG_TRACE = 0,
    NX_LOG_DEBUG,
    NX_LOG_INFO,
    NX_LOG_WARN,
    NX_LOG_ERROR,
    NX_LOG_FATAL
} NXLogLevel;

typedef struct {
    NXFile          *file;
    bool             use_stdout;
    bool             use_timestamps;
    NXLogLevel       min_level;
    NXStringBuilder *buffer;
} NXLogger;

const char *NX_LOG_LEVEL_STRINGS[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

const char *NX_LOG_LEVEL_COLORS[] = {
    COLOR_BRIGHT_BLACK, /* TRACE */
    COLOR_CYAN,         /* DEBUG */
    COLOR_GREEN,        /* INFO */
    COLOR_YELLOW,       /* WARN */
    COLOR_RED,          /* ERROR */
    COLOR_MAGENTA       /* FATAL */
};

NXLogger *nx_logger_create(const char *filename, bool use_stdout, bool use_timestamps,
                           NXLogLevel min_level);
void      nx_logger_destroy(NXLogger *logger);
void      nx_logger_log(NXLogger *logger, NXLogLevel level, const char *format, ...);

#define nx_logger_trace(l, f) nx_logger_log(l, NX_LOG_TRACE, f)
#define nx_logger_trace1(l, f, a1) nx_logger_log(l, NX_LOG_TRACE, f, a1)
#define nx_logger_trace2(l, f, a1, a2) nx_logger_log(l, NX_LOG_TRACE, f, a1, a2)
#define nx_logger_trace3(l, f, a1, a2, a3) nx_logger_log(l, NX_LOG_TRACE, f, a1, a2, a3)

#define nx_logger_debug(l, f) nx_logger_log(l, NX_LOG_DEBUG, f)
#define nx_logger_debug1(l, f, a1) nx_logger_log(l, NX_LOG_DEBUG, f, a1)
#define nx_logger_debug2(l, f, a1, a2) nx_logger_log(l, NX_LOG_DEBUG, f, a1, a2)
#define nx_logger_debug3(l, f, a1, a2, a3) nx_logger_log(l, NX_LOG_DEBUG, f, a1, a2, a3)

#define nx_logger_info(l, f) nx_logger_log(l, NX_LOG_INFO, f)
#define nx_logger_info1(l, f, a1) nx_logger_log(l, NX_LOG_INFO, f, a1)
#define nx_logger_info2(l, f, a1, a2) nx_logger_log(l, NX_LOG_INFO, f, a1, a2)
#define nx_logger_info3(l, f, a1, a2, a3) nx_logger_log(l, NX_LOG_INFO, f, a1, a2, a3)

#define nx_logger_warn(l, f) nx_logger_log(l, NX_LOG_WARN, f)
#define nx_logger_warn1(l, f, a1) nx_logger_log(l, NX_LOG_WARN, f, a1)
#define nx_logger_warn2(l, f, a1, a2) nx_logger_log(l, NX_LOG_WARN, f, a1, a2)
#define nx_logger_warn3(l, f, a1, a2, a3) nx_logger_log(l, NX_LOG_WARN, f, a1, a2, a3)

#define nx_logger_error(l, f) nx_logger_log(l, NX_LOG_ERROR, f)
#define nx_logger_error1(l, f, a1) nx_logger_log(l, NX_LOG_ERROR, f, a1)
#define nx_logger_error2(l, f, a1, a2) nx_logger_log(l, NX_LOG_ERROR, f, a1, a2)
#define nx_logger_error3(l, f, a1, a2, a3) nx_logger_log(l, NX_LOG_ERROR, f, a1, a2, a3)

#define nx_logger_fatal(l, f) nx_logger_log(l, NX_LOG_FATAL, f)
#define nx_logger_fatal1(l, f, a1) nx_logger_log(l, NX_LOG_FATAL, f, a1)
#define nx_logger_fatal2(l, f, a1, a2) nx_logger_log(l, NX_LOG_FATAL, f, a1, a2)
#define nx_logger_fatal3(l, f, a1, a2, a3) nx_logger_log(l, NX_LOG_FATAL, f, a1, a2, a3)
/* }}} */

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* NEXUS_H */

#ifdef NEXUS_IMPLEMENTATION

/* Memory Tracker {{{ */
#ifdef NX_DEBUG

typedef struct NXMemoryBlock {
    void                 *ptr;
    size_t                size;
    const char           *file;
    int                   line;
    struct NXMemoryBlock *next;
} NXMemoryBlock;

static NXMemoryBlock *memoryList = NULL;

void *nx_malloc_debug(size_t size, const char *file, int line) {
    void *ptr = malloc(size);
    if (ptr) {
        NXMemoryBlock *block = (NXMemoryBlock *) malloc(sizeof(NXMemoryBlock));
        block->ptr           = ptr;
        block->size          = size;
        block->file          = file;
        block->line          = line;
        block->next          = memoryList;
        memoryList           = block;
    }
    return ptr;
}

void *nx_calloc_debug(size_t num, size_t size, const char *file, int line) {
    void *ptr = calloc(num, size);
    if (ptr) {
        NXMemoryBlock *block = (NXMemoryBlock *) malloc(sizeof(NXMemoryBlock));
        block->ptr           = ptr;
        block->size          = num * size;
        block->file          = file;
        block->line          = line;
        block->next          = memoryList;
        memoryList           = block;
    }
    return ptr;
}

void *nx_realloc_debug(void *ptr, size_t size, const char *file, int line) {
    void *newPtr = realloc(ptr, size);
    if (newPtr) {
        if (ptr == NULL) {
            NXMemoryBlock *block = (NXMemoryBlock *) malloc(sizeof(NXMemoryBlock));
            block->ptr           = newPtr;
            block->size          = size;
            block->file          = file;
            block->line          = line;
            block->next          = memoryList;
            memoryList           = block;
        } else {
            NXMemoryBlock  *block;
            NXMemoryBlock **current = &memoryList;
            while (*current) {
                if ((*current)->ptr == ptr) {
                    NXMemoryBlock *temp = *current;
                    *current            = (*current)->next;
                    free(temp);
                    break;
                }
                current = &(*current)->next;
            }

            block       = (NXMemoryBlock *) malloc(sizeof(NXMemoryBlock));
            block->ptr  = newPtr;
            block->size = size;
            block->file = file;
            block->line = line;
            block->next = memoryList;
            memoryList  = block;
        }
    }
    return newPtr;
}

void nx_free_debug(void *ptr) {
    if (ptr) {
        NXMemoryBlock **block = &memoryList;
        while (*block) {
            if ((*block)->ptr == ptr) {
                NXMemoryBlock *temp = *block;
                *block              = (*block)->next;
                free(temp);
                break;
            }
            block = &(*block)->next;
        }
        free(ptr);
    }
}

void nx_print_memory_leaks(void) {
    if (memoryList) {
        NXMemoryBlock *block = memoryList;
        while (block) {
            fprintf(stderr,
                    "Leaked memory at address %p, size %lu bytes, "
                    "allocated at %s:%d\n",
                    block->ptr, block->size, block->file, block->line);
            block = block->next;
        }
    } else {
        fprintf(stderr, "No memory leaks detected.\n");
    }
}
#endif /* NX_DEBUG */
/* }}}} */

/* Error Handling {{{ */
void nx_die_impl(const char *file, int line, const char *fmt, ...) {
    va_list args;
    size_t  fmt_len;

    fprintf(stderr, "Fatal Error in %s:%d: ", file, line);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fmt_len = strlen(fmt);
    if (fmt_len > 0 && fmt[fmt_len - 1] == ':') {
        fputc(' ', stderr);
        perror(NULL);
    } else {
        fputc('\n', stderr);
    }

    nx_print_memory_leaks();

    exit(EXIT_FAILURE);
}
/* }}} */

/* Arena {{{ */
NXArena *nx_arena_create(void) {
    NXArena      *arena;
    NXArenaBlock *block;

    arena = malloc(sizeof(NXArena));
    if (!arena) {
        return NULL;
    }

    block = malloc(sizeof(NXArenaBlock));
    if (!block) {
        free(arena);
        return NULL;
    }

    block->memory = malloc(NX_ARENA_BLOCK_SIZE);
    if (!block->memory) {
        free(block);
        free(arena);
        return NULL;
    }

    block->used = 0;
    block->size = NX_ARENA_BLOCK_SIZE;
    block->next = NULL;

    arena->first   = block;
    arena->current = block;

    return arena;
}

void *nx_arena_alloc(NXArena *arena, size_t size) {
    NXArenaBlock *current;
    void         *ptr;

    /* Ensure 8-byte alignment using unsigned arithmetic */
    size = (size + 7U) & ~7U;

    current = arena->current;
    if (current->used + size > current->size) {
        size_t        new_block_size;
        NXArenaBlock *new_block;

        new_block_size = size > NX_ARENA_BLOCK_SIZE ? size : NX_ARENA_BLOCK_SIZE;

        new_block = malloc(sizeof(NXArenaBlock));
        if (!new_block) {
            return NULL;
        }

        new_block->memory = malloc(new_block_size);
        if (!new_block->memory) {
            free(new_block);
            return NULL;
        }

        new_block->used = 0;
        new_block->size = new_block_size;
        new_block->next = NULL;

        current->next  = new_block;
        arena->current = new_block;
        current        = new_block;
    }

    ptr = (char *) current->memory + current->used;
    current->used += size;

    return ptr;
}

void nx_arena_reset(NXArena *arena) {
    NXArenaBlock *block = arena->first;
    while (block) {
        block->used = 0;
        block       = block->next;
    }
    arena->current = arena->first;
}

void nx_arena_destroy(NXArena *arena) {
    NXArenaBlock *block = arena->first;
    while (block) {
        NXArenaBlock *next = block->next;
        free(block->memory);
        free(block);
        block = next;
    }
    free(arena);
}
/* }}} */

/* Linked Lists {{{ */
NXSinglyLinkedList *nx_sll_create(void) {
    NXSinglyLinkedList *list = (NXSinglyLinkedList *) nx_malloc(sizeof(NXSinglyLinkedList));
    if (!list) {
        return NULL;
    }
    list->head = list->tail = NULL;
    return list;
}

void nx_sll_destroy(NXSinglyLinkedList *list) {
    NXSLLNode *current = list->head;
    while (current) {
        NXSLLNode *next = current->next;
        nx_free(current);
        current = next;
    }
    nx_free(list);
}

void nx_sll_append(NXSinglyLinkedList *list, void *data) {
    NXSLLNode *new_node = (NXSLLNode *) nx_malloc(sizeof(NXSLLNode));
    if (!new_node) {
        return;
    }

    new_node->data = data;
    new_node->next = NULL;

    if (list->tail) {
        list->tail->next = new_node;
    } else {
        list->head = new_node;
    }

    list->tail = new_node;
}

void nx_sll_prepend(NXSinglyLinkedList *list, void *data) {
    NXSLLNode *new_node = (NXSLLNode *) nx_malloc(sizeof(NXSLLNode));
    if (!new_node) {
        return;
    }

    new_node->data = data;
    new_node->next = list->head;

    list->head = new_node;

    if (!list->tail) {
        list->tail = new_node;
    }
}

void nx_sll_remove(NXSinglyLinkedList *list, void *data) {
    NXSLLNode *current  = list->head;
    NXSLLNode *previous = NULL;

    while (current) {
        if (current->data == data) {
            if (previous) {
                previous->next = current->next;
            } else {
                list->head = current->next;
            }

            if (current == list->tail) {
                list->tail = previous;
            }

            nx_free(current);
            return;
        }

        previous = current;
        current  = current->next;
    }
}

NXDoublyLinkedList *nx_dll_create(void) {
    NXDoublyLinkedList *list = (NXDoublyLinkedList *) nx_malloc(sizeof(NXDoublyLinkedList));
    if (!list) {
        return NULL;
    }
    list->head = list->tail = NULL;
    return list;
}

void nx_dll_destroy(NXDoublyLinkedList *list) {
    NXDLLNode *current = list->head;
    while (current) {
        NXDLLNode *next = current->next;
        nx_free(current);
        current = next;
    }
    nx_free(list);
}

void nx_dll_append(NXDoublyLinkedList *list, void *data) {
    NXDLLNode *new_node = (NXDLLNode *) nx_malloc(sizeof(NXDLLNode));
    if (!new_node) {
        return;
    }

    new_node->data = data;
    new_node->next = NULL;
    new_node->prev = list->tail;

    if (list->tail) {
        list->tail->next = new_node;
    } else {
        list->head = new_node;
    }

    list->tail = new_node;
}

void nx_dll_prepend(NXDoublyLinkedList *list, void *data) {
    NXDLLNode *new_node = (NXDLLNode *) nx_malloc(sizeof(NXDLLNode));
    if (!new_node)
        return;

    new_node->data = data;
    new_node->next = list->head;
    new_node->prev = NULL;

    if (list->head) {
        list->head->prev = new_node;
    } else {
        list->tail = new_node;
    }

    list->head = new_node;
}

void nx_dll_remove(NXDoublyLinkedList *list, void *data) {
    NXDLLNode *current = list->head;

    while (current) {
        if (current->data == data) {
            if (current->prev) {
                current->prev->next = current->next;
            } else {
                list->head = current->next;
            }

            if (current->next) {
                current->next->prev = current->prev;
            } else {
                list->tail = current->prev;
            }

            nx_free(current);
            return;
        }

        current = current->next;
    }
}
/* }}} */

/* Hashmap {{{ */
static void _nx_hashmap_resize(NXHashMap *map) {
    size_t           i;
    size_t           old_capacity = map->capacity;
    NXHashMapEntry **old_buckets  = map->buckets;

    map->capacity *= 2;
    map->buckets = (NXHashMapEntry **) nx_calloc(map->capacity, sizeof(NXHashMapEntry *));

    for (i = 0; i < old_capacity; i++) {
        NXHashMapEntry *entry = old_buckets[i];
        while (entry) {
            NXHashMapEntry *next      = entry->next;
            size_t          new_index = map->hash_func(entry->key) % map->capacity;
            entry->next               = map->buckets[new_index];
            map->buckets[new_index]   = entry;
            entry                     = next;
        }
    }

    nx_free(old_buckets);
}

static size_t _nx_default_hash(void *key) {
    const char *str  = (const char *) key;
    size_t      hash = 0;
    while (*str) {
        hash = 31 * hash + (unsigned char) (*str++);
    }
    return hash;
}

static bool _nx_default_key_equal(void *key1, void *key2) {
    return strcmp((const char *) key1, (const char *) key2) == 0;
}

NXHashMap *nx_hashmap_create(size_t (*hash_func)(void *key),
                             bool (*key_equal)(void *key1, void *key2)) {
    NXHashMap *map = (NXHashMap *) nx_malloc(sizeof(NXHashMap));
    if (!map)
        return NULL;

    map->capacity  = NX_HASHMAP_INITIAL_CAPACITY;
    map->size      = 0;
    map->hash_func = hash_func ? hash_func : _nx_default_hash;
    map->key_equal = key_equal ? key_equal : _nx_default_key_equal;
    map->buckets   = (NXHashMapEntry **) nx_calloc(map->capacity, sizeof(NXHashMapEntry *));
    if (!map->buckets) {
        nx_free(map);
        return NULL;
    }
    return map;
}

void nx_hashmap_destroy(NXHashMap *map) {
    size_t i;
    for (i = 0; i < map->capacity; i++) {
        NXHashMapEntry *entry = map->buckets[i];
        while (entry) {
            NXHashMapEntry *next = entry->next;
            nx_free(entry);
            entry = next;
        }
    }
    nx_free(map->buckets);
    nx_free(map);
}

bool nx_hashmap_insert(NXHashMap *map, void *key, void *value) {
    /* Declare variables before any executable statements */
    size_t          index = map->hash_func(key) % map->capacity;
    NXHashMapEntry *entry = map->buckets[index];
    NXHashMapEntry *new_entry;

    while (entry) {
        if (map->key_equal(entry->key, key)) {
            entry->value = value;
            return true;
        }
        entry = entry->next;
    }

    new_entry = (NXHashMapEntry *) nx_malloc(sizeof(NXHashMapEntry));
    if (!new_entry) {
        return false;
    }
    new_entry->key      = key;
    new_entry->value    = value;
    new_entry->next     = map->buckets[index];
    map->buckets[index] = new_entry;

    map->size++;

    /* Fixing the conversion warning by explicitly casting to float */
    if ((float) map->size / (float) map->capacity > NX_HASHMAP_LOAD_FACTOR) {
        _nx_hashmap_resize(map);
    }
    return true;
}

void *nx_hashmap_get(NXHashMap *map, void *key) {
    size_t          index = map->hash_func(key) % map->capacity;
    NXHashMapEntry *entry = map->buckets[index];

    while (entry) {
        if (map->key_equal(entry->key, key)) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

bool nx_hashmap_remove(NXHashMap *map, void *key) {
    size_t          index = map->hash_func(key) % map->capacity;
    NXHashMapEntry *entry = map->buckets[index];
    NXHashMapEntry *prev  = NULL;

    while (entry) {
        if (map->key_equal(entry->key, key)) {
            if (prev) {
                prev->next = entry->next;
            } else {
                map->buckets[index] = entry->next;
            }
            nx_free(entry);
            map->size--;
            return true;
        }
        prev  = entry;
        entry = entry->next;
    }
    return false;
}
/* }}} */

/* String Builder {{{ */
static void _nx_string_builder_resize(NXStringBuilder *sb, size_t new_capacity) {
    if (new_capacity <= sb->capacity) {
        return;
    }
    sb->buffer = (char *) nx_realloc(sb->buffer, new_capacity);
    if (sb->buffer) {
        sb->capacity = new_capacity;
    } else {
        nx_die("Failed to allocate memory for string builder");
    }
}

NXStringBuilder *nx_string_builder_create(void) {
    NXStringBuilder *sb = (NXStringBuilder *) nx_malloc(sizeof(NXStringBuilder));
    if (!sb) {
        return NULL;
    }
    sb->capacity = NX_STRING_BUILDER_INITIAL_CAPACITY;
    sb->length   = 0;
    sb->buffer   = (char *) nx_malloc(sb->capacity);
    if (!sb->buffer) {
        nx_free(sb);
        return NULL;
    }
    sb->buffer[0] = '\0';
    return sb;
}

void nx_string_builder_destroy(NXStringBuilder *sb) {
    if (sb) {
        nx_free(sb->buffer);
        nx_free(sb);
    }
}

void nx_string_builder_append(NXStringBuilder *sb, const char *str) {
    size_t str_len = strlen(str);
    if (sb->length + str_len >= sb->capacity) {
        size_t new_capacity = sb->capacity * NX_STRING_BUILDER_GROWTH_FACTOR;
        while (sb->length + str_len >= new_capacity) {
            new_capacity *= NX_STRING_BUILDER_GROWTH_FACTOR;
        }
        _nx_string_builder_resize(sb, new_capacity);
    }
    memcpy(sb->buffer + sb->length, str, str_len + 1);
    sb->length += str_len;
}

void nx_string_builder_append_char(NXStringBuilder *sb, char c) {
    if (sb->length + 1 >= sb->capacity) {
        _nx_string_builder_resize(sb, sb->capacity * NX_STRING_BUILDER_GROWTH_FACTOR);
    }
    sb->buffer[sb->length]     = c;
    sb->buffer[sb->length + 1] = '\0';
    sb->length++;
}

const char *nx_string_builder_to_cstring(NXStringBuilder *sb) {
    return sb->buffer;
}

void nx_string_builder_clear(NXStringBuilder *sb) {
    sb->length    = 0;
    sb->buffer[0] = '\0';
}
/* }}} */

/* Command Runner {{{ */
NXCR *nx_cr_create(void) {
    NXCR *cr = (NXCR *) nx_malloc(sizeof(NXCR));
    if (!cr) {
        return NULL;
    }

    cr->command = nx_string_builder_create();
    if (!cr->command) {
        nx_free(cr);
        return NULL;
    }

    cr->output    = NULL;
    cr->exit_code = 0;
    return cr;
}

void nx_cr_destroy(NXCR *cr) {
    if (cr) {
        nx_string_builder_destroy(cr->command);
        nx_free(cr->output);
        nx_free(cr);
    }
}

void nx_cr_append(NXCR *cr, const char *str) {
    nx_string_builder_append(cr->command, str);
    nx_string_builder_append(cr->command, " ");
}

int nx_cr_execute(NXCR *cr) {
    int              pipefd[2];
    pid_t            pid;
    int              status;
    char             buffer[4096];
    ssize_t          bytes_read;
    NXStringBuilder *output_sb;
    const char      *temp_output;

    nx_free(cr->output);
    cr->output = NULL;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        close(pipefd[0]);
        if (dup2(pipefd[1], STDOUT_FILENO) == -1 || dup2(pipefd[1], STDERR_FILENO) == -1) {
            perror("dup2");
            exit(1);
        }
        close(pipefd[1]);

        execlp("/bin/sh", "sh", "-c", nx_string_builder_to_cstring(cr->command), NULL);
        perror("execlp");
        exit(1);
    }

    close(pipefd[1]);
    output_sb = nx_string_builder_create();

    while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        fprintf(stdout, "%s", buffer);
        fflush(stdout);
        nx_string_builder_append(output_sb, buffer);
    }

    close(pipefd[0]);

    if (output_sb->length > 0) {
        temp_output = nx_string_builder_to_cstring(output_sb);
        cr->output  = (char *) nx_malloc(strlen(temp_output) + 1);
        if (cr->output) {
            strcpy(cr->output, temp_output);
        }
    }

    nx_string_builder_destroy(output_sb);

    waitpid(pid, &status, 0);
    cr->exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    nx_string_builder_clear(cr->command);

    return cr->exit_code;
}

const char *nx_cr_get_output(NXCR *cr) {
    return cr->output ? cr->output : "";
}

int nx_cr_get_exit_code(NXCR *cr) {
    return cr->exit_code;
}

int nx_cr_run(const char *command) {
    NXCR *cr;
    int   result;

    cr = nx_cr_create();
    if (!cr) {
        fprintf(stderr, "Failed to create NXCR instance.\n");
        return EXIT_FAILURE;
    }

    nx_string_builder_append(cr->command, command);
    result = nx_cr_execute(cr);
    nx_cr_destroy(cr);
    return result;
}
/* }}} */

/* Command Runner (Build) {{{ */
static const char *_nx_extract_basename(const char *filepath) {
    const char *slash = strrchr(filepath, '/');
    if (slash) {
        return slash + 1;
    }
    return filepath;
}

static void _nx_remove_extension(char *filename) {
    char *dot = strrchr(filename, '.');
    if (dot) {
        *dot = '\0';
    }
}

int nx_compile_command(const char *description, const char **args, int arg_count,
                       bool enable_warnings) {
    NXCR *cr;
    int   i;
    int   result;

    cr = nx_cr_create();
    if (!cr) {
        fprintf(stderr, "Failed to create NXCR instance for %s.\n", description);
        return EXIT_FAILURE;
    }

    for (i = 0; i < arg_count; ++i) {
        nx_cr_append(cr, args[i]);
    }

    if (enable_warnings) {
        nx_cr_enable_gcc_warnings(cr);
    }

    result = nx_cr_execute(cr);
    if (result != 0) {
        printf("%sCompilation of %s failed.%s\n", COLOR_RED, description, COLOR_RESET);
    } else {
        printf("%sCompilation of %s succeeded.%s\n", COLOR_GREEN, description, COLOR_RESET);
    }

    nx_cr_destroy(cr);
    return result;
}

int nx_rebuild(const char *source_file, int argc, char **argv) {
    const char *basename_with_ext;
    char        basename[256];
    const char *output_executable;
    struct stat src_stat;
    struct stat exe_stat;
    int         need_build;
    const char *args[8];
    int         compile_result;
    char      **new_argv;
    int         i;

    basename_with_ext = _nx_extract_basename(source_file);
    if (strlen(basename_with_ext) >= sizeof(basename)) {
        fprintf(stderr, "Error: Source file basename is too long.\n");
        return -1;
    }
    strncpy(basename, basename_with_ext, sizeof(basename) - 1);
    basename[sizeof(basename) - 1] = '\0';
    _nx_remove_extension(basename);

    output_executable = basename;
    need_build        = 0;

    if (stat(source_file, &src_stat) != 0) {
        perror("stat source_file");
        return -1;
    }
    if (stat(output_executable, &exe_stat) != 0) {
        need_build = 1;
    } else if (src_stat.st_mtime > exe_stat.st_mtime) {
        need_build = 1;
    }

    if (need_build) {
        printf("%sRebuilding %s due to changes in %s.%s\n", COLOR_YELLOW, output_executable,
               source_file, COLOR_RESET);

        args[0] = "cc";
        args[1] = source_file;
        args[2] = "-o";
        args[3] = output_executable;
        args[4] = "-Wall";
        args[5] = "-Wextra";
        args[6] = "-fdiagnostics-color=always";
        args[7] = "-O2";

        compile_result = nx_compile_command(output_executable, args, nx_len(args), 1);
        if (compile_result != 0) {
            return compile_result;
        }

        new_argv = (char **) nx_malloc((size_t) (argc + 1) * sizeof(char *));
        if (new_argv == NULL) {
            perror("malloc");
            return -1;
        }

        for (i = 0; i < argc; i++) {
            new_argv[i] = argv[i];
        }
        new_argv[argc] = NULL;

        execv(output_executable, new_argv);
        perror("execv");
        nx_free(new_argv);
        return -1;
    }

    return 0;
}
/* }}} */

/* File IO {{{ */
NXFile *nx_file_open(const char *filename, const char *mode) {
    size_t  len;
    NXFile *nx_file = (NXFile *) nx_malloc(sizeof(NXFile));
    if (!nx_file) {
        return NULL;
    }
    len               = strlen(filename) + 1;
    nx_file->filename = (char *) nx_malloc(len);
    if (!nx_file->filename) {
        nx_free(nx_file);
        return NULL;
    }
    memcpy(nx_file->filename, filename, len);
    nx_file->mode = mode;
    nx_file->file = fopen(filename, mode);
    if (!nx_file->file) {
        nx_free(nx_file->filename);
        nx_free(nx_file);
        return NULL;
    }
    return nx_file;
}

void nx_file_close(NXFile *nx_file) {
    if (nx_file) {
        if (nx_file->file) {
            fclose(nx_file->file);
        }
        nx_free(nx_file->filename);
        nx_free(nx_file);
    }
}

char *nx_file_read_all(const char *filename) {
    FILE *file = fopen(filename, "rb");
    char *buffer;
    long  length;
    if (!file) {
        return NULL;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    length = ftell(file);
    if (length == -1L) {
        fclose(file);
        return NULL;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    buffer = (char *) nx_malloc((size_t) length + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    if (fread(buffer, 1, (size_t) length, file) != (size_t) length) {
        nx_free(buffer);
        fclose(file);
        return NULL;
    }
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

int nx_file_write_all(const char *filename, const char *data) {
    FILE  *file = fopen(filename, "wb");
    size_t data_len;
    if (!file) {
        return -1;
    }
    data_len = strlen(data);
    if (fwrite(data, 1, data_len, file) != data_len) {
        fclose(file);
        return -1;
    }
    fclose(file);
    return 0;
}

int nx_file_exists(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

long nx_file_size(const char *filename) {
    FILE *file = fopen(filename, "rb");
    long  size;
    if (!file) {
        return -1;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return -1;
    }
    size = ftell(file);
    fclose(file);
    return size;
}
/* }}} */

/* Logging {{{ */
static void _nx_logger_format_message(NXLogger *logger, NXLogLevel level, const char *format,
                                      va_list args) {
    time_t current_time;
    char   timestamp[32];

    nx_string_builder_clear(logger->buffer);

    /* Add timestamp if enabled */
    if (logger->use_timestamps) {
        current_time = time(NULL);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&current_time));
        nx_string_builder_append(logger->buffer, "[");
        nx_string_builder_append(logger->buffer, timestamp);
        nx_string_builder_append(logger->buffer, "] ");
    }

    /* Add log level */
    nx_string_builder_append(logger->buffer, "[");
    nx_string_builder_append(logger->buffer, NX_LOG_LEVEL_STRINGS[level]);
    nx_string_builder_append(logger->buffer, "] ");

    /* Format the actual message */
    {
        char formatted_message[4096];
        nx_vsnprintf(formatted_message, sizeof(formatted_message), format, args);
        nx_string_builder_append(logger->buffer, formatted_message);
    }

    nx_string_builder_append(logger->buffer, "\n");
}

static void _nx_logger_write(NXLogger *logger, NXLogLevel level) {
    const char *message = nx_string_builder_to_cstring(logger->buffer);

    if (logger->use_stdout) {
        fprintf(stdout, "%s%s%s", NX_LOG_LEVEL_COLORS[level], message, COLOR_RESET);
        fflush(stdout);
    }

    if (logger->file) {
        fprintf(logger->file->file, "%s", message);
        fflush(logger->file->file);
    }
}

NXLogger *nx_logger_create(const char *filename, bool use_stdout, bool use_timestamps,
                           NXLogLevel min_level) {
    NXLogger *logger = (NXLogger *) nx_malloc(sizeof(NXLogger));
    if (!logger) {
        return NULL;
    }

    logger->buffer = nx_string_builder_create();
    if (!logger->buffer) {
        nx_free(logger);
        return NULL;
    }

    logger->file = NULL;
    if (filename) {
        logger->file = nx_file_open(filename, "a");
        if (!logger->file) {
            nx_string_builder_destroy(logger->buffer);
            nx_free(logger);
            return NULL;
        }
    }

    logger->use_stdout     = use_stdout;
    logger->use_timestamps = use_timestamps;
    logger->min_level      = min_level;

    return logger;
}

void nx_logger_destroy(NXLogger *logger) {
    if (logger) {
        if (logger->file) {
            nx_file_close(logger->file);
        }
        nx_string_builder_destroy(logger->buffer);
        nx_free(logger);
    }
}

void nx_logger_log(NXLogger *logger, NXLogLevel level, const char *format, ...) {
    if (level >= logger->min_level) {
        va_list args;
        va_start(args, format);
        _nx_logger_format_message(logger, level, format, args);
        va_end(args);
        _nx_logger_write(logger, level);
    }
}
/* }}} */

#endif /* NEXUS_IMPLEMENTATION */
