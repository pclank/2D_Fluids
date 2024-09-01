#pragma once

#include "Timer.hpp"
#include <string>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

/// <summary>
/// GUI wrapper that handles all imgui related calls
/// </summary>
class GUI
{
public:
    GUI(GLFWwindow* pWindow, Timer& timer);

    /// <summary>
    /// Initialize our GUI wrapper
    /// </summary>
    void Init();

    /// <summary>
    /// Render our GUI with updated reference data
    /// </summary>
    void Render();

    /// <summary>
    /// Perform GUI cleanup
    /// </summary>
    void Cleanup();

    /// <summary>
    /// Whether the random force flag is enabled
    /// </summary>
    /// <returns>: the flag</returns>
    bool IsForceEnabled();

    /// <summary>
    /// Returns the scale of the random force
    /// </summary>
    /// <returns>: the scale</returns>
    float GetForceScale();
    
    /// <summary>
    /// Whether the random force direction should be randomized
    /// </summary>
    /// <returns>: the flag</returns>
    bool GetForceDirFlag();

    /// <summary>
    /// Returns the bias for the display texture mixing
    /// </summary>
    /// <returns>: the bias</returns>
    float GetMixBias();

//private:
    /// <summary>
    /// The GUI callback is used to update our reference's state using the GUI_BUTTON enum
    /// </summary>
    //void GuiButtonCallback(GUI_BUTTON button);

private:
    GLFWwindow* p_window;
    Timer& m_timer;
    bool rand_force;
    bool rand_force_dir;
    float force_scale;
    float mix_bias;
};
