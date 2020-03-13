#include <GLFW/glfw3.h>
#include <stdio.h>

#include "../../thirdparty/nanovg/src/nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "../../thirdparty/nanovg/src/nanovg_gl.h"

#include "../core/node.h"
#include "../core/utils.h"

extern struct node **nodes;
extern unsigned int nodes_pointer;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    printf("key_callback: %s %d %d %d %d\n", 
        glfwGetKeyName(key, scancode), 
        key, scancode, action, mods);
    switch (key)
    {
    case GLFW_KEY_Q:
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void cursor_pos_callback(GLFWwindow *window, double x, double y)
{
    printf("cursor_pos_callback: %d %d\n", (int)x, (int)y);
}

// TODO: x and y will be in node
void draw_node(struct NVGcontext *vg, struct node *node) 
{
    const int pin_size = 10;
    const int pin_padding = 5;

    float width = pin_padding + (pin_size + pin_padding) * 16;
    const float height = 45;

    int x = node->x;
    int y = node->y;

    nvgBeginPath(vg);
    nvgRect(vg, x, y, width, height);
    nvgFillColor(vg, nvgRGBA(255, 192, 0, 255));
    nvgFill(vg);
    nvgStroke(vg);

    nvgFillColor(vg, nvgRGBA(0, 0, 0, 128));
    for (int i = 0; i < 16; i++)
    {
        nvgBeginPath(vg);
        nvgRect(vg, 
            x + pin_padding + (pin_size + pin_padding) * i, y, 
            pin_size, pin_size);
        nvgFill(vg);
    }

    for (int i = 0; i < 16; i++)
    {
        nvgBeginPath(vg);
        nvgRect(vg, 
            x + pin_padding + (pin_size + pin_padding) * i, y + height - pin_size, 
            pin_size, pin_size);
        nvgFill(vg);
    }

    nvgFontSize(vg, 15.0f);
    nvgFontFace(vg, "sans");
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
    nvgText(vg, x + 10, y + height / 2, node->name, NULL);
}

int main(void)
{
    GLFWwindow *window;
    struct NVGcontext *vg;

    struct node *node;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

#ifndef _WIN32 // don't require this on win32, and works with more cards
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

#ifdef DEMO_MSAA
    glfwWindowHint(GLFW_SAMPLES, 4);
#endif

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(512, 512, "World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);

    vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
    if (vg == NULL)
    {
        printf("Could not init nanovg.\n");
        return -1;
    }

    nvgCreateFont(vg, "sans", "./assets/Roboto-Regular.ttf");

    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glEnable(GL_CULL_FACE);
    // glDisable(GL_DEPTH_TEST);
    // end initialization

    init_nodes();
    make_node("node_name", 100, 100, sum);

    int frame = 0;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        frame++;

        /* Render here */
        glViewport(0, 0, 512, 512);
        glClearColor(0.5f, 0.5f, 0.5f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        nvgBeginFrame(vg, 512, 512, 1);

        for (int i = 0; i < nodes_pointer; i++)
        {
            draw_node(vg, nodes[i]);
        }

        nvgEndFrame(vg);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}