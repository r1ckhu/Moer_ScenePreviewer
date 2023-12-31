#include "Scene.h"

Scene::Scene(int width, int height) {
   this->width = width;
   this->height = height;
   this->shader = PhoneShader();
}

void Scene::loadScene(std::string fullScenePath, std::string workingDir) {
   clearPreviousScene();
   std::ifstream file(fullScenePath);
   this->workingDir = workingDir + "\\";
   if (file.is_open()) {
      file >> json;
      file.close();
   }
   LoadMeshesFronJson(json);
   LoadCameraFromJson(json);
   createVAOsFromMeshes();
   quadVAO.create_buffers();
   light.position = camera->cameraPosition;
   framebuffer.create_buffers(width, height);
}

void Scene::saveScene(std::string fullScenePath) {
   json["camera"]["transform"]["position"] = {camera->cameraPosition[0],
                                              camera->cameraPosition[1],
                                              camera->cameraPosition[2]};
   json["camera"]["transform"]["look_at"] = {
       camera->pointLookAt[0], camera->pointLookAt[1], camera->pointLookAt[2]};
   json["camera"]["transform"]["up"] = {camera->up[0], camera->up[1],
                                        camera->up[2]};
   std::ofstream ofs(fullScenePath);
   if (ofs.is_open()) {
      ofs << json;
      ofs.close();
   }
}

void Scene::render() {
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
   quadVAO.draw();
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
#ifdef DEBUG
         std::cout << "Load mesh at " << file_path << std::endl;
#endif
         mesh = std::make_shared<Mesh>(workingDir + file_path);
      } else if (type == "quad") {
#ifdef DEBUG
         std::cout << "Load quad." << std::endl;
#endif
         mesh = std::make_shared<Quad>();
      } else if (type == "cube") {
#ifdef DEBUG
         std::cout << "Load cube." << std::endl;
#endif
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
#ifdef DEBUG
   std::cout << "Getting Transform for this entity..." << std::endl;
#endif
   Matrix4f ret = Matrix4f::Identity();
   try {
      if (transformJson.contains("translate")) {
         auto translate = transformJson.at("translate");
         ret *=
             Transform::getTranslate(translate[0], translate[1], translate[2]);
#ifdef DEBUG
         std::cout << "Translate: " << translate[0] << ' ' << translate[1]
                   << ' ' << translate[2] << std::endl;
#endif
      }
      if (transformJson.contains("scale")) {
         auto scale = transformJson.at("scale");
         if (scale.is_array()) {
            ret *= Transform::getScale(scale[0], scale[1], scale[2]);
#ifdef DEBUG
            std::cout << "Scale: " << scale[0] << ' ' << scale[1] << ' '
                      << scale[2] << std::endl;
#endif
         } else {
            ret *= Transform::getScale(scale.get<float>(), scale.get<float>(),
                                       scale.get<float>());
#ifdef DEBUG
            std::cout << "Scale: " << scale.get<float>() << std::endl;
#endif
         }
      }
      if (transformJson.contains("rotation")) {
         auto rotation = transformJson.at("rotation");
         float x = rotation[0], y = rotation[1], z = rotation[2];
         ret *= Transform::getRotateEuler(
             Transform::AngleDegreeValue(x), Transform::AngleDegreeValue(y),
             Transform::AngleDegreeValue(z), Transform::EulerType::EULER_YZX);
#ifdef DEBUG
         std::cout << "Roatation: " << rotation[0] << ' ' << rotation[1] << ' '
                   << rotation[2] << std::endl;
#endif
      }
      return ret;
   } catch (const std::out_of_range& e) {
      std::cerr
          << "Missing essential property when loading entity's transform:  "
          << e.what() << std::endl;
   }
}

void Scene::LoadCameraFromJson(const Json& sceneJson) {
#ifdef DEBUG
   std::cout << "Loading Camera..." << std::endl;
#endif
   Json cameraJson = sceneJson.at("camera");
   camera = std::make_shared<PinHoleCamera>(cameraJson);
#ifdef DEBUG
   std::cout << "Load camera successfully." << std::endl;
#endif
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
}