#include "GLWindow.h"

GLWindow::GLWindow() : glfwWindow(nullptr), isRunning(true) {
   openglCtx = std::make_unique<OpenGL_Context>();
   imguiCtx = std::make_unique<ImGui_Context>();
}
void GLWindow::setNativeWindow(void* window) {
   glfwWindow = (GLFWwindow*)window;
}
void* GLWindow::getNativeWindow() { return glfwWindow; }
bool GLWindow::getIsRunning() { return isRunning; }

void GLWindow::setScene(std::shared_ptr<Scene> scene) { this->scene = scene; }

bool GLWindow::init(int width, int height, const std::string& title) {
   this->width = width;
   this->height = height;
   this->title = title;
   openglCtx->init(this);
   imguiCtx->init(this);
   float xscale, yscale;
   glfwGetWindowContentScale(glfwWindow, &xscale, &yscale);

   this->actualWidth = width * xscale;
   this->actualHeight = height * yscale;
   scene = std::make_shared<Scene>(actualWidth, actualHeight);

   return isRunning;
}

GLWindow::~GLWindow() {
   openglCtx->end();
   imguiCtx->end();
}

void GLWindow::onResize(int width, int height) {
   this->width = width;
   this->height = height;
   float xscale, yscale;
   glfwGetWindowContentScale(glfwWindow, &xscale, &yscale);

   this->actualWidth = width * xscale;
   this->actualHeight = height * yscale;
   // TODO: call actual resize here
   render();
}

void GLWindow::onScroll(float delta) {
   // mSceneView->on_mouse_wheel(delta);
}

void GLWindow::onKey(int key, int scancode, int action, int mods) {
   if (action == GLFW_PRESS) {
   }
}

void GLWindow::onClose() {
   isRunning = false;
   imGuiManager.onClose();
}

void GLWindow::render() {
   // Clear the view
   openglCtx->preRender();
   imguiCtx->preRender();

   GLuint renderResultTextureId = scene->render();
   imGuiManager.render(scene, renderResultTextureId);

   imguiCtx->postRender();
   // Render end, swap buffers
   openglCtx->postRender();
}

void GLWindow::handleInput() {
   // TODO: move this and camera to scene UI component?

   if (glfwGetKey(glfwWindow, GLFW_KEY_W) == GLFW_PRESS) {
      // mSceneView->on_mouse_wheel(-0.4f);
   }

   if (glfwGetKey(glfwWindow, GLFW_KEY_S) == GLFW_PRESS) {
      // mSceneView->on_mouse_wheel(0.4f);
   }

   if (glfwGetKey(glfwWindow, GLFW_KEY_F) == GLFW_PRESS) {
      // mSceneView->reset_view();
   }

   double x, y;
   glfwGetCursorPos(glfwWindow, &x, &y);

   // mSceneView->on_mouse_move(x, y, Input::GetPressedButton(mWindow));
}