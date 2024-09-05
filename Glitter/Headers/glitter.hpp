// Preprocessor Directives
#ifndef GLITTER
#define GLITTER
#pragma once

// System Headers
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <CL/cl.hpp>

// Local Headers
#include <Timer.hpp>

// Define Some Constants
const int mWidth = 1024;
const int mHeight = 1024;

Timer main_timer;

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
cl::Kernel divergence_kernel;
cl::Kernel jacobi_kernel;
cl::Kernel gradient_kernel;
cl::Kernel vorticity_kernel;
cl::Kernel vorticity_confiner_kernel;
cl::Kernel display_convert_kernel;
cl::Kernel boundary_kernel;
cl::Kernel tex_randomize_kernel;
cl::Kernel force_randomize_kernel;
cl::Kernel mix_kernel;
cl::Kernel tex_copy_kernel;
cl::Kernel tex_neg_randomize_kernel;
cl::Kernel neg_check_kernel;
cl::Kernel click_effect_test_kernel;
cl::Kernel image_reset_kernel;
cl::Kernel click_effect_kernel;
cl::Kernel add_dye_kernel;
cl::Kernel gravity_kernel;

cl::NDRange global_tex(mWidth, mHeight);
cl::NDRange global(10);
cl::NDRange single_thread(1);

//float hardcoded_vertices[] = {
//    // positions          // colors           // texture coords
//     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
//     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
//    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
//    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
//};

float hardcoded_vertices[] = {
    // positions          // colors           // texture coords
     1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
     1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
    -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
    -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
};

unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

// Macros
#define TEXTURE_TEST
#define LOAD_TEXTURE
//#define RAND_TEX
#define STD_TIMESTEP
#define VORTICITY
#define NEUMANN_BOUND
//#define DISABLE_SIM
#define RESET_TEXTURES
//#define RESET_PRESSURE_EACH_ITER
#define JACOBI_REPS 50

#ifdef TEXTURE_TEST
cl::make_kernel<cl::Image2D> tester(test_kernel);
cl::make_kernel<cl::Image2D, cl::Buffer> debug_tester(debug_kernel);
#else
cl::make_kernel<cl::Buffer> tester(test_kernel);
#endif // TEXTURE_TEST

cl::make_kernel<float, float, float, cl::Image2D, cl::Image2D, cl::Image2D> advecter(advect_kernel);
cl::make_kernel<cl::Image2D, cl::Image2D> tex_copier(tex_copy_kernel);
cl::make_kernel<float, cl::Image2D, cl::Image2D> divergencer(divergence_kernel);
cl::make_kernel<float, float, cl::Image2D, cl::Image2D, cl::Image2D> jacobier(divergence_kernel);
cl::make_kernel<float, cl::Image2D, cl::Image2D, cl::Image2D> gradienter(gradient_kernel);
cl::make_kernel<float, cl::Image2D, cl::Image2D> vorticitier(vorticity_kernel);
cl::make_kernel<float, float, float, float, cl::Image2D, cl::Image2D, cl::Image2D> vorticity_confiner(vorticity_confiner_kernel);
cl::make_kernel<float, cl::Image2D, cl::Image2D> boundarier(boundary_kernel);
cl::make_kernel<cl::Image2D, cl::Image2D> display_converter(display_convert_kernel);
cl::make_kernel<float, cl::Image2D, cl::Image2D, cl::Image2D> mixer(mix_kernel);
cl::make_kernel<cl::Image2D> tex_randomizer(tex_randomize_kernel);
cl::make_kernel<cl::Image2D> tex_neg_randomizer(tex_neg_randomize_kernel);
cl::make_kernel<cl::Image2D, cl::Image2D> neg_checker(neg_check_kernel);
cl::make_kernel<float, int, cl::Image2D, cl::Image2D> force_randomizer(force_randomize_kernel);
cl::make_kernel<int, int, cl::Image2D> click_effect_tester(click_effect_test_kernel);
cl::make_kernel<cl::Image2D> image_resetter(image_reset_kernel);
cl::make_kernel<int, int, float, int, cl::Image2D, cl::Image2D> click_effecter(click_effect_kernel);
cl::make_kernel<int, int, float, int, cl::Image2D> dye_adder(add_dye_kernel);
cl::make_kernel<cl::Image2D, cl::Image2D> gravitier(gravity_kernel);

// Images
cl::Image2D target_texture;
cl::Image2D old_vel;
cl::Image2D new_vel;
cl::Image2D velocity_divergence;
cl::Image2D old_pressure;
cl::Image2D new_pressure;
cl::Image2D vorticity;
cl::Image2D display_texture;
cl::Image2D dye_texture;
cl::Image2D dye_texture_new;

// Reference: https://github.com/nothings/stb/blob/master/stb_image.h#L4
// To use stb_image, add this in *one* C++ source file.
     #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#endif //~ Glitter Header
