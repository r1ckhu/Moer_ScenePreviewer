#include "OpenGLContext.h"

#include "OpenGLHeader.h"

static void on_key_callback(GLFWwindow* glfwWindow, int key, int scancode,
                            int action, int mods) {
   IWindow* window =
       static_cast<IWindow*>(glfwGetWindowUserPointer(glfwWindow));
   window->onKey(key, scancode, action, mods);
}

static void on_scroll_callback(GLFWwindow* window, double xoffset,
                               double yoffset) {}

static void on_window_size_callback(GLFWwindow* window, int width, int height) {

}

static void on_window_close_callback(GLFWwindow* glfwWindow) {
   IWindow* window =
       static_cast<IWindow*>(glfwGetWindowUserPointer(glfwWindow));
   window->onClose();
}

bool OpenGL_Context::init(IWindow* window) {
   Render::RenderContext::init(window);

   /* Initialize the library */
   if (!glfwInit()) {
      fprintf(stderr, "Error: GLFW Window couldn't be initialized\n");
      return false;
   }

   // Create the window and store this window as window pointer
   // so that we can use it in callback functions

   glfwWindowHint(GLFW_SAMPLES, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
   glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
   // glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

   auto glWindow = glfwCreateWindow(window->width, window->height,
                                    window->title.c_str(), nullptr, nullptr);
   if (!glWindow) {
      fprintf(stderr, "Error: GLFW Window couldn't be created\n");
      return false;
   }
   window->setNativeWindow(glWindow);

   glfwSetWindowUserPointer(glWindow, window);
   // glfwSetKeyCallback(glWindow, on_key_callback);
   // glfwSetScrollCallback(glWindow, on_scroll_callback);
   glfwSetWindowSizeCallback(glWindow, on_window_size_callback);
   glfwSetWindowCloseCallback(glWindow, on_window_close_callback);
   glfwMakeContextCurrent(glWindow);

   gladLoadGL();
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_MULTISAMPLE);
   return true;
}

void OpenGL_Context::preRender() {
   glViewport(0, 0, window->width, window->height);
   glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGL_Context::postRender() {
   glfwPollEvents();
   glfwSwapBuffers((GLFWwindow*)window->getNativeWindow());
}

void OpenGL_Context::end() {
   glfwDestroyWindow((GLFWwindow*)window->getNativeWindow());
   glfwTerminate();
}