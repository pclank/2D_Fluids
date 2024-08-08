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
cl::Kernel test_kernel;
//cl::NDRange global(mWidth * mHeight);
cl::NDRange global(10);

cl::make_kernel<cl::Buffer> tester(test_kernel);

// Images
cl::Image2D target_texture;

// Reference: https://github.com/nothings/stb/blob/master/stb_image.h#L4
// To use stb_image, add this in *one* C++ source file.
//     #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#endif //~ Glitter Header
