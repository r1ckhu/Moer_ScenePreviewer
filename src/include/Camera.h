#pragma once
#include <iostream>
#include <nlohmann/json.hpp>

#include "Shader.h"
#include "Transform.h"

using Vector3f = Eigen::Vector3f;
using Vector4f = Eigen::Vector4f;
using Vector2i = Eigen::Vector2i;
using Point3f = Eigen::Vector3f;
using Json = nlohmann::json;

class PinHoleCamera {
  public:
   PinHoleCamera(Point3f lookFrom, Point3f lookAt, Vector3f up,
                 float xFov = 45.f, Vector2i resolution = Vector2i(512, 512));
   PinHoleCamera(const Json& cameraJson);
   Vector3f lookAt, up, right;
   Point3f cameraPosition, pointLookAt;

  public:
   // Shader is a light-weight class, so just pass value
   void update(Shader shader);

  public:
   // model matrix = indentiy
   Matrix4f model, view, projection, viewPort;
   Matrix4f worldToScreen;

  private:
   void setActualValue(Point3f lookFrom, Point3f lookAt, Vector3f up,
                       float xFov, Vector2i resolution);
};
