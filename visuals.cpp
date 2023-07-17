#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <iostream>

// register other functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

int main()
{
    // init the glsl context
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

// macos specific config
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // get the desktop resolution
    int win_width, win_height;
    const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    win_width = mode->width;
    win_height = mode->height;

    // create the glfw window
    GLFWwindow* window = glfwCreateWindow(win_width, win_height, "visuals", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // init glad to load opengl func ptr addresses
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // set size of rendering window
    glViewport(0, 0, win_width, win_height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  

    // simple render loop with double buffer
    while(!glfwWindowShouldClose(window)) {
        // input
        processInput(window);

        // render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // state setting func
        glClear(GL_COLOR_BUFFER_BIT); // state using func

        // display
        glfwPollEvents();    
        glfwSwapBuffers(window);
    }

    // clean up resources and properly exit
    glfwTerminate();
    return 0;
}

// callback function to resize window with user
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}  

// process input to exit when escape key hit
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// g++ visuals.cpp glad.c -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl; ./a.out