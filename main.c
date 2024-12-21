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
    void (*loop)(NXUIContext *nxui, NXUIMesh *mesh, NXUIShaderProgram *shader_prog);
} Art;

static void print_help(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("Options:\n");
    printf("  -h, --help       Display this help message\n");
    printf("  -a, --art        Select an art to render\n");
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

static void art_one_loop(NXUIContext *nxui, NXUIMesh *mesh, NXUIShaderProgram *shader_prog) {
    (void) mesh;
    float timeValue = (float) glfwGetTime();
    nxui_set_uniform_float(shader_prog, "iTime", timeValue);
    nxui_render_ui(nxui);
}

static float         vertex_data1[] = {-1.0f, -1.0f, 0.0f, 0.0f, 1.0f,  -1.0f, 1.0f, 0.0f,
                                       -1.0f, 1.0f,  0.0f, 1.0f, -1.0f, 1.0f,  0.0f, 1.0f,
                                       1.0f,  -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  1.0f, 1.0f};
static NXUIAttribute attributes1[]  = {
    {0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0},
    {1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float))}};

static Art arts[] = {
    {
        "shaders/vertart1.glsl",
        "shaders/fragart1.glsl",
        vertex_data1,
        24 * sizeof(float),
        attributes1,
        2,
        art_one_loop,
    },
};

int main(int argc, char *argv[]) {
    GLFWwindow       *window;
    NXUIContext      *nxui;
    NXUIShaderProgram shader_prog;
    NXUIMesh          mesh;
    Art              *selected_art = &arts[0];
    const char       *art_name     = "art_one";
    int               opt;
    size_t            i;

    struct option long_options[] = {
        {"help", no_argument, NULL, 'h'}, {"art", required_argument, NULL, 'a'}, {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, "ha:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                print_help(argv[0]);
                return EXIT_SUCCESS;
            case 'a':
                art_name = optarg;
                break;
            default:
                print_help(argv[0]);
                return EXIT_FAILURE;
        }
    }

    for (i = 0; i < nx_len(arts); i++) {
        if (strcmp(art_name, arts[i].vertex_shader) == 0) {
            selected_art = &arts[i];
            break;
        }
    }

    window = init_window();
    nxui   = nxui_context_init();

    shader_prog = nxui_create_shader_program_from_files(selected_art->vertex_shader,
                                                        selected_art->fragment_shader);
    nxui_context_add_shader(nxui, shader_prog);

    mesh =
        nxui_create_mesh(selected_art->vertices, selected_art->vertex_size, NULL, 0,
                         selected_art->attribute_count, selected_art->attributes, GL_STATIC_DRAW);
    mesh.shader = &nxui->shaders[0];
    nxui_context_add_mesh(nxui, mesh);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.078f, 0.078f, 0.117f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        selected_art->loop(nxui, &mesh, &shader_prog);

        glfwSwapBuffers(window);
    }

    nxui_context_destroy(nxui);
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
