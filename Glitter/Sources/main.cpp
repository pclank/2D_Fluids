// Local Headers
#include "glitter.hpp"
#include <tools.hpp>
#include <Shader.hpp>
#include <physics.hpp>

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <direct.h>
#include <wingdi.h>

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

    cl_context_properties properties[] =
    {
      CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
      CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
      CL_CONTEXT_PLATFORM, (cl_context_properties)default_platform(),
      NULL
    };

    cl_int err = CL_SUCCESS;
    context = clCreateContext(properties, 1, &default_device(), NULL, NULL, &err);

    if (err != CL_SUCCESS) {
        std::cout << "Error creating context" << " " << err << "\n";
        //exit(-1);
    }

    // Create OpenCL Image
    //target_texture = cl::Image2D(context, CL_MEM_READ_WRITE, cl::ImageFormat(CL_RGBA, CL_FLOAT), mWidth, mHeight);

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
    //kernel_source = "kernel void test(__global int* test_buf){int x = get_global_id(0);\ntest_buf[x] = x;\n}";
    //kernel_source = "kernel void test(__global int* test_buf){int x = get_global_id(0);\ntest_buf[x] = x;\n}\n\nkernel void tex_test(write_only image2d_t tgt_tex)\n{\nint x = get_global_id(0);\nint y = get_global_id(1);\nwrite_imagef(tgt_tex, (int2)(x, y), (float4)(0.1 * x, 0.0f, 0.0f, 0.0f));\n}";
    kernel_source = ReadFile2("C:/Repos/2D_Fluids/Glitter/Sources/gpu_src/test.cl");
    sources.push_back({ kernel_source.c_str(), kernel_source.length() });

    // Build program and compile
    program = cl::Program(context, sources);

    if (program.build({ default_device }) != CL_SUCCESS)
    {
        std::cout << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
        exit(1);
    }
    
    // Prepare buffers
    test_buffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(int) * 10);
    debug_buffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float) * mWidth * mHeight);

    // OpenGL shaders
    Shader simple_shader("C:/Repos/2D_Fluids/Glitter/Shaders/simple_shader.vs", "C:/Repos/2D_Fluids/Glitter/Shaders/simple_shader.fs");

    // Setup OpenGL Buffers
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(hardcoded_vertices), hardcoded_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // OpenGL texture
    unsigned int gl_texture;
    glGenTextures(1, &gl_texture);
    glBindTexture(GL_TEXTURE_2D, gl_texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // OpenGL velocity new texture
    unsigned int gl_texture_new;
    glGenTextures(1, &gl_texture_new);
    glBindTexture(GL_TEXTURE_2D, gl_texture_new);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

#ifdef LOAD_TEXTURE
    int width, height, nrChannels;
    //unsigned char* data = stbi_load("C:/Repos/2D_Fluids/textures/container.jpg", &width, &height, &nrChannels, 0);
    unsigned char* data = stbi_load("C:/Repos/2D_Fluids/textures/wall.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glBindTexture(GL_TEXTURE_2D, gl_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, gl_texture_new);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
#else
    // Set empty
    std::vector<GLubyte> emptyData(mWidth* mHeight * 4, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &emptyData[0]);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#endif // LOAD_TEXTURE

    //glGenerateMipmap(GL_TEXTURE_2D);

    target_texture = clCreateFromGLTexture(context(), CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, gl_texture, &err);
    std::cout << "Created CL Image2D with err:\t" << err << std::endl;

    new_vel = clCreateFromGLTexture(context(), CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, gl_texture_new, &err);
    std::cout << "Created CL Image2D with err:\t" << err << std::endl;

    // Flush GL queue
    glFinish();
    glFlush();

    // Acquire shared objects
    err = clEnqueueAcquireGLObjects(queue(), 1, &target_texture(), 0, NULL, NULL);
    std::cout << "Acquired GL objects with err:\t" << err << std::endl;
    err = clEnqueueAcquireGLObjects(queue(), 1, &new_vel(), 0, NULL, NULL);
    std::cout << "Acquired GL objects with err:\t" << err << std::endl;

#ifdef TEXTURE_TEST
    tester = cl::Kernel(program, "tex_test");
    //tester(cl::EnqueueArgs(queue, global_tex), target_texture).wait();
    cl::NDRange global_test(width, height);
    //tester(cl::EnqueueArgs(queue, global_test), target_texture).wait();

    debug_tester = cl::Kernel(program, "tex_read_test");
    //debug_tester(cl::EnqueueArgs(queue, global_tex), target_texture, debug_buffer);
    debug_tester(cl::EnqueueArgs(queue, global_test), target_texture, debug_buffer);

    advecter = cl::Kernel(program, "AvectFluid");
    advecter(cl::EnqueueArgs(queue, global_test), 0.1f, 1.0f / 1, target_texture, target_texture, new_vel).wait();

    tex_copier = cl::Kernel(program, "CopyTexture");
    tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();

    // We have to generate the mipmaps again!!!
    glBindTexture(GL_TEXTURE_2D, gl_texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gl_texture_new);
    glGenerateMipmap(GL_TEXTURE_2D);

    float test_c[10];
    queue.enqueueReadBuffer(debug_buffer, CL_TRUE, 0, sizeof(float) * 10, &test_c);

    std::cout << test_c[0] << test_c[1] << test_c[2] << std::endl;
#else
    tester = cl::Kernel(program, "test");
    tester(cl::EnqueueArgs(queue, global), test_buffer).wait();

    int test_c[10];
    queue.enqueueReadBuffer(test_buffer, CL_TRUE, 0, sizeof(int) * 10, &test_c);

    std::cout << test_c[0] << test_c[1] << test_c[2] << std::endl;
#endif // TEXTURE_TEST

    // Release shared objects                                                          
    err = clEnqueueReleaseGLObjects(queue(), 1, &target_texture(), 0, NULL, NULL);
    std::cout << "Releasing GL objects with err:\t" << err << std::endl;
    err = clEnqueueReleaseGLObjects(queue(), 1, &new_vel(), 0, NULL, NULL);
    std::cout << "Releasing GL objects with err:\t" << err << std::endl;

    // Flush CL queue
    err = clFinish(queue());
    std::cout << "Finished CL queue with err:\t" << err << std::endl;

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false)
    {
        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Flush GL queue
        glFinish();
        glFlush();

        // Acquire shared objects
        err = clEnqueueAcquireGLObjects(queue(), 1, &target_texture(), 0, NULL, NULL);
        err = clEnqueueAcquireGLObjects(queue(), 1, &new_vel(), 0, NULL, NULL);

        // Run kernels
        advecter(cl::EnqueueArgs(queue, global_test), 0.1f, 1.0f / 1, target_texture, target_texture, new_vel).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();

        // Release shared objects                                                          
        err = clEnqueueReleaseGLObjects(queue(), 1, &target_texture(), 0, NULL, NULL);
        err = clEnqueueReleaseGLObjects(queue(), 1, &new_vel(), 0, NULL, NULL);

        // Flush CL queue
        err = clFinish(queue());

        // bind Texture
        glBindTexture(GL_TEXTURE_2D, gl_texture);
        glBindTexture(GL_TEXTURE_2D, gl_texture_new);
        glGenerateMipmap(GL_TEXTURE_2D);

        // render container
        simple_shader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }

    // Clear buffers
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    
    glfwTerminate();

    return EXIT_SUCCESS;
}
