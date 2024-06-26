#pragma once
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
   // Shader is a light-weight class, so just pass value
   void update(Shader shader);
   void adjustYawAndPitch(float deltaYaw, float deltaPitch);
   void resetToInitValue();
   void resetPosition();
   void resetLookAtPoint();
   void resetXFov();

  public:
   // Camera properties
   Vector3f vecLookAt, up, right;
   Point3f cameraPosition, pointLookAt;
   float xFov;
   float aspectRatio;
   Vector2i resolution;
   // model matrix = indentiy
   Matrix4f model, view, projection, viewPort;
   Matrix4f worldToScreen;

  private:
   void setActualValue(Point3f lookFrom, Point3f lookAt, Vector3f up,
                       float xFov, Vector2i resolution);
   void reCalculateAllMatrices();

  private:
   Vector3f ini_vecLookAt, ini_up;
   Point3f ini_cameraPosition, ini_pointLookAt;
   float ini_xFov;
};
