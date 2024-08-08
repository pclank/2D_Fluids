// Local Headers
#include "glitter.hpp"
#include <tools.hpp>

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <direct.h>

int main(int argc, char * argv[]) {

    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto mWindow = glfwCreateWindow(mWidth, mHeight, "OpenGL", nullptr, nullptr);

    // Check for Valid Context
    if (mWindow == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    gladLoadGL();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

    // OpenCL initialization
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);
    if (all_platforms.size() == 0) {
        std::cout << " No platforms found. Check OpenCL installation!\n";
        exit(1);
    }
    cl::Platform default_platform = all_platforms[0];
    std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << "\n";

    std::vector<cl::Device> all_devices;
    default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    if (all_devices.size() == 0) {
        std::cout << " No devices found.\n";
        exit(1);
    }

    default_device = all_devices[0];
    std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << "\n";

    context = cl::Context({ default_device });

    // Create OpenCL Image
    target_texture = cl::Image2D(context, CL_MEM_READ_ONLY, cl::ImageFormat(CL_RGBA, CL_FLOAT), mWidth, mHeight);

    queue = cl::CommandQueue(context, default_device);

    // Write Constant Images
    cl::size_t<3> origin;
    cl::size_t<3> region;
    region[0] = mWidth;
    region[1] = mHeight;
    region[2] = 1;
    //queue.enqueueWriteImage(target_texture, CL_TRUE, origin, region, 0, 0, &data[0]);

    // OpenCL source
    // Define a buffer 
    const size_t size = 1024;
    // Allocate a character array to store the directory path
    char buffer[size];

    // Call _getcwd to get the current working directory and store it in buffer
    if (getcwd(buffer, size) != NULL) {
        // print the current working directory
        std::cout << "Current working directory: " << buffer << std::endl;
    }
    else {
        // If _getcwd returns NULL, print an error message
        std::cerr << "Error getting current working directory" << std::endl;
    }


    //kernel_source = ReadFile("C:/Repos/2D_Fluids/Build/2D_Fluids/Release/gpu_src/test.cl");
    kernel_source = "kernel void test(__global int* test_buf){int x = get_global_id(0);\ntest_buf[x] = x;\n}";
    sources.push_back({ kernel_source.c_str(), kernel_source.length() });

    // Build program and compile
    program = cl::Program(context, sources);

    if (program.build({ default_device }) != CL_SUCCESS)
    {
        std::cout << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
        exit(1);
    }

    tester = cl::Kernel(program, "test");
    
    test_buffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(int) * 10);
    //queue.enqueueFillBuffer(test_buffer, 0, 0, sizeof(int) * 10);

    tester(cl::EnqueueArgs(queue, global), test_buffer).wait();

    int test_c[10];
    queue.enqueueReadBuffer(test_buffer, CL_TRUE, 0, sizeof(int) * 10, &test_c);

    std::cout << test_c[0] << test_c[1] << test_c[2] << std::endl;

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false)
    {
        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }   
    
    glfwTerminate();

    return EXIT_SUCCESS;
}
