#include "GUI.hpp"

GUI::GUI(GLFWwindow* pWindow, Timer& timer)
    :
    p_window(pWindow),
    m_timer(timer)
{ }

void GUI::Init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(p_window, true);
    //ImGui_ImplOpenGL3_Init("#version 130");
    //ImGui_ImplOpenGL3_Init("#version 330");
    ImGui_ImplOpenGL3_Init();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
}

void GUI::Render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Control Window");
    ImGui::Text("DeltaTime: %f", m_timer.GetDeltaTime());
    ImGui::Text("FPS: %.2f", m_timer.GetFPS());
    ImGui::Text("Use SPACEBAR to enable/disable cursor!");
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::Cleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

//void GUI::GuiButtonCallback(GUI_BUTTON button)
//{
//    switch (button)
//    {
//    case GUI_BUTTON::MODEL_TOGGLE:
//        break;
//    case GUI_BUTTON::CAMERA_MODE_TOGGLE:
//        m_camera.arcball_mode = !m_camera.arcball_mode;
//        m_cameraMode = m_camera.arcball_mode ?
//            std::string("Camera Type: Arcball Camera") : std::string("Camera Type: Normal Camera");
//        break;
//    default:
//        break;
//    }
//}