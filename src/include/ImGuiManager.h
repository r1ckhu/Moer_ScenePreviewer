#pragma once

#include <memory>

#include "Scene.h"

class ImGuiManager {
  public:
   void init();
   void render(std::shared_ptr<Scene> scene, GLuint renderResultTextureId);
   void onClose();
};