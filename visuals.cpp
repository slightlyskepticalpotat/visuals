#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <SFML/Audio.hpp>

#include <iostream>
#include <sstream>
#include <fstream>

// register other functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

int main(int argc, char *argv[]) // name of audio file
{
    if (argc != 2) {
        printf("No audio file provided");
        exit(0);
    }

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

    // basic vertex shader
    std::ostringstream sstream;
    std::ifstream fs("vertexShaderSource.vert");
    sstream << fs.rdbuf();
    const std::string str(sstream.str());
    const char* vertexShaderSource = str.c_str();

    // compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // basic fragment shader
    std::ostringstream sstream2;
    std::ifstream fs2("fragmentShaderSource.frag");
    sstream2 << fs2.rdbuf();
    const std::string str2(sstream2.str());
    const char* fragmentShaderSource = str2.c_str();

    // compile fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // link all of our shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // delete shaders after linking them
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);  

    // input vertex data
    float vertices[] = {
        -0.5303f, -0.5303f, 0.0000f,
        0.5303f, -0.5303f, 0.0000f,
        0.0000f,  0.7500f, 0.0000f
    };

    // opengl object (vertex buffer object)
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    // draw vao object (vertex array object)
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);  

    // ebo is element buffer object
    // stores indicies opengl uses to draw

    // bind the vao and copy data there
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // bind to buffer and populate
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // tell opengl how to read vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);  

    // get the sound data with sfml
    sf::SoundBuffer sound_buffer;
    sound_buffer.loadFromFile(argv[1]);
    const sf::Int16* samples = sound_buffer.getSamples();
    std::size_t count = sound_buffer.getSampleCount();
    int proc_count = count / 1000;
    int proc_samples[proc_count];
    for (int i = 0; i < proc_count; i++) {
        proc_samples[i] = samples[i * 1000];
    }

    // useful if sound samples are all small
    // int min_sample = -16, max_sample = 16, current;

    // actually process the sound data
    // because there are too many samples, have to remove some
    int min_sample = 2147483647, max_sample = -2147483647, current;
    for (int i = 0; i < proc_count; i++) {
        current = proc_samples[i];
        if (current < min_sample) {
            min_sample = current;
        }
        if (current > max_sample) {
            max_sample = current;
        }
    }

    // scale the sound data
    printf("%d %d\n", min_sample, max_sample);
    long long audio_i = 0;
    const float h = (float) 1 / (float) std::max(abs(max_sample), abs(min_sample));

    // frame time counter init
    double last = glfwGetTime();
    int frames = 0;

    // simple render loop with double buffer
    while(!glfwWindowShouldClose(window)) {
        // close if we exceeded the time
        if (audio_i >= proc_count) {
            glfwSetWindowShouldClose(window, true);
        }

        // calculate current visualisation and update counter
        float cur_colour = (float) abs(proc_samples[audio_i]) * h;
        double now = glfwGetTime();
        frames += 1;

        // this updates every second
        if (now - last >= 1.0){ 
            printf("%d fps\n", (int) frames);
            printf("%.2f val\n", cur_colour);
            frames = 0;
            last += 1.0;
        }

        // input
        processInput(window);

        // render background with audio data
        glClearColor(cur_colour, cur_colour, cur_colour, 1.0f); // state setting func
        glClear(GL_COLOR_BUFFER_BIT); // state using func
        audio_i += 1;
        
        // render the fucking triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // draw wireframe triangle
        glDrawArrays(GL_TRIANGLES, 0, 3);

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

// g++ visuals.cpp glad.c -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lsfml-audio -lsfml-window -lsfml-system; ./a.out c418_sweden.flac
