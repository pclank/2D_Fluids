// Preprocessor Directives
#ifndef GLITTER
#define GLITTER
#pragma once

// System Headers
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <CL/cl.hpp>

// Define Some Constants
const int mWidth = 1280;
const int mHeight = 800;

// **********************************************************************************
// OpenCL section
// **********************************************************************************

cl::Device default_device;
cl::Context context;
std::string kernel_source;
cl::Program::Sources sources;
cl::CommandQueue queue;
cl::Program program;
cl::Buffer test_buffer;
cl::Buffer debug_buffer;
cl::Kernel test_kernel;
cl::Kernel debug_kernel;
cl::Kernel advect_kernel;
cl::Kernel tex_copy_kernel;
cl::NDRange global_tex(mWidth, mHeight);
cl::NDRange global(10);

float hardcoded_vertices[] = {
    // positions          // colors           // texture coords
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
};

unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

// Macros
#define TEXTURE_TEST
#define LOAD_TEXTURE

#ifdef TEXTURE_TEST
cl::make_kernel<cl::Image2D> tester(test_kernel);
cl::make_kernel<cl::Image2D, cl::Buffer> debug_tester(debug_kernel);
#else
cl::make_kernel<cl::Buffer> tester(test_kernel);
#endif // TEXTURE_TEST

cl::make_kernel<float, float, cl::Image2D, cl::Image2D, cl::Image2D> advecter(advect_kernel);
cl::make_kernel<cl::Image2D, cl::Image2D> tex_copier(tex_copy_kernel);

// Images
cl::Image2D target_texture;
cl::Image2D old_vel;
cl::Image2D new_vel;

// Reference: https://github.com/nothings/stb/blob/master/stb_image.h#L4
// To use stb_image, add this in *one* C++ source file.
     #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#endif //~ Glitter Header
