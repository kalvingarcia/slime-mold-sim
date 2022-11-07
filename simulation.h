#pragma once
// defining some error constants inorder to find where the code is exiting upon error
#define GLFW_INIT_FAIL -1
#define GLFW_WINDOW_FAIL -2
#define GLEW_INIT_FAIL -3

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <random>
#include <cmath>

#define GLEW_STATIC
#include <glew.h>
#include <glfw3.h>

#include <json.hpp>
using json = nlohmann::json;

#include "shader.h"

// program settings
struct program_settings {
    bool fullscreen = false; // boolean to keep track of if the window is fullscreen
    bool paused = true; // boolean to keep track of if the simulation is paused

    // width and height of the window / map of the simulation
    int width = 0;
    int height = 0;
} window_settings; // declaring a struct as well

void frame_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
/*
    get_current_monitor function

    takes in a GLFWwindow pointer
    returns a GLFWmonitor pointer

    description:
        checks for the best monitor fit based on the placement of the window
*/
GLFWmonitor* get_current_monitor(GLFWwindow* window) {
    int monitor_count; // the monitor count for the user
    int window_x, window_y, window_width, window_height; // the windows current sizing and positon
    int monitor_x, monitor_y, monitor_width, monitor_height; // the sizing and positon for the monitors
    int overlap, best_overlap; // the variables that hold the overlap of the window to each monitor
    GLFWmonitor* best, ** monitors; // the best monitor and the monitor array
    const GLFWvidmode* mode; // the current mode of the monitor

    // initializing the best
    best_overlap = 0;
    best = NULL;

    // getting the position of the window and the size of the window
    glfwGetWindowPos(window, &window_x, &window_y);
    glfwGetWindowSize(window, &window_width, &window_height);
    // getting the monitors
    monitors = glfwGetMonitors(&monitor_count);

    // time to check which monitor is best
    for (int i = 0; i < monitor_count; i++) {
        mode = glfwGetVideoMode(monitors[i]);
        glfwGetMonitorPos(monitors[i], &monitor_x, &monitor_y);
        // setting the monitor width and height
        monitor_width = mode->width;
        monitor_height = mode->height;

        // checking the overlap of the monitor and window
        overlap = std::max(0, std::min(window_x + window_width, monitor_x + monitor_width) - std::max(window_x, monitor_x)) *
            std::max(0, std::min(window_y + window_height, monitor_y + monitor_width) - std::max(window_y, monitor_y));

        if (best_overlap < overlap) {
            best_overlap = overlap;
            best = monitors[i];
        }
    }

    return best; // returning the best monitor
}
/*
    key_callback function

    takes a GLFWwindow pointer, an int that represents a key, an int representing the action

    description:
        the function checks for certain key press events and handles them
*/
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // handling the esc key, which quits
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // handling the f11 key, which full screens the program
    if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
        if (!window_settings.fullscreen) {
            window_settings.fullscreen = true;
            glfwSetWindowMonitor(window, get_current_monitor(window), 0, 0, window_settings.width, window_settings.height, GLFW_DONT_CARE);
            glfwSwapBuffers(window);
        }
        else {
            window_settings.fullscreen = false;
            glfwSetWindowMonitor(window, NULL, 500, 500, window_settings.width, window_settings.height, GLFW_DONT_CARE);
            glfwSwapBuffers(window);
        }
    }

    // handling the space key, which pauses the simulation
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        window_settings.paused = !(window_settings.paused);
}

/*
    Simulation class

    description:
        this is the class that drives the simulation. it initializes the simulation settings and all the GLFW parameters it needs to function
        the class also has the main program loop

    member variables:
        sim_settings 
        settingsSSBO
        simulaton_window
        VBO, VAO, EBO
        trail_texture, agent_texture
        AGENT_COUNT
        spawn_method
        agent_array
        agentSSBO
        display
        compute
*/
class Simulation {
    private:
        // simulation settings
        struct simulation_settings {
            // agent settings
            float move_speed;
            float turn_speed;
            float sensor_angle;
            float sensor_distance;

            // map size
            int width;
            int height;

            // diffusion and decay settings
            float r;
            float g;
            float b;
            float decay_rate;
            float diffuse_rate;
        } sim_settings; // a simulation_settings struct to house all the information from the json file
        GLuint settingsSSBO; // used to hold the shader storage buffer object            
        GLFWwindow* simulation_window = NULL; // pointer to the GLFWwindow

        // drawing information
        GLuint VBO, VAO, EBO; // Vertex Buffer Object, Vertex Array Object, and Edge Buffer Object
        GLuint trail_texture, agent_texture; // used to store the images that the agents and trails generate

        // agent settings
        int AGENT_COUNT; // agent count
        std::string spawn_method; // the method the agents will be spawned
        struct agent {
            float x;
            float y;
            float angle;
        } *agent_array; // holds all the agent struct pointers
        GLuint agentSSBO; // agent shader storage buffer object

        // shaders
        DisplayShader* display; // the vertex and fragment shaders
        ComputeShader* compute; // the compute shader

        /*
            init_settings function

            description:
                this function opens the settings json file and initializes the sim_settings member variable
        */
        void init_settings() {
            std::ifstream fin("./settings.json");
            if (!fin) {
                fprintf(stderr, "Could not load the settings json file.\n");
                exit(-1);
            }

            json settings_file;
            fin >> settings_file;

            AGENT_COUNT = settings_file["agent_count"];
            spawn_method = settings_file["spawn_method"];

            window_settings.width = settings_file["map_width"];
            window_settings.height = settings_file["map_height"];

            sim_settings.move_speed = settings_file["move_speed"];
            sim_settings.turn_speed = settings_file["turn_speed"];
            sim_settings.sensor_angle = settings_file["sensor_angle"];
            sim_settings.sensor_distance = settings_file["sensor_distance"];

            sim_settings.width = settings_file["map_width"];
            sim_settings.height = settings_file["map_height"];

            sim_settings.r = settings_file["color_r"] / 255.0f;
            sim_settings.g = settings_file["color_g"] / 255.0f;
            sim_settings.b = settings_file["color_b"] / 255.0f;
            sim_settings.decay_rate = settings_file["decay_rate"];
            sim_settings.diffuse_rate = settings_file["diffuse_rate"];
        }
        /*
            init_buffer function

            description:
                this function take the vertex information, and creates openGL buffers and initializes them
        */
        void init_buffers() {
            float rectangle_vert[] = {
                // positions        // colors          // texture coords
                1.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,  // top right
                1.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,  // bot right
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.0f, 0.0f,  // bot left
                -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  0.0f, 1.0f   // top left
            };

            unsigned int indices[] = {
                0, 1, 3, // first triangle
                1, 2, 3 // second triangle
            };

            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle_vert), rectangle_vert, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            // position attrib
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // color attrib
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            // texture coord attrib
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);
        }
        /*
            init_textures function

            description:
                creates the trail and agent textures that will be drawn onto the screen
        */
        void init_textures() {
            glGenTextures(1, &trail_texture);
            glGenTextures(1, &agent_texture);

            // trail texture
            glBindTexture(GL_TEXTURE_2D, trail_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, window_settings.width, window_settings.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

            // agent texture
            glBindTexture(GL_TEXTURE_2D, agent_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, window_settings.width, window_settings.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

            float alphaVal[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            glClearTexImage(agent_texture, 0, GL_RGBA, GL_FLOAT, alphaVal);
        }
        /*
            init_agents function

            description:
               positions the agents based on the spawn method using random functions 
        */
        void init_agents() {
            std::random_device rd;
            std::mt19937 gen(rd());

            for (int i = 0; i < AGENT_COUNT; i++) {
                int center_x = window_settings.width / 2;
                int center_y = window_settings.height / 2;

                if (this->spawn_method == "center") {
                    std::uniform_real_distribution<> randomAngle(0, 12.5662);
                    agent_array[i].x = center_x;
                    agent_array[i].y = center_y;
                    agent_array[i].angle = randomAngle(gen);
                } else if (spawn_method == "random") {
                    std::uniform_real_distribution<> randomAngle(0, 6.2831);
                    std::uniform_int_distribution<> randomX(0, window_settings.width);
                    std::uniform_int_distribution<> randomY(0, window_settings.height);

                    agent_array[i].x = randomX(gen);
                    agent_array[i].y = randomY(gen);
                    agent_array[i].angle = randomAngle(gen);
                } else if(spawn_method == "circle") {
                    std::uniform_real_distribution<> randomAngle(0, 6.2831);
                    std::uniform_int_distribution<> randomR(0, (window_settings.width + window_settings.height) / 10);

                    float radius = randomR(gen);
                    float spawn_angle = randomAngle(gen);

                    agent_array[i].angle = randomAngle(gen);
                    agent_array[i].x = center_x + radius * cos(spawn_angle);
                    agent_array[i].y = center_y + radius * sin(spawn_angle);
                    
                } else if (spawn_method == "ring") {
                    std::uniform_real_distribution<> randomAngle(0, 6.2831);

                    float radius = (window_settings.width + window_settings.height) / 10;
                    float spawn_angle = randomAngle(gen);

                    agent_array[i].angle = randomAngle(gen);
                    agent_array[i].x = center_x + radius * cos(spawn_angle);
                    agent_array[i].y = center_y + radius * sin(spawn_angle);

                }
            }
        }

	public:
        /*
            Simulation contructor

            description:
                calls all the functions necessary to intialize the simulation
                handles intializing GLFW and GLEW
                Creates the display and compute shader programs
        */
        Simulation() {
            init_settings();

            // Initialise GLFW
            if (!glfwInit()) {
                fprintf(stderr, "Failed to initialize GLFW\n");
                exit(GLFW_INIT_FAIL);
            }

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            simulation_window = glfwCreateWindow(window_settings.width, window_settings.height, "Slime Mold Simulation", NULL, NULL);
            if (simulation_window == NULL) {
                fprintf(stderr, "Failed to open GLFW window.\n");
                glfwTerminate();
                exit(GLFW_WINDOW_FAIL);
            }

            // Initialize GLEW
            glfwMakeContextCurrent(simulation_window);

            glfwSetFramebufferSizeCallback(simulation_window, frame_callback);
            glfwSetKeyCallback(simulation_window, key_callback);

            if (glewInit() != GLEW_OK) {
                fprintf(stderr, "Failed to initialize GLEW\n");
                exit(GLEW_INIT_FAIL);
            }

            display = new DisplayShader();
            compute = new ComputeShader();

            init_buffers();
            init_textures();

            glGenBuffers(1, &settingsSSBO);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, settingsSSBO);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(sim_settings), &sim_settings, GL_STATIC_DRAW);

            agent_array = (agent*)malloc(AGENT_COUNT * sizeof(agent));
            init_agents();

            glGenBuffers(1, &agentSSBO);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, agentSSBO);
            glBufferData(GL_SHADER_STORAGE_BUFFER, AGENT_COUNT * sizeof(agent), agent_array, GL_DYNAMIC_READ);

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        /*
            Simulation destructure
        
            description:
                handles the destruction of the pointers within the program
        */
        ~Simulation() {
            delete display, compute;
            free(agent_array);
            glfwTerminate();
        }

        /*
            run function

            description:
                used to fun the simulation
        */
        void run() {
            // Clear the screen
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers(simulation_window);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            while (!glfwWindowShouldClose(simulation_window)) {
                if (window_settings.paused) {
                    glfwPollEvents();
                    continue;
                }

                // Clear the screen
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                // Run simulation

                // run vertex and fragment shader
                display->use();
                glBindVertexArray(VAO);

                glBindImageTexture(1, trail_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
                glBindImageTexture(2, agent_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, settingsSSBO);

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                // run compute shader
                compute->use();

                glBindImageTexture(1, trail_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
                glBindImageTexture(2, agent_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, settingsSSBO);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, agentSSBO);

                const int compute_divisor = 256;
                compute->dispatch(AGENT_COUNT / compute_divisor, 1);

                glMemoryBarrier(GL_ALL_BARRIER_BITS);

                // Swap buffers
                glfwSwapBuffers(simulation_window);
                glfwPollEvents();
            }
        }
};