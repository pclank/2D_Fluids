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

//private:
    /// <summary>
    /// The GUI callback is used to update our reference's state using the GUI_BUTTON enum
    /// </summary>
    //void GuiButtonCallback(GUI_BUTTON button);

private:
    GLFWwindow* p_window;
    Timer& m_timer;
};
