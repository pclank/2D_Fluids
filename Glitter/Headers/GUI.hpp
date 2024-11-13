#pragma once

#include "Timer.hpp"
#include <string>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

enum ClickMode {
    VELOCITY_MODE, DYE_MODE
};

enum RenderedTexture {
    VELOCITY, PRESSURE, DYE
};

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
    /// Reset random force flag
    /// </summary>
    void ResetForceEnabled();

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

    /// <summary>
    /// Resets all input flags
    /// </summary>
    void ResetInputFlags();

    /// <summary>
    /// Update mouse position in GUI
    /// </summary>
    /// <param name="xpos"></param>
    /// <param name="ypos"></param>
    inline void MousePositionUpdate(double xpos, double ypos)
    {
        mouse_prev_xpos = mouse_xpos;
        mouse_prev_ypos = mouse_ypos;
        mouse_xpos = xpos;
        mouse_ypos = ypos;
    }

//private:
    /// <summary>
    /// The GUI callback is used to update our reference's state using the GUI_BUTTON enum
    /// </summary>
    //void GuiButtonCallback(GUI_BUTTON button);

    double mouse_xpos;
    double mouse_ypos;
    double mouse_prev_xpos;
    double mouse_prev_ypos;
    bool cursor_enabled;
    bool clicking_enabled;
    bool clicked;
    bool reset_pressed;
    bool dye_extreme_mode;
    bool gui_enabled;
    bool apply_gravity;
    bool normalize_vel_dir;
    int selected_index;
    float viscosity;
    float dx;
    ClickMode click_mode;
    RenderedTexture rendered_texture;

private:
    GLFWwindow* p_window;
    Timer& m_timer;
    bool rand_force;
    bool rand_force_dir;
    float force_scale;
    float mix_bias;
};
