/* Copyright (C) 2024 Fragart
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* TODO: flowfield art */

#define NXUI_IMPLEMENTATION
#include "nxui.h"

#define NEXUS_IMPLEMENTATION
#define NEXUS_MATH
#include "nexus.h"

#include <GLFW/glfw3.h>
#include <getopt.h>

typedef struct {
    const char    *vertex_shader;
    const char    *fragment_shader;
    float         *vertices;
    size_t         vertex_size;
    NXUIAttribute *attributes;
    int            attribute_count;
    void (*loop)(GLFWwindow *window, NXUIShaderProgram *shader_prog);
} Art;

/* clang-format off */
static float vertex_data[] = {
    /* Vertex 1: Position (-1.0, -1.0), TexCoord (0.0, 0.0) */
    -1.0f, -1.0f, 0.0f, 0.0f,

    /* Vertex 2: Position ( 1.0, -1.0), TexCoord (1.0, 0.0) */
    1.0f, -1.0f, 1.0f, 0.0f,

    /* Vertex 3: Position ( 1.0,  1.0), TexCoord (1.0, 1.0) */
    -1.0f, 1.0f, 0.0f, 1.0f,

    /* Vertex 4: Position (-1.0,  1.0), TexCoord (0.0, 1.0) */
    -1.0f, 1.0f, 0.0f, 1.0f,

    /* Vertex 5: Position (-1.0, -1.0), TexCoord (0.0, 0.0) */
    1.0f, -1.0f, 1.0f, 0.0f,

    /* Vertex 6: Position ( 1.0,  1.0), TexCoord (1.0, 1.0) */
    1.0f, 1.0f, 1.0f, 1.0f
};
/* clang-format on */

#define NUM_POINTS 500

static float flowfield_vertex_data[NUM_POINTS * 4];

static void generate_flowfield_vertex_data(void) {
    int i;
    srand((unsigned int) time(NULL));
    for (i = 0; i < NUM_POINTS * 4; i += 4) {
        flowfield_vertex_data[i]     = ((float) rand() / (float) RAND_MAX) * 2.0f - 1.0f;
        flowfield_vertex_data[i + 1] = ((float) rand() / (float) RAND_MAX) * 2.0f - 1.0f;
        flowfield_vertex_data[i + 2] = (float) rand() / (float) RAND_MAX;
        flowfield_vertex_data[i + 3] = (float) rand() / (float) RAND_MAX;
    }
}

static NXUIAttribute attributes[] = {
    {0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0},
    {1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float))}};

static void res_time(GLFWwindow *window, NXUIShaderProgram *shader_prog) {
    int   width, height;
    float timeValue = (float) glfwGetTime();
    glfwGetFramebufferSize(window, &width, &height);
    nxui_set_uniform_float(shader_prog, "iTime", timeValue);
    nxui_set_uniform_vec2(shader_prog, "iResolution", (float) width, (float) height);
}

static void res(GLFWwindow *window, NXUIShaderProgram *shader_prog) {
    float timeValue = (float) glfwGetTime();
    (void) window;
    nxui_set_uniform_float(shader_prog, "iTime", timeValue);
}

static Art arts[] = {
    {
        "shaders/vertex.glsl",
        "shaders/mandelbrot.glsl",
        vertex_data,
        sizeof(vertex_data),
        attributes,
        2,
        res_time,
    },
    {
        "shaders/vertex.glsl",
        "shaders/julia.glsl",
        vertex_data,
        sizeof(vertex_data),
        attributes,
        2,
        res_time,
    },
    {
        "shaders/vertex.glsl",
        "shaders/julianoise.glsl",
        vertex_data,
        sizeof(vertex_data),
        attributes,
        2,
        res_time,
    },
    {
        "shaders/vertexflowfield.glsl",
        "shaders/flowfield.glsl",
        flowfield_vertex_data,
        sizeof(flowfield_vertex_data),
        attributes,
        2,
        res,
    },
};

static const char *get_art_name(const Art *art) {
    static char name[256];
    const char *dot;
    size_t      len;
    const char *slash = strrchr(art->fragment_shader, '/');
    if (!slash) {
        slash = art->fragment_shader;
    } else {
        slash++;
    }
    dot = strrchr(slash, '.');
    len = dot ? (size_t) (dot - slash) : strlen(slash);
    strncpy(name, slash, len);
    name[len] = '\0';
    return name;
}

static void print_help(const char *prog_name) {
    size_t    i;
    const int name_width = 15;

    /* clang-format off */
    printf(COLOR_GREEN "Fragart\n" COLOR_RESET);
    printf("A program for rendering mesmerizing shader-based art made by @xlc-dev using Nexus.\n\n");

    printf(COLOR_CYAN "Usage: " COLOR_RESET "%s [OPTIONS]\n", prog_name);
    printf(COLOR_CYAN "Options:\n" COLOR_RESET);
    printf("  " COLOR_GREEN "-h, --help      " COLOR_RESET "Display this help message and exit\n");
    printf("  " COLOR_GREEN "-a, --art <name> " COLOR_RESET "Select an art to render\n");
    printf("  " COLOR_GREEN "-r, --run-all   " COLOR_RESET "Run all available arts sequentially\n");

    printf("\n");

    printf(COLOR_YELLOW "Available art options:\n" COLOR_RESET);
    for (i = 0; i < sizeof(arts)/sizeof(arts[0]); i++) {
        printf("  " COLOR_BLUE "- %-*s " COLOR_RESET "Render the %s\n",
               name_width - 2, get_art_name(&arts[i]), get_art_name(&arts[i]));
    }
    /* clang-format on */
}

static void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    (void) window;
    glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    (void) scancode;
    (void) mods;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    (void) window;
    (void) button;
    (void) mods;

    if (action == GLFW_PRESS) {
    }
}

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
    (void) window;
    (void) xpos;
    (void) ypos;
}

static GLFWwindow *init_window(void) {
    GLFWwindow *window;
    if (!glfwInit()) {
        nx_die("Failed to initialize GLFW");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 600, "Fragart", NULL, NULL);
    if (!window) {
        glfwTerminate();
        nx_die("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        nx_die("Failed to initialize GLAD");
    }

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    return window;
}

int main(int argc, char *argv[]) {
    GLFWwindow       *window;
    NXUIContext      *nxui;
    NXUIShaderProgram shader_prog;
    NXUIMesh          mesh;
    Art              *selected_art = NULL;
    const char       *art_name     = NULL;
    int               opt;
    size_t            i;
    size_t            current_art;
    int               run_all     = 0;
    time_t            last_switch = time(NULL);

    struct option long_options[] = {{"help", no_argument, NULL, 'h'},
                                    {"art", required_argument, NULL, 'a'},
                                    {"run-all", no_argument, NULL, 'r'},
                                    {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, "ha:r", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                print_help(argv[0]);
                return EXIT_SUCCESS;
            case 'a':
                art_name = optarg;
                run_all  = 0;
                break;
            case 'r':
                run_all = 1;
                break;
            default:
                print_help(argv[0]);
                return EXIT_FAILURE;
        }
    }

    generate_flowfield_vertex_data();

    if (run_all && art_name) {
        fprintf(stderr,
                COLOR_RED "Error: Options '-a' and '-r' cannot be used together.\n" COLOR_RESET);
        print_help(argv[0]);
        return EXIT_FAILURE;
    }

    if (!run_all && !art_name) {
        print_help(argv[0]);
        return EXIT_FAILURE;
    }

    window = init_window();
    nxui   = nxui_context_init();

    if (run_all) {
        if (sizeof(arts) / sizeof(arts[0]) == 0) {
            fprintf(stderr, COLOR_RED "Error: No arts available to run.\n" COLOR_RESET);
            return EXIT_FAILURE;
        }
        selected_art = &arts[0];
        last_switch  = time(NULL);
        current_art  = 0;
    } else {
        for (i = 0; i < sizeof(arts) / sizeof(arts[0]); i++) {
            if (strcmp(art_name, get_art_name(&arts[i])) == 0) {
                selected_art = &arts[i];
                break;
            }
        }
        if (!selected_art) {
            fprintf(stderr, COLOR_RED "Error: Unknown art '%s'\n" COLOR_RESET, art_name);
            return EXIT_FAILURE;
        }
    }

    shader_prog = nxui_create_shader_program_from_files(selected_art->vertex_shader,
                                                        selected_art->fragment_shader);
    nxui_context_add_shader(nxui, shader_prog);

    mesh =
        nxui_create_mesh(selected_art->vertices, selected_art->vertex_size, NULL, 0,
                         selected_art->attribute_count, selected_art->attributes, GL_STATIC_DRAW);
    mesh.shader = &nxui->shaders[0];
    mesh.mode   = (nx_strcmp(selected_art->vertex_shader, "shaders/vertexflowfield.glsl") == 0)
                      ? GL_POINTS
                      : GL_TRIANGLES;
    nxui_context_add_mesh(nxui, mesh);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (run_all) {
            time_t now = time(NULL);
            if (difftime(now, last_switch) >= 30.0) {
                current_art++;
                if (current_art >= sizeof(arts) / sizeof(arts[0])) {
                    break;
                }
                selected_art = &arts[current_art];
                last_switch  = now;

                nxui_context_destroy(nxui);
                nxui = nxui_context_init();

                shader_prog = nxui_create_shader_program_from_files(selected_art->vertex_shader,
                                                                    selected_art->fragment_shader);
                nxui_context_add_shader(nxui, shader_prog);

                mesh = nxui_create_mesh(selected_art->vertices, selected_art->vertex_size, NULL, 0,
                                        selected_art->attribute_count, selected_art->attributes,
                                        GL_STATIC_DRAW);
                mesh.shader = &nxui->shaders[0];
                mesh.mode =
                    (nx_strcmp(selected_art->vertex_shader, "shaders/vertexflowfield.glsl") == 0)
                        ? GL_POINTS
                        : GL_TRIANGLES;
                nxui_context_add_mesh(nxui, mesh);
            }
        }

        nxui_clear(0.078f, 0.078f, 0.117f, 1.0f);
        selected_art->loop(window, &shader_prog);
        nxui_render_ui(nxui);
        glfwSwapBuffers(window);
    }

    nxui_context_destroy(nxui);
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
