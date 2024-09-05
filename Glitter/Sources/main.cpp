// Local Headers
#include "glitter.hpp"
#include <tools.hpp>
#include <Shader.hpp>
#include <physics.hpp>
#include <GUI.hpp>

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <direct.h>
#include <wingdi.h>

// Some Globals
GUI* gui_pointer;
const std::vector<RenderedTexture> selectables{ VELOCITY, PRESSURE, DYE };

// Callbacks
void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

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

    // Initialize our GUI
    GUI gui = GUI(mWindow, main_timer);
    gui.Init();
    gui_pointer = &gui;

    // OpenGL Callback Functions
    glfwSetCursorPosCallback(mWindow, CursorPositionCallback);
    glfwSetMouseButtonCallback(mWindow, MouseButtonCallback);
    glfwSetKeyCallback(mWindow, KeyboardCallback);

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

    // OpenGL velocity divergence texture
    unsigned int gl_velocity_divergence;
    glGenTextures(1, &gl_velocity_divergence);
    glBindTexture(GL_TEXTURE_2D, gl_velocity_divergence);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // OpenGL pressure textures
    unsigned int gl_pressure_old;
    glGenTextures(1, &gl_pressure_old);
    glBindTexture(GL_TEXTURE_2D, gl_pressure_old);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned int gl_pressure_new;
    glGenTextures(1, &gl_pressure_new);
    glBindTexture(GL_TEXTURE_2D, gl_pressure_new);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // OpenGL vorticity texture
    unsigned int gl_vorticity;
    glGenTextures(1, &gl_vorticity);
    glBindTexture(GL_TEXTURE_2D, gl_vorticity);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // OpenGL dye texture
    unsigned int gl_dye;
    glGenTextures(1, &gl_dye);
    glBindTexture(GL_TEXTURE_2D, gl_dye);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // OpenGL dye texture new
    unsigned int gl_dye_new;
    glGenTextures(1, &gl_dye_new);
    glBindTexture(GL_TEXTURE_2D, gl_dye_new);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // OpenGL display texture
    unsigned int gl_display;
    glGenTextures(1, &gl_display);
    glBindTexture(GL_TEXTURE_2D, gl_display);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

#ifdef LOAD_TEXTURE
    int width, height, nrChannels;
    //unsigned char* data = stbi_load("C:/Repos/2D_Fluids/textures/container.jpg", &width, &height, &nrChannels, 0);
    //unsigned char* data = stbi_load("C:/Repos/2D_Fluids/textures/wall.jpg", &width, &height, &nrChannels, 0);
    unsigned char* data = stbi_load("C:/Repos/2D_Fluids/textures/bricks1K.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        // Pick correct channels
        int src_channels = GL_RGB;
        if (nrChannels == 4)
            src_channels = GL_RGBA;

        std::cout << "Texture width: " << width << " Texture height: " << height << " Texture channels: " << nrChannels << std::endl;

        glBindTexture(GL_TEXTURE_2D, gl_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, src_channels, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, gl_texture_new);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, src_channels, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, gl_velocity_divergence);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, src_channels, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, gl_pressure_old);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, src_channels, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, gl_pressure_new);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, src_channels, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, gl_vorticity);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, src_channels, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, gl_dye);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, src_channels, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, gl_dye_new);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, src_channels, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, gl_display);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, src_channels, GL_UNSIGNED_BYTE, data);
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

    velocity_divergence = clCreateFromGLTexture(context(), CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, gl_velocity_divergence, &err);
    std::cout << "Created CL Image2D with err:\t" << err << std::endl;

    old_pressure = clCreateFromGLTexture(context(), CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, gl_pressure_old, &err);
    std::cout << "Created CL Image2D with err:\t" << err << std::endl;

    new_pressure = clCreateFromGLTexture(context(), CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, gl_pressure_new, &err);
    std::cout << "Created CL Image2D with err:\t" << err << std::endl;

    vorticity = clCreateFromGLTexture(context(), CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, gl_vorticity, &err);
    std::cout << "Created CL Image2D with err:\t" << err << std::endl;

    dye_texture = clCreateFromGLTexture(context(), CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, gl_dye, &err);
    std::cout << "Created CL Image2D with err:\t" << err << std::endl;

    dye_texture_new = clCreateFromGLTexture(context(), CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, gl_dye_new, &err);
    std::cout << "Created CL Image2D with err:\t" << err << std::endl;

    display_texture = clCreateFromGLTexture(context(), CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, gl_display, &err);
    std::cout << "Created CL Image2D with err:\t" << err << std::endl;

    cl_image_format form;
    clGetImageInfo(display_texture(), CL_IMAGE_FORMAT, sizeof(cl_image_format), &form, NULL);
    std::cout << form.image_channel_data_type << std::endl;
    /*CL_UNORM_INT8
    CL_FLOAT*/

    // Flush GL queue
    glFinish();
    glFlush();

    // Acquire shared objects
    err = clEnqueueAcquireGLObjects(queue(), 1, &target_texture(), 0, NULL, NULL);
    std::cout << "Acquired GL objects with err:\t" << err << std::endl;
    err = clEnqueueAcquireGLObjects(queue(), 1, &new_vel(), 0, NULL, NULL);
    std::cout << "Acquired GL objects with err:\t" << err << std::endl;
    err = clEnqueueAcquireGLObjects(queue(), 1, &velocity_divergence(), 0, NULL, NULL);
    std::cout << "Acquired GL objects with err:\t" << err << std::endl;
    err = clEnqueueAcquireGLObjects(queue(), 1, &old_pressure(), 0, NULL, NULL);
    std::cout << "Acquired GL objects with err:\t" << err << std::endl;
    err = clEnqueueAcquireGLObjects(queue(), 1, &new_pressure(), 0, NULL, NULL);
    std::cout << "Acquired GL objects with err:\t" << err << std::endl;
    err = clEnqueueAcquireGLObjects(queue(), 1, &vorticity(), 0, NULL, NULL);
    std::cout << "Acquired GL objects with err:\t" << err << std::endl;
    err = clEnqueueAcquireGLObjects(queue(), 1, &dye_texture(), 0, NULL, NULL);
    std::cout << "Acquired GL objects with err:\t" << err << std::endl;
    err = clEnqueueAcquireGLObjects(queue(), 1, &dye_texture_new(), 0, NULL, NULL);
    std::cout << "Acquired GL objects with err:\t" << err << std::endl;
    err = clEnqueueAcquireGLObjects(queue(), 1, &display_texture(), 0, NULL, NULL);
    std::cout << "Acquired GL objects with err:\t" << err << std::endl;

    cl::NDRange global_test(width, height);
    cl::NDRange global_1D(width * height);
    
#ifdef RAND_TEX
    // Texture randomizer
    tex_randomizer = cl::Kernel(program, "RandomizeTexture");
    tex_randomizer(cl::EnqueueArgs(queue, global_test), target_texture).wait();
    tex_randomizer(cl::EnqueueArgs(queue, global_test), old_pressure).wait();
#endif // RAND_TEX

#ifdef TEXTURE_TEST
    tester = cl::Kernel(program, "tex_test");

    debug_tester = cl::Kernel(program, "tex_read_test");
    //debug_tester(cl::EnqueueArgs(queue, global_tex), target_texture, debug_buffer);
    debug_tester(cl::EnqueueArgs(queue, global_test), target_texture, debug_buffer);

    advecter = cl::Kernel(program, "AvectFluid");
    //advecter(cl::EnqueueArgs(queue, global_test), 0.1f, 1.0f / 1, target_texture, target_texture, new_vel).wait();

    tex_copier = cl::Kernel(program, "CopyTexture");
    //tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();

    // Prepare the rest of the kernels
    divergencer = cl::Kernel(program, "Divergence");
    jacobier = cl::Kernel(program, "Jacobi");
    gradienter = cl::Kernel(program, "Gradient");
    vorticitier = cl::Kernel(program, "Vorticity");
    vorticity_confiner = cl::Kernel(program, "VorticityConfinement");
#ifdef NEUMANN_BOUND
    boundarier = cl::Kernel(program, "NeumannBoundary");
#else
    boundarier = cl::Kernel(program, "Boundary");
#endif // NEUMANN_BOUND
    display_converter = cl::Kernel(program, "DisplayConvert");
    mixer = cl::Kernel(program, "Mix");
    force_randomizer = cl::Kernel(program, "RandomForce");
    tex_neg_randomizer = cl::Kernel(program, "RandomizeNegativeTexture");
    neg_checker = cl::Kernel(program, "CheckNegativeValues");
    click_effect_tester = cl::Kernel(program, "ClickEffectTest");
    image_resetter = cl::Kernel(program, "ResetImage");
    click_effecter = cl::Kernel(program, "ClickAddPressure");
    dye_adder = cl::Kernel(program, "AddDye");
    gravitier = cl::Kernel(program, "ApplyGravity");

#ifdef RESET_TEXTURES
    image_resetter(cl::EnqueueArgs(queue, global_test), target_texture).wait();
    image_resetter(cl::EnqueueArgs(queue, global_test), new_vel).wait();
    image_resetter(cl::EnqueueArgs(queue, global_test), velocity_divergence).wait();
    image_resetter(cl::EnqueueArgs(queue, global_test), old_pressure).wait();
    image_resetter(cl::EnqueueArgs(queue, global_test), new_pressure).wait();
    image_resetter(cl::EnqueueArgs(queue, global_test), vorticity).wait();
    image_resetter(cl::EnqueueArgs(queue, global_test), dye_texture).wait();
    image_resetter(cl::EnqueueArgs(queue, global_test), dye_texture_new).wait();
#endif // RESET_TEXTURES

    // We have to generate the mipmaps again!!!
    glBindTexture(GL_TEXTURE_2D, gl_texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gl_texture_new);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gl_velocity_divergence);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gl_pressure_old);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gl_pressure_new);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gl_vorticity);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gl_dye);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gl_dye_new);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gl_display);
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
    err = clEnqueueReleaseGLObjects(queue(), 1, &velocity_divergence(), 0, NULL, NULL);
    std::cout << "Releasing GL objects with err:\t" << err << std::endl;
    err = clEnqueueReleaseGLObjects(queue(), 1, &old_pressure(), 0, NULL, NULL);
    std::cout << "Releasing GL objects with err:\t" << err << std::endl;
    err = clEnqueueReleaseGLObjects(queue(), 1, &new_pressure(), 0, NULL, NULL);
    std::cout << "Releasing GL objects with err:\t" << err << std::endl;
    err = clEnqueueReleaseGLObjects(queue(), 1, &vorticity(), 0, NULL, NULL);
    std::cout << "Releasing GL objects with err:\t" << err << std::endl;
    err = clEnqueueReleaseGLObjects(queue(), 1, &dye_texture(), 0, NULL, NULL);
    std::cout << "Releasing GL objects with err:\t" << err << std::endl;
    err = clEnqueueReleaseGLObjects(queue(), 1, &dye_texture_new(), 0, NULL, NULL);
    std::cout << "Releasing GL objects with err:\t" << err << std::endl;
    err = clEnqueueReleaseGLObjects(queue(), 1, &display_texture(), 0, NULL, NULL);
    std::cout << "Releasing GL objects with err:\t" << err << std::endl;

    // Flush CL queue
    err = clFinish(queue());
    std::cout << "Finished CL queue with err:\t" << err << std::endl;

    // Initialize Timer
    main_timer.Init();

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false)
    {
        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        // Update Timer
        main_timer.UpdateTime();

#ifdef STD_TIMESTEP
        float time_step = 1.0f;
#else
        float time_step = main_timer.GetDeltaTime();
#endif // STD_TIMESTEP

        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Flush GL queue
        glFinish();
        glFlush();

        // Acquire shared objects
        err = clEnqueueAcquireGLObjects(queue(), 1, &target_texture(), 0, NULL, NULL);
        err = clEnqueueAcquireGLObjects(queue(), 1, &new_vel(), 0, NULL, NULL);
        err = clEnqueueAcquireGLObjects(queue(), 1, &velocity_divergence(), 0, NULL, NULL);
        err = clEnqueueAcquireGLObjects(queue(), 1, &old_pressure(), 0, NULL, NULL);
        err = clEnqueueAcquireGLObjects(queue(), 1, &new_pressure(), 0, NULL, NULL);
        err = clEnqueueAcquireGLObjects(queue(), 1, &vorticity(), 0, NULL, NULL);
        err = clEnqueueAcquireGLObjects(queue(), 1, &dye_texture(), 0, NULL, NULL);
        err = clEnqueueAcquireGLObjects(queue(), 1, &dye_texture_new(), 0, NULL, NULL);
        err = clEnqueueAcquireGLObjects(queue(), 1, &display_texture(), 0, NULL, NULL);

        // Reset simulation
        if (gui.reset_pressed)
        {
            image_resetter(cl::EnqueueArgs(queue, global_test), target_texture).wait();
            image_resetter(cl::EnqueueArgs(queue, global_test), new_vel).wait();
            image_resetter(cl::EnqueueArgs(queue, global_test), velocity_divergence).wait();
            image_resetter(cl::EnqueueArgs(queue, global_test), old_pressure).wait();
            image_resetter(cl::EnqueueArgs(queue, global_test), new_pressure).wait();
            image_resetter(cl::EnqueueArgs(queue, global_test), vorticity).wait();
            image_resetter(cl::EnqueueArgs(queue, global_test), dye_texture).wait();
            image_resetter(cl::EnqueueArgs(queue, global_test), dye_texture_new).wait();
            gui.reset_pressed = false;
        }

#ifndef DISABLE_SIM
        // ****************************************************************************************
        // Add Dye or Force
        // ****************************************************************************************

        // Random force
        if (gui.IsForceEnabled())
        {
            force_randomizer(cl::EnqueueArgs(queue, global_test), gui.GetForceScale(), gui.GetForceDirFlag(), target_texture, new_vel).wait();
            tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();
            //force_randomizer(cl::EnqueueArgs(queue, global_test), gui.GetForceScale(), old_pressure, new_pressure).wait();
            //tex_copier(cl::EnqueueArgs(queue, global_test), old_pressure, new_pressure).wait();

            gui.ResetForceEnabled();
        }

        // Gravity
        if (gui.apply_gravity)
        {
            gravitier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();
        }

        // Click adder
        if (gui.clicked && gui.clicking_enabled)
        {
            // Velocity adder
            /*if (gui.click_mode == VELOCITY_MODE)
                click_effecter(cl::EnqueueArgs(queue, single_thread), static_cast<int>(gui.mouse_xpos), static_cast<int>(gui.mouse_ypos), gui.GetForceScale(), gui.dye_extreme_mode, new_vel, target_texture).wait();*/
            if (gui.click_mode == VELOCITY_MODE)
                dye_adder(cl::EnqueueArgs(queue, single_thread), static_cast<int>(gui.mouse_xpos), static_cast<int>(gui.mouse_ypos), gui.GetForceScale(), gui.dye_extreme_mode, target_texture).wait();
            // Dye adder
            else
                dye_adder(cl::EnqueueArgs(queue, single_thread), static_cast<int>(gui.mouse_xpos), static_cast<int>(gui.mouse_ypos), gui.GetForceScale(), gui.dye_extreme_mode, dye_texture).wait();
        }

        // ****************************************************************************************
        // Bound Velocity
        // ****************************************************************************************
#ifdef NEUMANN_BOUND
        boundarier(cl::EnqueueArgs(queue, global_1D), -1.0f, target_texture, new_vel).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();
#else
        boundarier(cl::EnqueueArgs(queue, global_test), -1.0f, target_texture, new_vel).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();
#endif // NEUMANN_BOUND

        // ****************************************************************************************
        // Diffusion for viscous fluid
        // ****************************************************************************************
        float centerFactor = 1.0f / (gui.viscosity * time_step);
        float stencilFactor = 1.0f / (4.0f + centerFactor);
        if (gui.viscosity > 0.0f)
        {
            for (int i = 0; i < JACOBI_REPS; i++)
            {
                jacobier(cl::EnqueueArgs(queue, global_test), centerFactor, stencilFactor, target_texture, target_texture, new_vel).wait();
                tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();
            }
        }

        // ****************************************************************************************
        // Bound Velocity
        // ****************************************************************************************
#ifdef NEUMANN_BOUND
        boundarier(cl::EnqueueArgs(queue, global_1D), -1.0f, target_texture, new_vel).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();
#else
        boundarier(cl::EnqueueArgs(queue, global_test), -1.0f, target_texture, new_vel).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();
#endif // NEUMANN_BOUND

        // ****************************************************************************************
        // Project divergent velocity into divergence-free field
        // ****************************************************************************************

        // Divergence of velocity field
        divergencer(cl::EnqueueArgs(queue, global_test), 0.5f / gui.dx, target_texture, velocity_divergence).wait();
        //tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();

        // Pressure disturbance
#ifdef RESET_PRESSURE_EACH_ITER
        image_resetter(cl::EnqueueArgs(queue, global_test), old_pressure).wait();
        image_resetter(cl::EnqueueArgs(queue, global_test), new_pressure).wait();
#endif // RESET_PRESSURE_EACH_ITER

        centerFactor = -1.0f;
        stencilFactor = 0.25f;
        for (int i = 0; i < JACOBI_REPS; i++)
        {
#ifdef NEUMANN_BOUND
            boundarier(cl::EnqueueArgs(queue, global_1D), 1.0f, old_pressure, new_pressure).wait();
            tex_copier(cl::EnqueueArgs(queue, global_test), new_pressure, old_pressure).wait();
#else
            boundarier(cl::EnqueueArgs(queue, global_test), 1.0f, old_pressure, new_pressure).wait();
            tex_copier(cl::EnqueueArgs(queue, global_test), new_pressure, old_pressure).wait();
#endif // NEUMANN_BOUND

            //jacobier(cl::EnqueueArgs(queue, global_test), centerFactor, stencilFactor, old_pressure, target_texture, new_pressure).wait();
            jacobier(cl::EnqueueArgs(queue, global_test), centerFactor, stencilFactor, old_pressure, velocity_divergence, new_pressure).wait();
            tex_copier(cl::EnqueueArgs(queue, global_test), new_pressure, old_pressure).wait();
        }

        // Set no-slip velocity
#ifdef NEUMANN_BOUND
        boundarier(cl::EnqueueArgs(queue, global_1D), -1.0f, target_texture, new_vel).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();
#else
        boundarier(cl::EnqueueArgs(queue, global_test), -1.0f, target_texture, new_vel).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();
#endif // NEUMANN_BOUND

        // Subtract gradient(p) from u to get divergence-free velocity field
        gradienter(cl::EnqueueArgs(queue, global_test), 0.5f / gui.dx, old_pressure, target_texture, new_vel).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();

        // ****************************************************************************************
        // Advect Velocity
        // ****************************************************************************************
        advecter(cl::EnqueueArgs(queue, global_test), time_step, 1.0f / gui.dx, 1.0f, target_texture, target_texture, new_vel).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();

#ifdef NEUMANN_BOUND
        boundarier(cl::EnqueueArgs(queue, global_1D), -1.0f, target_texture, new_vel).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();
#else
        boundarier(cl::EnqueueArgs(queue, global_test), -1.0f, target_texture, new_vel).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();
#endif // NEUMANN_BOUND

        // ****************************************************************************************
        // Vorticity
        // ****************************************************************************************
#ifdef VORTICITY
        vorticitier(cl::EnqueueArgs(queue, global_test), 0.5f / gui.dx, target_texture, vorticity).wait();

#ifdef NEUMANN_BOUND
        boundarier(cl::EnqueueArgs(queue, global_1D), -1.0f, target_texture, new_vel).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();
#else
        boundarier(cl::EnqueueArgs(queue, global_test), -1.0f, target_texture, new_vel).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();
#endif // NEUMANN_BOUND

        vorticity_confiner(cl::EnqueueArgs(queue, global_test), 0.5f / gui.dx, time_step, 0.035f, 0.035f, vorticity, target_texture, new_vel).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();
#endif // VORTICITY

        // ****************************************************************************************
        // Project divergent velocity into divergence-free field
        // ****************************************************************************************

        // Divergence of velocity field
        divergencer(cl::EnqueueArgs(queue, global_test), 0.5f / gui.dx, target_texture, velocity_divergence).wait();
        //tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();

        // Pressure disturbance
#ifdef RESET_PRESSURE_EACH_ITER
        image_resetter(cl::EnqueueArgs(queue, global_test), old_pressure).wait();
        image_resetter(cl::EnqueueArgs(queue, global_test), new_pressure).wait();
#endif // RESET_PRESSURE_EACH_ITER

        centerFactor = -1.0f;
        stencilFactor = 0.25f;
        for (int i = 0; i < JACOBI_REPS; i++)
        {
#ifdef NEUMANN_BOUND
            boundarier(cl::EnqueueArgs(queue, global_1D), 1.0f, old_pressure, new_pressure).wait();
            tex_copier(cl::EnqueueArgs(queue, global_test), new_pressure, old_pressure).wait();
#else
            boundarier(cl::EnqueueArgs(queue, global_test), 1.0f, old_pressure, new_pressure).wait();
            tex_copier(cl::EnqueueArgs(queue, global_test), new_pressure, old_pressure).wait();
#endif // NEUMANN_BOUND

            //jacobier(cl::EnqueueArgs(queue, global_test), centerFactor, stencilFactor, old_pressure, target_texture, new_pressure).wait();
            jacobier(cl::EnqueueArgs(queue, global_test), centerFactor, stencilFactor, old_pressure, velocity_divergence, new_pressure).wait();
            tex_copier(cl::EnqueueArgs(queue, global_test), new_pressure, old_pressure).wait();
        }

        // Set no-slip velocity
#ifdef NEUMANN_BOUND
        boundarier(cl::EnqueueArgs(queue, global_1D), -1.0f, target_texture, new_vel).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();
#else
        boundarier(cl::EnqueueArgs(queue, global_test), -1.0f, target_texture, new_vel).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();
#endif // NEUMANN_BOUND

        // Subtract gradient(p) from u to get divergence-free velocity field
        gradienter(cl::EnqueueArgs(queue, global_test), 0.5f / gui.dx, old_pressure, target_texture, new_vel).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), new_vel, target_texture).wait();

        // ****************************************************************************************
        // Advect Dye
        // ****************************************************************************************
        //advecter(cl::EnqueueArgs(queue, global_test), time_step, 1.0f / gui.dx, 0.995f, target_texture, dye_texture, dye_texture_new).wait();
        advecter(cl::EnqueueArgs(queue, global_test), time_step, 1.0f / gui.dx, 1.0f, target_texture, dye_texture, dye_texture_new).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), dye_texture_new, dye_texture).wait();

        // ****************************************************************************************
        // Bound Dye
        // ****************************************************************************************
#ifdef NEUMANN_BOUND
        boundarier(cl::EnqueueArgs(queue, global_1D), 0.0f, dye_texture, dye_texture_new).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), dye_texture_new, dye_texture).wait();
#else
        boundarier(cl::EnqueueArgs(queue, global_test), 0.0f, dye_texture, dye_texture_new).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), dye_texture_new, dye_texture).wait();
#endif // NEUMANN_BOUND

        // ****************************************************************************************
        // Diffusion for dye
        // ****************************************************************************************
        centerFactor = 1.0f / (gui.viscosity * time_step);
        stencilFactor = 1.0f / (4.0f + centerFactor);
        for (int i = 0; i < JACOBI_REPS; i++)
        {
            //jacobier(cl::EnqueueArgs(queue, global_test), centerFactor, stencilFactor, dye_texture, target_texture, dye_texture_new).wait();
            jacobier(cl::EnqueueArgs(queue, global_test), centerFactor, stencilFactor, dye_texture, dye_texture, dye_texture_new).wait();
            tex_copier(cl::EnqueueArgs(queue, global_test), dye_texture_new, dye_texture).wait();
        }

        // ****************************************************************************************
        // Bound Dye
        // ****************************************************************************************
#ifdef NEUMANN_BOUND
        boundarier(cl::EnqueueArgs(queue, global_1D), 0.0f, dye_texture, dye_texture_new).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), dye_texture_new, dye_texture).wait();
#else
        boundarier(cl::EnqueueArgs(queue, global_test), 0.0f, dye_texture, dye_texture_new).wait();
        tex_copier(cl::EnqueueArgs(queue, global_test), dye_texture_new, dye_texture).wait();
#endif // NEUMANN_BOUND

        // Display stuff
        mixer(cl::EnqueueArgs(queue, global_test), gui.GetMixBias(), new_vel, new_pressure, display_texture).wait();
        //display_converter(cl::EnqueueArgs(queue, global_test), new_vel, display_texture).wait();
#endif // DISABLE_SIM

        // Release shared objects
        err = clEnqueueReleaseGLObjects(queue(), 1, &target_texture(), 0, NULL, NULL);
        err = clEnqueueReleaseGLObjects(queue(), 1, &new_vel(), 0, NULL, NULL);
        err = clEnqueueReleaseGLObjects(queue(), 1, &velocity_divergence(), 0, NULL, NULL);
        err = clEnqueueReleaseGLObjects(queue(), 1, &old_pressure(), 0, NULL, NULL);
        err = clEnqueueReleaseGLObjects(queue(), 1, &new_pressure(), 0, NULL, NULL);
        err = clEnqueueReleaseGLObjects(queue(), 1, &vorticity(), 0, NULL, NULL);
        err = clEnqueueReleaseGLObjects(queue(), 1, &dye_texture(), 0, NULL, NULL);
        err = clEnqueueReleaseGLObjects(queue(), 1, &dye_texture_new(), 0, NULL, NULL);
        err = clEnqueueReleaseGLObjects(queue(), 1, &display_texture(), 0, NULL, NULL);

        // Flush CL queue
        err = clFinish(queue());

        // bind Texture
        //glBindTexture(GL_TEXTURE_2D, gl_texture);
        //glBindTexture(GL_TEXTURE_2D, gl_texture_new);
        //glBindTexture(GL_TEXTURE_2D, gl_pressure_old);
        //glBindTexture(GL_TEXTURE_2D, gl_vorticity);
        //glBindTexture(GL_TEXTURE_2D, gl_display);

        if (selectables[gui.selected_index] == DYE)
            glBindTexture(GL_TEXTURE_2D, gl_dye);
        else if (selectables[gui.selected_index] == VELOCITY)
            glBindTexture(GL_TEXTURE_2D, gl_texture);
        else if (selectables[gui.selected_index] == PRESSURE)
            glBindTexture(GL_TEXTURE_2D, gl_pressure_old);

        glGenerateMipmap(GL_TEXTURE_2D);

        // render container
        simple_shader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Render GUI
        if (gui.gui_enabled)
            gui.Render();

        // Reset input flags
        gui.ResetInputFlags();

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }

    // Cleanup GUI
    gui.Cleanup();

    // Clear buffers
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    
    glfwTerminate();

    return EXIT_SUCCESS;
}

/// <summary>
/// Callback function for mouse cursor movement
/// </summary>
/// <param name="window"></param>
/// <param name="xpos"></param>
/// <param name="ypos"></param>
void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    gui_pointer->MousePositionUpdate(xpos, ypos);

    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
}

/// <summary>
/// Callback function for mouse button press
/// </summary>
/// <param name="window"></param>
/// <param name="button"></param>
/// <param name="action"></param>
/// <param name="mods"></param>
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        std::cout << "MOUSE CLICK on x: " << gui_pointer->mouse_xpos << " y: " << gui_pointer->mouse_ypos << std::endl;
        // TODO: Add functionality!
        gui_pointer->clicked = true;
    }

    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}

/// <summary>
/// Callback Function for keyboard button press
/// </summary>
/// <param name="window"></param>
/// <param name="key"></param>
/// <param name="scancode"></param>
/// <param name="action"></param>
/// <param name="mods"></param>
void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Enable/Disable Cursor
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        if (gui_pointer->cursor_enabled)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);

        gui_pointer->cursor_enabled = !gui_pointer->cursor_enabled;
    }
    // Enable/Disable Viewport clicking
    else if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        gui_pointer->clicking_enabled = !gui_pointer->clicking_enabled;
    }
    // Reset simulation
    else if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        gui_pointer->reset_pressed = true;
    }
    // Switch click mode
    else if (key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        gui_pointer->click_mode = (gui_pointer->click_mode == VELOCITY_MODE) ? DYE_MODE : VELOCITY_MODE;
    }
    // Enable/Disable GUI
    else if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
    {
        gui_pointer->gui_enabled = !gui_pointer->gui_enabled;
    }
}
