#include "Scene.h"

#include <fstream>
#include <iostream>

Scene::Scene(int width, int height) {
   this->width = width;
   this->height = height;
   this->shader = PhoneShader();
}

void Scene::loadScene(std::string fullScenePath, std::string workingDir) {
   clearPreviousScene();
   auto startTime = std::chrono::high_resolution_clock::now();
   std::ifstream file(fullScenePath);
   this->workingDir = workingDir + "\\";
   this->fullScenePath = fullScenePath;
   if (file.is_open()) {
      file >> json;
      file.close();
   }
   LoadMeshesFronJson(json);
   LoadCameraFromJson(json);
   width = camera->resolution[0];
   height = camera->resolution[1];
   createVAOsFromMeshes();
   quadVAO.create_buffers();
   light.position = camera->cameraPosition;
   framebuffer.create_buffers(width, height);

   if (json.contains("renderer")) {
      MoerRenderConfigLoader::generate(this->moerRenderConfig,
                                       json.at("renderer"));
   }

   auto endTime = std::chrono::high_resolution_clock::now();
   loadDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
       endTime - startTime);
}

void Scene::saveScene(const std::string& fullScenePath) {
   json["camera"]["transform"]["position"] = {camera->cameraPosition[0],
                                              camera->cameraPosition[1],
                                              camera->cameraPosition[2]};
   json["camera"]["transform"]["look_at"] = {
       camera->pointLookAt[0], camera->pointLookAt[1], camera->pointLookAt[2]};
   json["camera"]["transform"]["up"] = {camera->up[0], camera->up[1],
                                        camera->up[2]};
   json["camera"]["fov"] = camera->xFov;
   std::ofstream ofs(fullScenePath);
   if (ofs.is_open()) {
      ofs << json;
      ofs.close();
   }
}

int Scene::getMeshCount() { return meshes.size(); }
int Scene::getTotalIndices() { return totalIndices; }
int Scene::getTotalVertices() { return totalVertices; }

GLuint Scene::render() {
   shader.use();
   material.update(shader);
   light.update(shader);
   if (camera != nullptr) {
      camera->update(shader);
   }

   framebuffer.bind();
   for (auto&& VAO : VAOs) {
      VAO.draw();
   }
   framebuffer.unbind();

   screenShader.use();
   screenShader.setInt("screenTexture", 0);
   glDisable(GL_DEPTH_TEST);
   glEnable(GL_MULTISAMPLE);
   glBindTexture(GL_TEXTURE_2D, framebuffer.get_texture());
   return framebuffer.get_texture();
}

void Scene::LoadMeshesFronJson(const Json& SceneJson) {
   for (auto& entity : SceneJson.at("entities")) {
      LoadSingleMeshFromJson(entity);
   }
}

void Scene::LoadSingleMeshFromJson(const Json& entityJson) {
   try {
      std::string type = entityJson.at("type");
      std::shared_ptr<Mesh> mesh = nullptr;
      if (type == "mesh") {
         std::string file_path = entityJson.at("file");
         mesh = std::make_shared<Mesh>(workingDir + file_path);
         assert(mesh->openglIndex.size() % 3 == 0);
         totalIndices += mesh->openglIndex.size();
         totalVertices += mesh->vertices_nr();
      } else if (type == "quad") {
         mesh = std::make_shared<Quad>();
      } else if (type == "cube") {
         mesh = std::make_shared<Cube>();
      } else {
         throw std::invalid_argument("Unknown value for key 'type': " + type);
      }

      if (entityJson.contains("transform") &&
          !entityJson.at("transform").empty()) {
         auto transform = getTransform(entityJson.at("transform"));
         mesh->apply(transform);
      }
      meshes.push_back(mesh);

   } catch (const std::out_of_range& e) {
      std::cerr << "Missing essential property when loading entity:  "
                << e.what() << std::endl;
   } catch (const std::invalid_argument& e) {
      std::cerr << "Invalid argument for keys in entity: " << e.what()
                << std::endl;
   }
}

Matrix4f Scene::getTransform(const Json& transformJson) {
   Matrix4f ret = Matrix4f::Identity();
   try {
      if (transformJson.contains("position")) {
         auto translate = transformJson.at("position");
         ret *=
             Transform::getTranslate(translate[0], translate[1], translate[2]);
      }
      if (transformJson.contains("scale")) {
         auto scale = transformJson.at("scale");
         if (scale.is_array()) {
            ret *= Transform::getScale(scale[0], scale[1], scale[2]);
         } else {
            ret *= Transform::getScale(scale.get<float>(), scale.get<float>(),
                                       scale.get<float>());
         }
      }
      if (transformJson.contains("rotation")) {
         auto rotation = transformJson.at("rotation");
         float x = rotation[0], y = rotation[1], z = rotation[2];
         ret *= Transform::getRotateEuler(
             Transform::AngleDegreeValue(x), Transform::AngleDegreeValue(y),
             Transform::AngleDegreeValue(z), Transform::EulerType::EULER_YZX);
      }
      return ret;
   } catch (const std::out_of_range& e) {
      std::cerr
          << "Missing essential property when loading entity's transform:  "
          << e.what() << std::endl;
   }
   return ret;
}

void Scene::LoadCameraFromJson(const Json& sceneJson) {
   Json cameraJson = sceneJson.at("camera");
   camera = std::make_shared<PinHoleCamera>(cameraJson);
}

void Scene::createVAOsFromMeshes() {
   for (auto&& mesh : meshes) {
      VertexArrayObject VAO;
      VAO.create_buffers(mesh);
      VAOs.push_back(VAO);
   }
}

void Scene::clearPreviousScene() {
   // FIXME: possible memory leak
   for (auto&& VAO : VAOs) {
      VAO.delete_buffers();
   }
   VAOs.clear();
   meshes.clear();
   json.clear();
   camera = nullptr;
   totalIndices = totalVertices = 0;
}