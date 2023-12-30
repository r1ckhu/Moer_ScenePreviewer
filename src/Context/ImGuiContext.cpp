#include "ImGuiContext.h"

bool ImGui_Context::init(IWindow *window) {
   GLFWwindow *glfwWindow =
       static_cast<GLFWwindow *>(window->getNativeWindow());

   // Setup Dear ImGui context
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO &io = ImGui::GetIO();
   (void)io;
   io.ConfigFlags |=
       ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
   io.ConfigFlags |=
       ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

   float xscale, yscale;
   glfwGetWindowContentScale(glfwWindow, &xscale, &yscale);

   // Setup Dear ImGui style
   ImGui::StyleColorsDark();
   ImGui::GetStyle().ScaleAllSizes(xscale);

   // ImGui::StyleColorsLight();
   // Setup Platform/Renderer backends
   const char *glsl_version = "#version 130";
   ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
   ImGui_ImplOpenGL3_Init(glsl_version);

   ImFont *font = io.Fonts->AddFontFromMemoryCompressedBase85TTF(
       FiraCode_compressed_data_base85, static_cast<int>(xscale * 16));
   return true;
};

void ImGui_Context::preRender() {
   // Start the Dear ImGui frame
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();
};

void ImGui_Context::postRender() {
   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
};

void ImGui_Context::end() {
   ImGui_ImplOpenGL3_Shutdown();
   ImGui_ImplGlfw_Shutdown();
   ImGui::DestroyContext();
};
