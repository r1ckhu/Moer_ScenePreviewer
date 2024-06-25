#ifndef __SCENE_H__
#define __SCENE_H__
#include <chrono>
#include <memory>
#include <nlohmann/json.hpp>

#include "Camera.h"
#include "FrameBuffer.h"
#include "Light.h"
#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include "Transform.h"
#include "VertexArrayObject.h"

using Json = nlohmann::json;

class Scene {
  public:
   std::shared_ptr<PinHoleCamera> camera;
   std::vector<std::shared_ptr<Mesh>> meshes;

   std::vector<VertexArrayObject> VAOs;
   QuadVertexArrayObject quadVAO;

   FrameBuffer framebuffer;
   Shader shader;
   ScreenShader screenShader;
   DefaultWhiteMaterial material;
   DefaultWhiteLight light;
   std::chrono::duration<long long, std::milli> loadDurationMs;

  public:
   Scene(int width, int height);
   GLuint render();
   void loadScene(std::string fullScenePath, std::string workingDir);
   void saveScene(std::string fullScenePath);
   int getMeshCount();
   int getTotalVertices();
   int getTotalIndices();
   int width, height;  // framebuffer size
   std::string workingDir;

  private:
   Json json;
   void LoadMeshesFronJson(const Json& sceneJson);
   void LoadSingleMeshFromJson(const Json& entityJson);
   void LoadCameraFromJson(const Json& sceneJson);
   void createVAOsFromMeshes();
   void clearPreviousScene();
   Matrix4f getTransform(const Json& json);
   int totalVertices, totalIndices;
};

#endif